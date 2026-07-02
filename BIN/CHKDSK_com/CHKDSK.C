/*
 * CHKDSK.C -- OS/2 Warp CHKDSK.COM replacement
 *
 * Based on osFree project design principles (OpenWatcom, OS/2 target).
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -wx -s -fo=CHKDSK.OBJ CHKDSK.C
 *   wlink @CHKDSK.LNK
 *
 * ============================================================
 * Architecture
 * ============================================================
 * The real CHKDSK.COM is a thin launcher that:
 *  1. Parses the command line (drive letter, /F, /C, /V flags)
 *  2. Determines which IFS (installable file system) owns the
 *     target drive using DosFSAttach / DosQueryFSAttach
 *  3. Loads the corresponding checker DLL (e.g. HPFS.IFS calls
 *     its own internal check, CHKDSK32.DLL for FAT, JFSCHK for JFS)
 *  4. For drives using the default FAT filesystem, loads
 *     CHKDSK32.DLL and calls its exported entry point
 *  5. For IFS drives, uses DosDevIOCtl (category 0x08) or the
 *     IFS-specific check interface
 *
 * This replacement:
 *  - Parses command line correctly
 *  - Queries the drive's filesystem type via DosQueryFSInfo
 *  - Attempts to load CHKDSK32.DLL for FAT/FAT32 drives
 *  - Falls back to a read-only disk statistics report (always safe)
 *  - Reports volume label, serial number, disk space
 *  - Outputs in the same format as IBM CHKDSK
 *
 * Flags supported:
 *   /F  -- fix errors (passed to CHKDSK32 if available)
 *   /C  -- skip cycle checking (HPFS specific, accepted/ignored)
 *   /V  -- verbose (show all files/directories)
 *   /?  -- help
 * ============================================================
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* Version                                                             */
/* ------------------------------------------------------------------ */
#define CHKDSK_VERSION  "1.00"

/* ------------------------------------------------------------------ */
/* Filesystem type names returned by DosQueryFSInfo level 2           */
/* ------------------------------------------------------------------ */
#define FSTYPE_FAT      "FAT"
#define FSTYPE_HPFS     "HPFS"
#define FSTYPE_JFS      "JFS"
#define FSTYPE_CDFS     "CDFS"
#define FSTYPE_RAMFS    "RAMFS"
#define FSTYPE_FAT32    "FAT32"
#define FSTYPE_NTFS     "NTFS"

/* ------------------------------------------------------------------ */
/* CHKDSK32.DLL exported entry point (for FAT volumes)                */
/* Ordinal 1: ChkDsk32(drive, fFix, fVerbose, pszReport, cbReport)   */
/* ------------------------------------------------------------------ */
typedef APIRET (APIENTRY *PFNCHKDSK32)(PSZ pszDrive, BOOL fFix,
                                        BOOL fVerbose,
                                        PSZ pszReport, ULONG cbReport);

/* ------------------------------------------------------------------ */
/* Parsed options                                                       */
/* ------------------------------------------------------------------ */
typedef struct {
    CHAR  szDrive[4];       /* e.g. "C:" */
    BOOL  fFix;             /* /F */
    BOOL  fVerbose;         /* /V */
    BOOL  fSkipCycles;      /* /C (HPFS specific) */
    BOOL  fHelp;            /* /? */
} CHKDSK_OPTS;

/* ------------------------------------------------------------------ */
/* Forward declarations                                                */
/* ------------------------------------------------------------------ */
static void  ShowHelp(void);
static BOOL  ParseArgs(int argc, char *argv[], CHKDSK_OPTS *pOpts);
static void  GetCurrentDrive(PSZ pszDrive);
static APIRET QueryFSType(PSZ pszDrive, PSZ pszFSType, ULONG cbFSType);
static APIRET ShowDiskInfo(PSZ pszDrive, BOOL fVerbose);
static APIRET RunChkDsk32(PSZ pszDrive, BOOL fFix, BOOL fVerbose);

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    CHKDSK_OPTS opts;
    CHAR        szFSType[32];
    APIRET      rc;

    memset(&opts, 0, sizeof(opts));

    printf("\n");
    printf("The CHKDSK utility\n");
    printf("Version " CHKDSK_VERSION " (cWarp/osFree replacement)\n");
    printf("\n");

    if (!ParseArgs(argc, argv, &opts))
        return 1;

    if (opts.fHelp)
    {
        ShowHelp();
        return 0;
    }

    /* Determine drive */
    if (!opts.szDrive[0])
        GetCurrentDrive(opts.szDrive);

    printf("Checking drive %s\n\n", opts.szDrive);

    /* Query filesystem type */
    rc = QueryFSType(opts.szDrive, szFSType, sizeof(szFSType));
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "SYS%04lu: Cannot query file system for drive %s\n",
                rc, opts.szDrive);
        return (int)rc;
    }

    printf("File system is %s.\n\n", szFSType);

    /* Route to appropriate checker */
    if (stricmp(szFSType, FSTYPE_FAT)   == 0 ||
        stricmp(szFSType, FSTYPE_FAT32) == 0)
    {
        /* Try CHKDSK32.DLL first */
        rc = RunChkDsk32(opts.szDrive, opts.fFix, opts.fVerbose);
        if (rc != NO_ERROR)
        {
            /* Fall back to read-only report */
            printf("CHKDSK32.DLL not available (rc=%lu), "
                   "performing read-only check.\n\n", rc);
            rc = ShowDiskInfo(opts.szDrive, opts.fVerbose);
        }
    }
    else if (stricmp(szFSType, FSTYPE_HPFS) == 0 ||
             stricmp(szFSType, FSTYPE_JFS)  == 0)
    {
        printf("For %s volumes, use the IFS-specific checker:\n", szFSType);
        if (stricmp(szFSType, FSTYPE_JFS) == 0)
            printf("  JFSCHK32.EXE %s%s\n",
                   opts.szDrive, opts.fFix ? " /F" : "");
        else
            printf("  HPFS.IFS provides checking via CHKDSK32.DLL\n");
        printf("\n");
        rc = ShowDiskInfo(opts.szDrive, opts.fVerbose);
    }
    else if (stricmp(szFSType, FSTYPE_CDFS) == 0)
    {
        printf("CHKDSK does not check CD-ROM file systems.\n\n");
        rc = ShowDiskInfo(opts.szDrive, opts.fVerbose);
    }
    else
    {
        printf("Unknown file system type '%s'.\n", szFSType);
        printf("No checker available for this volume.\n\n");
        rc = ShowDiskInfo(opts.szDrive, opts.fVerbose);
    }

    if (rc == NO_ERROR)
        printf("\nCHKDSK completed.\n");
    else
        fprintf(stderr, "\nCHKDSK completed with errors (rc=%lu).\n", rc);

    return (int)rc;
}

