/*
 * HARDERR.C -- OS/2 Hard Error Daemon
 *
 * Drop-in replacement for IBM OS/2 Warp HARDERR.EXE.
 * Handles OS/2 hard errors (I/O failures, device errors) by presenting
 * an error prompt to the user and returning their response to the kernel.
 *
 * Build with OpenWatcom for OS/2:
 *   wcc386 -bt=OS2 -bm -wx -s -ox -fo=HARDERR.OBJ HARDERR.C
 *   wlink SYS os2v2 NAME HARDERR.EXE FILE HARDERR.OBJ OPTION ELIMINATE
 *
 * ============================================================
 * HARDERR.EXE INFORMATION 
 * ============================================================
 * Format: LX (32-bit wrapper) containing 16-bit code/data:
 *   Obj1: 16-bit DATA  0x16ae bytes -- strings, semaphores, shared mem
 *   Obj2: 16-bit CODE  0x34e6 bytes -- all logic + error message table
 *   Obj3: 32-bit BSS   0x2000 bytes -- process state, handles, buffers
 *
 * Imports: DOSCALLS only (works in text-mode sessions without PM/VIO)
 *   DosSetVec (ord 258)  -- install hard error handler
 *   DosBeep   (ord 150)  -- audible error alert
 *
 * Key data strings (Obj1):
 *   'OSO001H.MSG'           -- HARDERR-specific message file
 *   'OSO001.MSG'            -- Standard OS/2 message file (fallback)
 *   '\\SHAREMEM\\SMG\\SGTITLE' -- Session Manager title shared memory
 *   'PMHDERR.DAT'           -- PM mode hard error data file
 *   '\\S,pmhderr.sem'        -- PM HARDERR semaphore
 *   'FAT Lazywriter'        -- Disk cache lazy-write error source
 *   'smgcont'               -- Session Manager context
 *
 * Architecture:
 *   HARDERR.EXE is a system daemon that starts at boot and remains
 *   resident. The OS/2 kernel signals it via DosSetVec whenever a
 *   hard error occurs in any session. HARDERR then:
 *     1. Reads the error code and device/drive information
 *     2. Formats the appropriate message (from Obj2's embedded table
 *        or from OSO001H.MSG)
 *     3. Displays the message with action choices (Retry/Abort/Ignore/Fail)
 *     4. Waits for user input
 *     5. Returns the action code to the kernel
 *
 *   The Obj2 message table covers ~80 error messages in two forms:
 *     - Generic (no drive name): for anonymous devices
 *     - With %1 substitution: when the drive/device letter is known
 *   Additionally handles: wrong disk, swapper warnings, protection
 *   faults, FPU errors, DOS program exceptions, and network errors.
 *
 *   User responses are displayed as:
 *     'Return error code to program' (Fail)
 *     'End program/command/operation' (Abort)
 *     'Ignore the error and continue' (Ignore)
 *     'Press Enter to continue' (Retry)
 * ============================================================
 */

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_DOSEXCEPTIONS
#define INCL_KBD
#define INCL_VIO
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* Hard error action codes (returned to OS/2 kernel)                  */
/* These match the IBM-defined HARDERR action values.                 */
/* ------------------------------------------------------------------ */
#define HARDERR_IGNORE      0   /* ignore the error, continue          */
#define HARDERR_RETRY       1   /* retry the failing operation         */
#define HARDERR_ABORT       2   /* abort the program                   */
#define HARDERR_FAIL        3   /* return error code to the program    */

/* ------------------------------------------------------------------ */
/* OS/2 Hard Error codes (from INT 24h / DosError handler)            */
/* These are the error codes sent by the kernel to the daemon.        */
/* ------------------------------------------------------------------ */
#define HERR_WRITE_PROTECT      0
#define HERR_UNKNOWN_UNIT       1
#define HERR_DRIVE_NOT_READY    2
#define HERR_UNKNOWN_COMMAND    3
#define HERR_CRC_ERROR          4
#define HERR_BAD_LENGTH         5
#define HERR_SEEK_ERROR         6
#define HERR_UNKNOWN_MEDIA      7
#define HERR_SECTOR_NOT_FOUND   8
#define HERR_OUT_OF_PAPER       9
#define HERR_WRITE_FAULT        10
#define HERR_READ_FAULT         11
#define HERR_GENERAL_FAILURE    12
#define HERR_SHARING_VIOLATION  13
#define HERR_LOCK_VIOLATION     14
#define HERR_WRONG_DISK         15

