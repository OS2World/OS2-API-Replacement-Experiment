/*
 * CHKDSK32.C -- OS/2 Check Disk Stub Launcher
 *
 * Drop-in replacement for IBM OS/2 Warp CHKDSK32.EXE.
 * This is the front-end stub that parses arguments and delegates the
 * actual disk checking to the CHKDSK32 DLL (loaded at runtime).
 *
 * Build with OpenWatcom for OS/2:
 *   wcc386 -bt=OS2 -bm -wx -s -ox -fo=CHKDSK32.OBJ CHKDSK32.C
 *   wlink SYS os2v2 NAME CHKDSK32.EXE FILE CHKDSK32.OBJ OPTION ELIMINATE
 *
 * ============================================================
 * CHKDSK32.EXE INFORMATION
 * ============================================================
 * Format: NE (16-bit OS/2 1.x), Microsoft C 5.x linker 5.10
 * Module: 'CHKDSK' / 'chkdsk32.EXE'
 * Segments: CODE 0x1292 bytes (entry CS:0x01ba) + DATA 0x0311 bytes
 * Only imports: DOSCALLS (12 relocations)
 *
 * DOSCALLS imports (from relocation table):
 *   ord   5 = DosRead              -- read filesystem data
 *   ord  38 = DosClose             -- close file handle
 *   ord  44 = DosOpen              -- open disk/file
 *   ord  45 = DosQueryCurrentDisk  -- query active drive
 *   ord  46 = DosSetDefaultDisk    -- set active drive
 *   ord  49 = DosAllocSeg          -- allocate 16-bit segment
 *   ord  58 = DosGetEnv            -- query cmd args/environment
 *   ord  77 = DosQFileMode         -- query filesystem/drive info
 *   ord  89 = DosAllocHuge         -- allocate large buffer
 *   ord  92 = DosFreeSeg           -- free memory segment
 *   ord 138 = DosSetFilePtr        -- seek within file/disk
 *
 * Key data strings (Seg2):
 *   'CHKDSK32'    -- module name for DosLoadModule
 *   '/AuToChEcK'  -- secret flag: boot manager automatic check
 *   'DosLoadModule error: rc = %u, Mod= %s'
 *   'DosGetProcAddr Failed, rc = %d'
 *   'DosFreeModule Error, rc = %u'
 *
 * Architecture (from disassembly):
 *   CS:0x0010 = main() -- parses argv, validates drive letter,
 *                         checks /AuToChEcK flag, calls through
 *   CS:0x01ba = C runtime startup (Microsoft C 5.x)
 *   CS:0x0288 = DosLoadModule/DosGetProcAddr sequence
 *   CS:0x02e9 = call through far pointer to real CHKDSK implementation
 *
 *   Entry at CS:0x0010 (main):
 *     - Checks argc >= 3 (requires at least: prog drive /opts)
 *     - Calls DosQueryCurrentDisk to find default drive if none given
 *     - Parses command line for drive letter and /AuToChEcK
 *     - Iterates drive flags table (0x40=removable, 0x08=remote)
 *     - Loads CHKDSK32 DLL via DosLoadModule
 *     - Calls DosGetProcAddr to find the entry ordinal
 *     - Passes (drive_num, argc, argv, env) to the DLL function
 *     - Calls DosFreeModule on exit
 *
 *   /AuToChEcK:
 *     Mixed-case flag passed by the IFS (Installable File System)
 *     layer during boot when it detects an unclean shutdown (dirty
 *     bit set in FAT/HPFS volume). Forces non-interactive mode where
 *     CHKDSK automatically fixes all errors and returns an exit code:
 *       0 = volume is clean / no errors found
 *       1 = errors found and fixed
 *       2 = errors found, could not fix all
 *       3 = fatal error (cannot access volume)
 *
 * Usage:
 *   CHKDSK32 [drive:] [/F] [/V] [/C] [/AuToChEcK]
 *
 *   drive:       Drive to check (default: current drive)
 *   /F           Fix errors automatically
 *   /V           Verbose mode (list all files)
 *   /C           Suppress confirmation prompts
 *   /AuToChEcK   Automatic mode (used by boot manager, non-interactive)
 * ============================================================
 */

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* Module and entry point names (from Seg2 data)                      */
/* ------------------------------------------------------------------ */
#define CHKDSK_DLL_NAME     "CHKDSK32"
#define AUTOCHK_FLAG        "/AuToChEcK"