/* ------------------------------------------------------------------ */
/* ShowHelp                                                            */
/* ------------------------------------------------------------------ */
static void ShowHelp(void)
{
    printf("Checks a disk and displays a status report.\n\n");
    printf("CHKDSK [drive:] [/F] [/V] [/C]\n\n");
    printf("  drive:  Specifies the drive to check.\n");
    printf("  /F      Fixes errors on the disk.\n");
    printf("  /V      Displays the full path and name of every file\n");
    printf("          on the disk.\n");
    printf("  /C      Skips cycle checking within directory structures\n");
    printf("          (HPFS volumes only).\n\n");
    printf("If /F is not specified, CHKDSK reports errors but does not\n");
    printf("fix them.\n");
}

/* ------------------------------------------------------------------ */
/* ParseArgs                                                           */
/* ------------------------------------------------------------------ */
static BOOL ParseArgs(int argc, char *argv[], CHKDSK_OPTS *pOpts)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '/' || argv[i][0] == '-')
        {
            char flag = (char)toupper((unsigned char)argv[i][1]);
            switch (flag)
            {
                case 'F': pOpts->fFix        = TRUE; break;
                case 'V': pOpts->fVerbose    = TRUE; break;
                case 'C': pOpts->fSkipCycles = TRUE; break;
                case '?': pOpts->fHelp       = TRUE; return TRUE;
                default:
                    fprintf(stderr, "Invalid parameter '%s'.\n", argv[i]);
                    return FALSE;
            }
        }
        else if (strlen(argv[i]) >= 2 && argv[i][1] == ':')
        {
            /* Drive specification */
            pOpts->szDrive[0] = (char)toupper((unsigned char)argv[i][0]);
            pOpts->szDrive[1] = ':';
            pOpts->szDrive[2] = '\0';
        }
        else
        {
            fprintf(stderr, "Invalid parameter '%s'.\n", argv[i]);
            return FALSE;
        }
    }
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* GetCurrentDrive                                                     */
/* ------------------------------------------------------------------ */
static void GetCurrentDrive(PSZ pszDrive)
{
    ULONG ulDriveNum = 0;
    ULONG ulDriveMap = 0;

    DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap);
    pszDrive[0] = (char)('A' + ulDriveNum - 1);
    pszDrive[1] = ':';
    pszDrive[2] = '\0';
}

/* ------------------------------------------------------------------ */
/* QueryFSType - get filesystem name string for a drive               */
/* ------------------------------------------------------------------ */
static APIRET QueryFSType(PSZ pszDrive, PSZ pszFSType, ULONG cbFSType)
{
    BYTE    abBuf[512];
    PFSQBUFFER2 pFSQ = (PFSQBUFFER2)abBuf;
    ULONG   cbBuf    = sizeof(abBuf);
    CHAR    szPath[8];
    APIRET  rc;

    sprintf(szPath, "%s\\", pszDrive);

    memset(abBuf, 0, sizeof(abBuf));
    rc = DosQueryFSAttach(szPath, 0, FSAIL_QUERYNAME, pFSQ, &cbBuf);
    if (rc != NO_ERROR)
    {
        /* Try without trailing backslash */
        rc = DosQueryFSAttach(pszDrive, 0, FSAIL_QUERYNAME, pFSQ, &cbBuf);
    }

    if (rc == NO_ERROR)
    {
        /* szFSDName follows szName in the buffer */
        PSZ pszName    = (PSZ)(pFSQ->szName);
        PSZ pszFSDName = pszName + pFSQ->cbName + 1;
        strncpy(pszFSType, pszFSDName, cbFSType - 1);
        pszFSType[cbFSType - 1] = '\0';
    }
    else
    {
        /* Fallback: try DosQueryFSInfo for volume info */
        FSINFO fsi;
        memset(&fsi, 0, sizeof(fsi));
        rc = DosQueryFSInfo(pszDrive[0] - 'A' + 1, FSIL_VOLSER,
                            &fsi, sizeof(fsi));
        if (rc == NO_ERROR)
            strncpy(pszFSType, "FAT", cbFSType - 1);
        else
            strncpy(pszFSType, "Unknown", cbFSType - 1);
        pszFSType[cbFSType - 1] = '\0';
        rc = NO_ERROR;
    }

    return rc;
}