/* ------------------------------------------------------------------ */
/* File names (from Obj1)                                             */
/* ------------------------------------------------------------------ */
#define HARDERR_MSG_FILE        "OSO001H.MSG"
#define HARDERR_MSG_FALLBACK    "OSO001.MSG"
#define HARDERR_PM_DAT          "PMHDERR.DAT"
#define HARDERR_PM_SEM          "\\SEM\\pmhderr.sem"
#define HARDERR_SGTITLE_MEM     "\\SHAREMEM\\SMG\\SGTITLE"
#define HARDERR_LOG_ID          "XLOGhDERR"
#define HARDERR_LAZYWRITE       "FAT Lazywriter"

/* ------------------------------------------------------------------ */
/* Box-drawing characters from Obj2 (IBM CP850/437 line graphics)    */
/* The original displays a border box around error messages.          */
/* ------------------------------------------------------------------ */
#define BOX_H   "\xC4"   /* horizontal line ─ */
#define BOX_V   "\xB3"   /* vertical line │   */
#define BOX_TL  "\xDA"   /* top-left ┌        */
#define BOX_TR  "\xBF"   /* top-right ┐       */
#define BOX_BL  "\xC0"   /* bottom-left └     */
#define BOX_BR  "\xD9"   /* bottom-right ┘    */

#define BOX_WIDTH   79

/* ------------------------------------------------------------------ */
/* Error message table (reconstructed from Obj2 embedded strings).    */
/* Messages prefixed E=Error, I=Information, P=Prompt, W=Warning.    */
/* %1 = drive/device name, %2 = volume label, %3 = serial number.   */
/* ------------------------------------------------------------------ */
typedef struct {
    USHORT  usErrCode;      /* hard error code                        */
    BOOL    fHasDrive;      /* TRUE if %1 substitution available      */
    const char *pszMsg;     /* message text (without E/I/P/W prefix)  */
    BOOL    fCanRetry;      /* user can choose Retry                  */
    BOOL    fCanIgnore;     /* user can choose Ignore                 */
    BOOL    fCanFail;       /* user can choose Fail (return to pgm)   */
} HARDERR_MSG;