/* ------------------------------------------------------------------ */
/* Exit codes                                                          */
/* ------------------------------------------------------------------ */
#define CHKDSK_OK           0   /* volume clean, no errors             */
#define CHKDSK_FIXED        1   /* errors found and fixed              */
#define CHKDSK_NOTFIXED     2   /* errors found, could not fix all     */
#define CHKDSK_FATAL        3   /* fatal: cannot access volume         */

/* ------------------------------------------------------------------ */
/* Parsed command-line options                                         */
/* ------------------------------------------------------------------ */
typedef struct {
    ULONG   ulDriveNum;     /* 1=A, 2=B, 3=C, ...                     */
    BOOL    fFix;           /* /F : fix errors                         */
    BOOL    fVerbose;       /* /V : verbose output                     */
    BOOL    fNoConfirm;     /* /C : suppress prompts                   */
    BOOL    fAutoCheck;     /* /AuToChEcK : non-interactive boot mode  */
} CHKDSK_OPTS;

/* ------------------------------------------------------------------ */
/* Typedef for the CHKDSK32.DLL main entry point                      */
/* Signature deduced from context:                                    */
/*   (drive_num, fix, verbose, noconfirm, autochk)                   */
/* ------------------------------------------------------------------ */
typedef ULONG (APIENTRY *PFNCHKDSK)(ULONG   ulDriveNum,
                                     BOOL    fFix,
                                     BOOL    fVerbose,
                                     BOOL    fNoConfirm,
                                     BOOL    fAutoCheck);

/* ------------------------------------------------------------------ */
/* Print usage                                                         */
/* ------------------------------------------------------------------ */
static void Usage(void)
{
    fputs(
        "Checks the file allocation table and files on a disk\n"
        "and displays a status report.\n"
        "\n"
        "CHKDSK32 [drive:] [/F] [/V] [/C]\n"
        "\n"
        "  drive:  Specifies the drive to check.\n"
        "  /F      Fixes errors on the disk.\n"
        "  /V      Lists each file in every directory as the disk is checked.\n"
        "  /C      Suppresses confirmation messages.\n",
        stderr);
}

/* ------------------------------------------------------------------ */
/* Parse a single command-line switch                                 */
/* Returns FALSE on unrecognised switch.                              */
/* ------------------------------------------------------------------ */
static BOOL ParseSwitch(const char *pszArg, CHKDSK_OPTS *pOpts)
{
    char ch;

    if (!pszArg || (pszArg[0] != '/' && pszArg[0] != '-'))
        return FALSE;

    /* Check for /AuToChEcK first (case-sensitive, mixed-case key)   */
    if (strcmp(pszArg + 1, AUTOCHK_FLAG + 1) == 0)
    {
        pOpts->fAutoCheck = TRUE;
        return TRUE;
    }

    ch = (char)toupper((unsigned char)pszArg[1]);

    switch (ch)
    {
    case 'F': pOpts->fFix       = TRUE; return TRUE;
    case 'V': pOpts->fVerbose   = TRUE; return TRUE;
    case 'C': pOpts->fNoConfirm = TRUE; return TRUE;
    default:
        fprintf(stderr, "CHKDSK32: Invalid parameter - %s\n", pszArg);
        return FALSE;
    }
}