/* ------------------------------------------------------------------ */
/* ShowDiskInfo - read-only disk statistics report                    */
/* ------------------------------------------------------------------ */
static APIRET ShowDiskInfo(PSZ pszDrive, BOOL fVerbose)
{
    FSINFO      fsi;
    FSALLOCATE  fsal;
    APIRET      rc;
    ULONG       ulDriveNum;
    unsigned __int64 ullTotal, ullFree, ullUsed;
    CHAR        szVolLabel[13];
    CHAR        szSerial[12];

    ulDriveNum = (ULONG)(toupper((unsigned char)pszDrive[0]) - 'A' + 1);

    /* Volume label and serial number */
    memset(&fsi, 0, sizeof(fsi));
    rc = DosQueryFSInfo(ulDriveNum, FSIL_VOLSER, &fsi, sizeof(fsi));
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "SYS%04lu: Cannot query volume info for %s\n",
                rc, pszDrive);
        return rc;
    }

    strncpy(szVolLabel, fsi.vol.szVolLabel, sizeof(szVolLabel) - 1);
    szVolLabel[sizeof(szVolLabel) - 1] = '\0';

    /* Volume serial number: first ULONG in FSINFO buffer at FSIL_VOLSER.
     * fdateCreation is typed as FDATE struct, so read it as raw ULONG. */
    {
        ULONG ulVSN = 0;
        memcpy(&ulVSN, &fsi, sizeof(ULONG));  /* VSN is first 4 bytes */
        sprintf(szSerial, "%04X-%04X",
                (USHORT)(ulVSN >> 16),
                (USHORT)(ulVSN & 0xFFFF));
    }

    printf("Volume label is %s\n",
           szVolLabel[0] ? szVolLabel : "(none)");
    printf("Volume Serial Number is %s\n\n", szSerial);

    /* Disk allocation info */
    memset(&fsal, 0, sizeof(fsal));
    rc = DosQueryFSInfo(ulDriveNum, FSIL_ALLOC, &fsal, sizeof(fsal));
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "SYS%04lu: Cannot query disk allocation for %s\n",
                rc, pszDrive);
        return rc;
    }

    ullTotal = (unsigned __int64)fsal.cUnit
             * (unsigned __int64)fsal.cSectorUnit
             * (unsigned __int64)fsal.cbSector;

    ullFree  = (unsigned __int64)fsal.cUnitAvail
             * (unsigned __int64)fsal.cSectorUnit
             * (unsigned __int64)fsal.cbSector;

    ullUsed  = ullTotal - ullFree;

    printf("%12llu bytes total disk space\n",   ullTotal);
    printf("%12llu bytes in use\n",              ullUsed);
    printf("%12llu bytes available on disk\n\n", ullFree);

    printf("%12lu bytes in each allocation unit\n",
           (ULONG)fsal.cSectorUnit * (ULONG)fsal.cbSector);
    printf("%12lu total allocation units on disk\n",   fsal.cUnit);
    printf("%12lu allocation units available on disk\n\n", fsal.cUnitAvail);

    /* Note: full /V file listing would require recursive DosFind* */
    if (fVerbose)
        printf("Note: /V (verbose file listing) not yet implemented.\n\n");

    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* RunChkDsk32 - load CHKDSK32.DLL and call its entry point          */
/* ------------------------------------------------------------------ */
static APIRET RunChkDsk32(PSZ pszDrive, BOOL fFix, BOOL fVerbose)
{
    HMODULE      hmod   = NULLHANDLE;
    PFNCHKDSK32  pfnChk = NULL;
    CHAR         szErr[256];
    CHAR         szReport[65536];
    APIRET       rc;

    rc = DosLoadModule(szErr, sizeof(szErr), "CHKDSK32", &hmod);
    if (rc != NO_ERROR)
        return rc;

    rc = DosQueryProcAddr(hmod, 1, NULL, (PFN *)&pfnChk);
    if (rc != NO_ERROR || !pfnChk)
    {
        DosFreeModule(hmod);
        return rc ? rc : ERROR_INVALID_FUNCTION;
    }

    szReport[0] = '\0';
    rc = pfnChk(pszDrive, fFix, fVerbose, szReport, sizeof(szReport));

    if (szReport[0])
        printf("%s\n", szReport);

    DosFreeModule(hmod);
    return rc;
}
