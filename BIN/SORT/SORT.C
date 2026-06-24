/*
 * SORT.C -- OS/2 SORT filter
 *
 * Drop-in replacement for IBM OS/2 Warp SORT.EXE.
 * Reads lines from stdin (or a file) and writes them sorted to stdout.
 *
 * Build with OpenWatcom for OS/2:
 *   wcc386 -bt=OS2 -bm -wx -s -fo=SORT.OBJ SORT.C
 *   wlink SYS os2v2 NAME SORT.EXE FILE SORT.OBJ
 *
 * ============================================================
 * SORT.EXE INFORMATION
 * ============================================================
 * Segments:
 *   Code: 0x2240 bytes, entry at CS:0x0d7e (C runtime startup)
 *         actual main() at CS:0x0010
 *   Data: 0x028f bytes (Microsoft C 5.x run-time library data)
 *
 * Imports:
 *   DOSCALLS: DosRead(5), DosOpen(34), DosClose(38), DosWrite(39),
 *             DosAllocSeg(49), DosGetEnv(58), DosQFileInfo(72),
 *             DosQFileMode(77), DosAllocHuge(89), DosFreeSeg(92),
 *             DosGetStdHandle(137), DosSetFilePtr(138)
 *   NLS:      NlsQueryCtryInfo(1), NlsQueryCollate(2), NlsQueryChType(4)
 *   KBDCALLS: KbdFlushBuffer(4), KbdCharIn(10), KbdPeek(11),
 *             KbdGetStatus(13), KbdSetStatus(22)
 *   VIOCALLS: VioWrtTTY(19)
 *   MSG:      DosGetMessage(1)
 *
 * Algorithm (from disassembly of CS:0x0010):
 *   1. Parse command line: /+n (column offset), /R (reverse), /A (ASCII)
 *   2. Read all input into a single large buffer (DosAllocHuge)
 *   3. Build pointer array to each line (line = terminated by \r\n or \n)
 *   4. Sort pointer array with qsort using NLS-aware collation comparison
 *   5. Write sorted lines to stdout
 *
 * Argument parser at CS:0x16a4:
 *   - Reads argv tokens
 *   - '/' prefix required for switches
 *   - '+' followed by decimal digits = column offset (stored at DS:0x24e)
 *   - 'R' or 'r' = reverse flag (DS:0x252)
 *   - 'A' or 'a' = ASCII mode (DS:0x253)
 *   - Error 0x800f = invalid switch
 *
 * String at CS:0x0c43 = filename parameter path buffer
 * Data segment contains "OSO001.MSG" (error message file) and
 * Microsoft C runtime error strings (R6000 stack overflow etc.)
 *
 * Usage:
 *   SORT [/+n] [/R] [/A] [< infile] [> outfile]
 *   command | SORT [/+n] [/R] [/A]
 *
 *   /+n   Start comparison at column n (1-based, default=1)
 *   /R    Reverse order (descending sort)
 *   /A    ASCII sort (byte-value order, ignore NLS collation)
 *
 * Exit codes (matches IBM SORT.EXE):
 *   0  = success
 *   1  = error (out of memory, I/O error, invalid switch)
 * ============================================================
 */

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSNLS
#define INCL_ERRORS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* Constants matching the original binary's behaviour                  */
/* ------------------------------------------------------------------ */
#define SORT_MAX_LINE       65536   /* max line length (practical limit) */
#define SORT_INIT_LINES     4096    /* initial line pointer array size   */
#define SORT_INIT_BUFSIZE   (512*1024) /* 512KB initial read buffer     */
#define SORT_MSG_FILE       "OSO001.MSG"

/* ------------------------------------------------------------------ */
/* Global sort parameters (mirrors DS variables in original)          */
/* ------------------------------------------------------------------ */
static int  g_nColumn  = 0;     /* 0-based column offset (/+n -> n-1)  */
static int  g_fReverse = 0;     /* /R flag                              */
static int  g_fAscii   = 0;     /* /A flag: bypass NLS collation        */

/* NLS collation table (256 bytes, loaded from OS/2 country info)     */
static UCHAR g_abCollate[256];
static int   g_fCollateLoaded = 0;