/* ------------------------------------------------------------------ */
/* Parse all command-line arguments                                   */
/* Returns FALSE on error.                                            */
/* ------------------------------------------------------------------ */
static BOOL ParseArgs(int argc, char **argv, CHKDSK_OPTS *pOpts)
{
    int     i;
    ULONG   ulDisk;
    ULONG   cbDisk;
    BOOL    fDriveSet = FALSE;

    memset(pOpts, 0, sizeof(*pOpts));

    /* Default to current drive (mirrors DosQueryCurrentDisk at CS:0x45) */
    DosQueryCurrentDisk(&ulDisk, &cbDisk);
    pOpts->ulDriveNum = ulDisk;   /* 1=A, 2=B, 3=C, ... */

    for (i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        /* Drive letter: single letter followed by colon, e.g. "C:" */
        if (strlen(arg) == 2 && isalpha((unsigned char)arg[0])
                              && arg[1] == ':')
        {
            if (fDriveSet)
            {
                fputs("CHKDSK32: Drive specified more than once.\n", stderr);
                return FALSE;
            }
            pOpts->ulDriveNum = (ULONG)(toupper((unsigned char)arg[0]) - 'A' + 1);
            fDriveSet = TRUE;
        }
        else if (arg[0] == '/' || arg[0] == '-')
        {
            if (!ParseSwitch(arg, pOpts))
                return FALSE;
        }
        else
        {
            fprintf(stderr, "CHKDSK32: Invalid parameter - %s\n", arg);
            return FALSE;
        }
    }

    return TRUE;
}

/* ------------------------------------------------------------------ */
/* Load CHKDSK32.DLL and call the main entry point.                  */
/* Mirrors the DosLoadModule/DosGetProcAddr sequence at CS:0x0288.   */
/* ------------------------------------------------------------------ */
static ULONG RunChkDsk(CHKDSK_OPTS *pOpts)
{
    HMODULE     hmod        = NULLHANDLE;
    PFNCHKDSK   pfnChkDsk   = NULL;
    CHAR        szFailName[CCHMAXPATH];
    APIRET      rc;
    ULONG       ulResult;

    /* Load the DLL (mirrors: 'DosLoadModule error: rc = %u, Mod= %s') */
    rc = DosLoadModule(szFailName, sizeof(szFailName),
                       CHKDSK_DLL_NAME, &hmod);
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "DosLoadModule error: rc = %u, Mod= %s\n",
                (unsigned)rc, szFailName);
        return CHKDSK_FATAL;
    }

    /* Get the entry point by ordinal 1                                */
    /* Mirrors: 'DosGetProcAddr Failed, rc = %d'                      */
    rc = DosQueryProcAddr(hmod, 1, NULL, (PFN *)&pfnChkDsk);
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "DosGetProcAddr Failed, rc = %d\n", (int)rc);
        DosFreeModule(hmod);
        return CHKDSK_FATAL;
    }

    /* Call through to the real implementation                         */
    /* Mirrors: far call at CS:0x02e9 via SS:[0x304] function pointer */
    ulResult = pfnChkDsk(pOpts->ulDriveNum,
                          pOpts->fFix,
                          pOpts->fVerbose,
                          pOpts->fNoConfirm,
                          pOpts->fAutoCheck);

    /* Free the DLL (mirrors: 'DosFreeModule Error, rc = %u')         */
    rc = DosFreeModule(hmod);
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "DosFreeModule Error, rc =  %u\n", (unsigned)rc);
        /* Non-fatal: return the check result anyway */
    }

    return ulResult;
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    CHKDSK_OPTS opts;
    ULONG       ulResult;

    /* Parse arguments */
    if (!ParseArgs(argc, argv, &opts))
    {
        Usage();
        return CHKDSK_FATAL;
    }

    /* In /AuToChEcK mode: force /F and suppress all prompts.
     * The IFS layer uses this for unattended boot-time checks.      */
    if (opts.fAutoCheck)
    {
        opts.fFix       = TRUE;
        opts.fNoConfirm = TRUE;
    }

    /* Load DLL and run check (mirrors CS:0x0288 dispatcher)          */
    ulResult = RunChkDsk(&opts);

    /* In /AuToChEcK mode, non-zero means errors were found/fixed.
     * Print a brief status for the boot log.                         */
    if (opts.fAutoCheck)
    {
        switch (ulResult)
        {
        case CHKDSK_OK:
            break;
        case CHKDSK_FIXED:
            fputs("CHKDSK32: Errors were found and corrected.\n", stdout);
            break;
        case CHKDSK_NOTFIXED:
            fputs("CHKDSK32: Errors were found but could not all be corrected.\n", stdout);
            break;
        default:
            fputs("CHKDSK32: A serious error occurred.\n", stderr);
            break;
        }
    }

    return (int)ulResult;
}