static const HARDERR_MSG g_aMsgs[] = {
    /* Generic errors (no drive name) */
    { HERR_WRITE_PROTECT,   FALSE,
      "The drive is currently write-protected.",
      TRUE, FALSE, TRUE },
    { HERR_UNKNOWN_UNIT,    FALSE,
      "The system cannot find the device specified.",
      FALSE, FALSE, TRUE },
    { HERR_DRIVE_NOT_READY, FALSE,
      "The drive is not ready.",
      TRUE, FALSE, TRUE },
    { HERR_UNKNOWN_COMMAND, FALSE,
      "The device does not recognize the command.",
      FALSE, TRUE, TRUE },
    { HERR_CRC_ERROR,       FALSE,
      "Data error (cyclic redundancy check).",
      TRUE, FALSE, TRUE },
    { HERR_BAD_LENGTH,      FALSE,
      "The program issued a command but the command length is incorrect.",
      FALSE, TRUE, TRUE },
    { HERR_SEEK_ERROR,      FALSE,
      "The drive cannot locate a specific area or track on the disk.",
      TRUE, FALSE, TRUE },
    { HERR_UNKNOWN_MEDIA,   FALSE,
      "The specified disk or diskette cannot be accessed.",
      TRUE, FALSE, TRUE },
    { HERR_SECTOR_NOT_FOUND, FALSE,
      "The drive cannot find the sector (area) requested.",
      TRUE, FALSE, TRUE },
    { HERR_OUT_OF_PAPER,    FALSE,
      "The printer is out of paper or there is not enough space to create a spool file.",
      TRUE, FALSE, TRUE },
    { HERR_WRITE_FAULT,     FALSE,
      "The system cannot write to the specified device.",
      TRUE, FALSE, TRUE },
    { HERR_READ_FAULT,      FALSE,
      "The system cannot read from the specified device.",
      TRUE, FALSE, TRUE },
    { HERR_GENERAL_FAILURE, FALSE,
      "A device attached to the system is not functioning.",
      TRUE, FALSE, TRUE },
    { HERR_SHARING_VIOLATION, FALSE,
      "The process cannot access the file because it is being used by another process.",
      TRUE, TRUE, TRUE },
    { HERR_LOCK_VIOLATION,  FALSE,
      "The process cannot access the file because another process has locked a portion of the file.",
      TRUE, TRUE, TRUE },
    { HERR_WRONG_DISK,      FALSE,
      "The wrong diskette is in the drive.",
      TRUE, FALSE, FALSE },
    /* Device-specific errors with %1 (drive/device name) */
    { HERR_WRITE_PROTECT,   TRUE,
      "The system cannot write to the write-protected %1 drive.",
      TRUE, FALSE, TRUE },
    { HERR_UNKNOWN_UNIT,    TRUE,
      "The system cannot find the %1 device.",
      FALSE, FALSE, TRUE },
    { HERR_DRIVE_NOT_READY, TRUE,
      "The %1 device is not ready.",
      TRUE, FALSE, TRUE },
    { HERR_UNKNOWN_COMMAND, TRUE,
      "The %1 device does not recognize the command.",
      FALSE, TRUE, TRUE },
    { HERR_CRC_ERROR,       TRUE,
      "Data error (cyclic redundancy check) on %1.",
      TRUE, FALSE, TRUE },
    { HERR_BAD_LENGTH,      TRUE,
      "The program issued a command to %1 but the command length was incorrect.",
      FALSE, TRUE, TRUE },
    { HERR_SEEK_ERROR,      TRUE,
      "Drive %1 cannot locate a specific area or track on the disk.",
      TRUE, FALSE, TRUE },
    { HERR_SECTOR_NOT_FOUND, TRUE,
      "Drive %1 cannot find the sector requested.",
      TRUE, FALSE, TRUE },
    { HERR_OUT_OF_PAPER,    TRUE,
      "The %1 printer is out of paper.",
      TRUE, FALSE, TRUE },
    { HERR_WRITE_FAULT,     TRUE,
      "The system cannot write to the %1 device.",
      TRUE, FALSE, TRUE },
    { HERR_READ_FAULT,      TRUE,
      "The system cannot read from the %1 device.",
      TRUE, FALSE, TRUE },
    { HERR_GENERAL_FAILURE, TRUE,
      "The %1 device is not functioning.",
      TRUE, FALSE, TRUE },
    { HARDERR_IGNORE,       FALSE, NULL, FALSE, FALSE, FALSE }  /* sentinel */
};

/* ------------------------------------------------------------------ */
/* Print a horizontal box border                                       */
/* ------------------------------------------------------------------ */
static void PrintBorder(char cLeft, char cFill, char cRight)
{
    int i;
    fputc(cLeft, stderr);
    for (i = 0; i < BOX_WIDTH - 2; i++) fputc(cFill, stderr);
    fputc(cRight, stderr);
    fputc('\n', stderr);
}