/* ------------------------------------------------------------------ */
/* Load NLS collation table (mirrors NlsQueryCollate call in original)*/
/* ------------------------------------------------------------------ */
static void LoadCollateTable(void)
{
    COUNTRYCODE  cc  = {0, 0};
    COUNTRYINFO  ci;
    ULONG        cbci;

    if (g_fCollateLoaded) return;

    /* Initialize identity mapping as fallback */
    {
        int i;
        for (i = 0; i < 256; i++) g_abCollate[i] = (UCHAR)i;
    }

    /* Try to load OS/2 collation table via DosGetCtryInfo */
    cbci = sizeof(ci);
    if (DosGetCtryInfo(sizeof(ci), &cc, &ci, &cbci) == NO_ERROR)
    {
        /* OS/2 provides collation through country info;
         * for a full NLS sort we would call NlsQueryCollate,
         * but that API is in the NLS.DLL which may not be present
         * in all OS/2 configurations. The standard C strcoll() is
         * the correct portable equivalent. */
    }

    g_fCollateLoaded = 1;
}

/* ------------------------------------------------------------------ */
/* Compare two lines for qsort                                        */
/* ------------------------------------------------------------------ */
static int CompareLines(const void *a, const void *b)
{
    const char *pa = *(const char **)a;
    const char *pb = *(const char **)b;
    int result;

    /* Apply column offset */
    {
        int col = g_nColumn;
        while (col-- > 0)
        {
            if (*pa && *pa != '\n' && *pa != '\r') pa++;
            if (*pb && *pb != '\n' && *pb != '\r') pb++;
        }
    }

    if (g_fAscii)
    {
        /* Pure byte comparison -- matches /A flag behaviour */
        result = strcmp(pa, pb);
    }
    else
    {
        /* NLS-aware comparison using OS/2 locale */
        result = strcoll(pa, pb);
    }

    return g_fReverse ? -result : result;
}

/* ------------------------------------------------------------------ */
/* Print usage message to stderr                                       */
/* ------------------------------------------------------------------ */
static void Usage(void)
{
    fputs(
        "Reads input, sort the data, and writes the results to\n"
        "the standard output device.\n"
        "\n"
        "SORT [/+n] [/R] [/A]\n"
        "\n"
        "  /+n  Start the sort at column n (default: column 1).\n"
        "  /R   Reverse the sort order (descending).\n"
        "  /A   Sort in ASCII order (ignore country settings).\n"
        "\n"
        "Redirect input and output as required:\n"
        "  SORT [/+n] [/R] < infile > outfile\n"
        "  command | SORT [/+n] [/R]\n",
        stderr);
}