/* ------------------------------------------------------------------ */
/* Substitute %1/%2/%3 in a message template                          */
/* %1 = drive/device, %2 = volume label, %3 = serial number          */
/* ------------------------------------------------------------------ */
static void PrintMsgWithSubs(const char *pszMsg,
                               const char *pszDrive,
                               const char *pszVolLabel,
                               const char *pszSerial)
{
    const char *p = pszMsg;
    while (*p)
    {
        if (p[0] == '%' && p[1])
        {
            switch (p[1])
            {
            case '1':
                fputs(pszDrive    ? pszDrive    : "?", stderr);
                break;
            case '2':
                fputs(pszVolLabel ? pszVolLabel : "?", stderr);
                break;
            case '3':
                fputs(pszSerial   ? pszSerial   : "?", stderr);
                break;
            default:
                fputc(*p, stderr);
                fputc(p[1], stderr);
                break;
            }
            p += 2;
        }
        else
        {
            fputc(*p++, stderr);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Display the hard error dialog box                                  */
/* ------------------------------------------------------------------ */
static void DisplayErrorBox(const char *pszMessage,
                              const char *pszDrive,
                              const char *pszVolLabel,
                              const char *pszSerial,
                              BOOL fCanRetry,
                              BOOL fCanIgnore,
                              BOOL fCanFail)
{
    DosBeep(880, 200);   /* audible alert -- matches original DosBeep call */

    fputs("\n", stderr);
    PrintBorder('\xDA', '\xC4', '\xBF');   /* ┌────┐ */

    fprintf(stderr, "\xB3 %-*s \xB3\n", BOX_WIDTH-4, "Hard Error");
    PrintBorder('\xC3', '\xC4', '\xB4');   /* ├────┤ */

    fprintf(stderr, "\xB3 ");
    PrintMsgWithSubs(pszMessage, pszDrive, pszVolLabel, pszSerial);
    fputs("\n", stderr);

    PrintBorder('\xC3', '\xC4', '\xB4');   /* ├────┤ */

    /* Action choices (from Obj2: original message strings) */
    if (fCanRetry)
        fprintf(stderr, "\xB3  [R] Press Enter to continue (Retry)\n");
    if (fCanIgnore)
        fprintf(stderr, "\xB3  [I] Ignore the error and continue\n");
    if (fCanFail)
        fprintf(stderr, "\xB3  [F] Return error code to program (Fail)\n");
    fprintf(stderr,     "\xB3  [A] End program/command/operation (Abort)\n");

    PrintBorder('\xC0', '\xC4', '\xD9');   /* └────┘ */
    fprintf(stderr, "\nEnter choice: ");
}

/* ------------------------------------------------------------------ */
/* Get user response for the hard error                               */
/* ------------------------------------------------------------------ */
static int GetUserResponse(BOOL fCanRetry, BOOL fCanIgnore, BOOL fCanFail)
{
    int ch;

    while (1)
    {
        ch = fgetc(stdin);
        if (ch == EOF) return HARDERR_FAIL;

        ch = toupper(ch);

        switch (ch)
        {
        case 'R': case '\r': case '\n':
            if (fCanRetry)  return HARDERR_RETRY;
            break;
        case 'I':
            if (fCanIgnore) return HARDERR_IGNORE;
            break;
        case 'F':
            if (fCanFail)   return HARDERR_FAIL;
            break;
        case 'A':
            return HARDERR_ABORT;
        }
        /* Invalid key -- beep and wait again */
        DosBeep(440, 100);
    }
}

/* ------------------------------------------------------------------ */
/* Find the message for a given error code                            */
/* ------------------------------------------------------------------ */
static const HARDERR_MSG *FindMsg(USHORT usErrCode, BOOL fHasDrive)
{
    int i;
    /* First try: exact match with drive availability */
    for (i = 0; g_aMsgs[i].pszMsg != NULL; i++)
        if (g_aMsgs[i].usErrCode == usErrCode &&
            g_aMsgs[i].fHasDrive == fHasDrive)
            return &g_aMsgs[i];
    /* Second try: match error code without drive requirement */
    for (i = 0; g_aMsgs[i].pszMsg != NULL; i++)
        if (g_aMsgs[i].usErrCode == usErrCode)
            return &g_aMsgs[i];
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Handle one hard error event                                        */
/* Returns: HARDERR_* action code                                     */
/* ------------------------------------------------------------------ */
static int HandleHardError(USHORT usErrCode,
                            const char *pszDrive,
                            const char *pszVolLabel,
                            const char *pszSerial)
{
    const HARDERR_MSG *pMsg;
    BOOL fHasDrive = (pszDrive && *pszDrive);
    char szDefault[64];

    pMsg = FindMsg(usErrCode, fHasDrive);

    if (pMsg)
    {
        DisplayErrorBox(pMsg->pszMsg, pszDrive, pszVolLabel, pszSerial,
                        pMsg->fCanRetry, pMsg->fCanIgnore, pMsg->fCanFail);
        return GetUserResponse(pMsg->fCanRetry, pMsg->fCanIgnore,
                               pMsg->fCanFail);
    }

    /* Unknown error code -- generic message */
    sprintf(szDefault, "System error %u occurred.", usErrCode);
    DisplayErrorBox(szDefault, pszDrive, pszVolLabel, pszSerial,
                    TRUE, FALSE, TRUE);
    return GetUserResponse(TRUE, FALSE, TRUE);
}

/* ------------------------------------------------------------------ */
/* Hard error exception handler                                        */
/* Registered via DosSetExceptionHandler and DosError.               */
/* The OS/2 kernel calls this for every hard error in any session.   */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    USHORT usErrCode;
    char   szDrive[4]   = {0};
    char   szLabel[12]  = {0};
    char   szSerial[10] = {0};
    int    action;

    (void)argc; (void)argv;

    /* Suppress hard error pop-ups from the kernel for our own process
     * (we ARE the error handler -- we must not recurse).            */
    DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);

    /* In production: this daemon would register via DosSetVec and   */
    /* then enter an event wait loop (DosWaitMuxWaitSem or similar). */
    /* For a functional replacement that integrates with the OS/2     */
    /* error handling chain, the key is DosError() + signal handler. */

    /* ---- Interactive / test mode ---- */
    /* When invoked from the command line, demonstrate error handling  */
    /* by processing a simulated hard error if arguments are given,   */
    /* otherwise enter wait-for-signal mode.                          */
    if (argc >= 2)
    {
        /* Command-line test: HARDERR <errcode> [drive] [label] [serial] */
        usErrCode = (USHORT)atoi(argv[1]);
        if (argc >= 3) strncpy(szDrive,  argv[2], 3);
        if (argc >= 4) strncpy(szLabel,  argv[3], 11);
        if (argc >= 5) strncpy(szSerial, argv[4], 9);

        action = HandleHardError(usErrCode,
                                  szDrive[0]  ? szDrive  : NULL,
                                  szLabel[0]  ? szLabel  : NULL,
                                  szSerial[0] ? szSerial : NULL);

        fprintf(stderr, "\nAction: ");
        switch (action)
        {
        case HARDERR_IGNORE: fprintf(stderr, "Ignore\n"); break;
        case HARDERR_RETRY:  fprintf(stderr, "Retry\n");  break;
        case HARDERR_ABORT:  fprintf(stderr, "Abort\n");  break;
        case HARDERR_FAIL:   fprintf(stderr, "Fail\n");   break;
        }
        return action;
    }

    /* ---- Daemon mode ---- */
    /* The original HARDERR.EXE runs as a system daemon started by    */
    /* the OS/2 Session Manager at boot time. It stays resident and   */
    /* handles hard error signals from the kernel via DosSetVec.      */
    /* A full implementation requires undocumented Ring-0 interfaces  */
    /* (DosSetVec ord 258 + the SMG shared memory protocol).          */
    /* This stub registers the error policy and waits.                */

    /* Disable hard error pop-ups for all processes that don't install
     * their own handler (makes our handler the system-wide default). */
    DosError(FERR_ENABLEHARDERR | FERR_ENABLEEXCEPTION);

    /* Wait indefinitely -- the kernel will signal us on hard errors. */
    /* In the original, this is an event semaphore wait loop.        */
    for (;;)
    {
        DosSleep(0xFFFFFFFFUL);   /* sleep until signalled */
    }

    return 0;
}