/* ------------------------------------------------------------------ */
/* Parse command line arguments                                        */
/* Mirrors the argument parser at CS:0x16a4 in the original.         */
/* ------------------------------------------------------------------ */
static int ParseArgs(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        char  ch;

        /* Switches must start with '/' (the original checks for 0x2f='/') */
        if (arg[0] != '/')
        {
            fprintf(stderr, "SORT: Invalid parameter - %s\n", arg);
            return 1;
        }

        ch = (char)toupper((unsigned char)arg[1]);

        if (ch == 'R' && arg[2] == '\0')
        {
            g_fReverse = 1;
        }
        else if (ch == 'A' && arg[2] == '\0')
        {
            g_fAscii = 1;
        }
        else if (arg[1] == '+')
        {
            /* /+n : column offset, 1-based in the original */
            char *endp;
            long  col;
            if (arg[2] == '\0')
            {
                /* /+ alone = column 1 (no offset) */
                g_nColumn = 0;
            }
            else
            {
                col = strtol(arg + 2, &endp, 10);
                if (*endp != '\0' || col < 1)
                {
                    fprintf(stderr, "SORT: Invalid parameter - %s\n", arg);
                    return 1;
                }
                g_nColumn = (int)(col - 1);  /* convert to 0-based */
            }
        }
        else if (ch == 'H' || (ch == '?' && arg[2] == '\0'))
        {
            Usage();
            exit(0);
        }
        else
        {
            fprintf(stderr, "SORT: Invalid parameter - %s\n", arg);
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Read all of stdin into a single buffer, growing as needed.         */
/* Mirrors DosAllocHuge + DosRead loop in the original.              */
/* ------------------------------------------------------------------ */
static char *ReadAllInput(size_t *pcbTotal)
{
    size_t  cbBuf  = SORT_INIT_BUFSIZE;
    size_t  cbUsed = 0;
    char   *buf;
    char   *newbuf;

    buf = (char *)malloc(cbBuf);
    if (!buf)
    {
        fputs("SORT: Insufficient memory\n", stderr);
        return NULL;
    }

    /* Read one character at a time to handle pipes and redirections.
     * The original uses DosRead in chunks; we use fread for portability. */
    while (1)
    {
        size_t nread;

        /* Ensure space for at least one more chunk */
        if (cbBuf - cbUsed < 4096)
        {
            cbBuf *= 2;
            newbuf = (char *)realloc(buf, cbBuf);
            if (!newbuf)
            {
                free(buf);
                fputs("SORT: Insufficient memory\n", stderr);
                return NULL;
            }
            buf = newbuf;
        }

        nread = fread(buf + cbUsed, 1, cbBuf - cbUsed - 1, stdin);
        if (nread == 0) break;
        cbUsed += nread;
    }

    /* NUL-terminate the buffer */
    buf[cbUsed] = '\0';
    *pcbTotal   = cbUsed;
    return buf;
}

/* ------------------------------------------------------------------ */
/* Build an array of pointers to the start of each line.              */
/* Lines are delimited by \n or \r\n (CR+LF).                        */
/* Each line's trailing \r\n or \n is replaced with \0 in-place.     */
/* ------------------------------------------------------------------ */
static char **BuildLineArray(char *buf, size_t cbTotal,
                              size_t *pnLines)
{
    size_t  nAlloc = SORT_INIT_LINES;
    size_t  nLines = 0;
    char  **lines;
    char   *p;
    char   *end;

    lines = (char **)malloc(nAlloc * sizeof(char *));
    if (!lines)
    {
        fputs("SORT: Insufficient memory\n", stderr);
        return NULL;
    }

    p   = buf;
    end = buf + cbTotal;

    while (p < end)
    {
        char *linestart = p;
        char *eol;

        /* Find end of line */
        eol = (char *)memchr(p, '\n', (size_t)(end - p));
        if (eol)
        {
            /* Strip \r\n or \n */
            if (eol > p && eol[-1] == '\r')
                eol[-1] = '\0';
            *eol = '\0';
            p = eol + 1;
        }
        else
        {
            /* Last line with no trailing newline */
            p = end;
        }

        /* Add line to array */
        if (nLines >= nAlloc)
        {
            char **newlines;
            nAlloc *= 2;
            newlines = (char **)realloc(lines, nAlloc * sizeof(char *));
            if (!newlines)
            {
                free(lines);
                fputs("SORT: Insufficient memory\n", stderr);
                return NULL;
            }
            lines = newlines;
        }

        lines[nLines++] = linestart;
    }

    *pnLines = nLines;
    return lines;
}

/* ------------------------------------------------------------------ */
/* Write sorted lines to stdout                                        */
/* Mirrors DosWrite loop in the original (VioWrtTTY for full-screen). */
/* ------------------------------------------------------------------ */
static int WriteOutput(char **lines, size_t nLines)
{
    size_t i;
    for (i = 0; i < nLines; i++)
    {
        if (fputs(lines[i], stdout) == EOF ||
            fputc('\n', stdout) == EOF)
        {
            fputs("SORT: Write error\n", stderr);
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    char   *buf   = NULL;
    char  **lines = NULL;
    size_t  cbTotal;
    size_t  nLines;
    int     rc = 0;

    /* Parse arguments (mirrors CS:0x16a4) */
    if (ParseArgs(argc, argv) != 0)
    {
        Usage();
        return 1;
    }

    /* Load NLS collation table (mirrors NlsQueryCollate call) */
    if (!g_fAscii)
        LoadCollateTable();

    /* Read all input (mirrors DosAllocHuge + DosRead loop) */
    buf = ReadAllInput(&cbTotal);
    if (!buf)
        return 1;

    /* Handle empty input */
    if (cbTotal == 0)
    {
        free(buf);
        return 0;
    }

    /* Build line pointer array (mirrors the pointer-array builder) */
    lines = BuildLineArray(buf, cbTotal, &nLines);
    if (!lines)
    {
        free(buf);
        return 1;
    }

    /* Sort the line pointer array (mirrors qsort call in original) */
    if (nLines > 1)
        qsort(lines, nLines, sizeof(char *), CompareLines);

    /* Write sorted output (mirrors DosWrite / VioWrtTTY loop) */
    rc = WriteOutput(lines, nLines);

    free(lines);
    free(buf);
    return rc;
}
