/*
 * spl1b.c - SPL1B.DLL replacement
 *
 * OS/2 Spooler Backend DLL.
 * Loaded by SPOOL.EXE to provide the in-process job scheduling engine,
 * INI-based queue/device database, and spool file management.
 *
 * Build: OpenWatcom  wcl386 -bt=os2 -bd -l=os2v2_dll spl1b.c
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS
#include <os2.h>
#include <pmspl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../include/spooler_ipc.h"

/* ------------------------------------------------------------------ */
/* Spooler return codes referenced below that the OS/2 Toolkit does    */
/* not define under these names. Map each to the closest real PM       */
/* spooler (PMERR_SPL_*) or base OS/2 error so callers see standard    */
/* values. (The genuine PMERR_SPL_* names resolve via <pmspl.h> +      */
/* INCL_SPLERRORS; base errors via INCL_DOSERRORS.)                    */
/* ------------------------------------------------------------------ */
#ifndef PMERR_SPL_INV_PARM
#define PMERR_SPL_INV_PARM               ERROR_INVALID_PARAMETER         /* bad argument (as DosPrint* return) */
#endif
#ifndef PMERR_SPL_PRINTER_ALREADY_EXISTS
#define PMERR_SPL_PRINTER_ALREADY_EXISTS PMERR_SPL_DEVICE_ALREADY_EXISTS /* a printer is a print device */
#endif
#ifndef PMERR_SPL_INV_PRINTER_NAME
#define PMERR_SPL_INV_PRINTER_NAME       PMERR_SPL_PRINTER_NOT_FOUND     /* device lookup failed */
#endif
#ifndef PMERR_SPL_PORT_ALREADY_EXISTS
#define PMERR_SPL_PORT_ALREADY_EXISTS    ERROR_ALREADY_EXISTS            /* no port-specific PMERR exists */
#endif
#ifndef PMERR_SPL_INV_PORT_NAME
#define PMERR_SPL_INV_PORT_NAME          PMERR_SPL_NO_SUCH_LOG_ADDRESS   /* a port is a logical address */
#endif

/* ------------------------------------------------------------------ */
/* Internal state                                                      */
/* ------------------------------------------------------------------ */

static HMTX     g_hmtxDB    = NULLHANDLE; /* serialises queue/job tables */
static SPLQUEUE g_queues[SPL_MAX_QUEUES];
static SPLJOB   g_jobs[SPL_MAX_JOBS];
static SPLDEVICE g_devices[SPL_MAX_DEVICES];
static USHORT   g_cQueues   = 0;
static USHORT   g_cJobs     = 0;
static USHORT   g_cDevices  = 0;
static USHORT   g_nextJobId = 1;
static CHAR     g_szSpoolDir[CCHMAXPATH];

/* ------------------------------------------------------------------ */
/* DLL init/term                                                       */
/* ------------------------------------------------------------------ */

ULONG _System _DLL_InitTerm(ULONG hModule, ULONG ulFlag)
{
    ULONG ulBootDrive;
    if (ulFlag == 0) {
        /* Initialisation */
        DosCreateMutexSem(NULL, &g_hmtxDB, 0, FALSE);
        memset(g_queues,  0, sizeof(g_queues));
        memset(g_jobs,    0, sizeof(g_jobs));
        memset(g_devices, 0, sizeof(g_devices));
        g_cQueues   = 0;
        g_cJobs     = 0;
        g_cDevices  = 0;
        g_nextJobId = 1;
        /* Build spool directory path on boot drive */
        ulBootDrive = 0;
        DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                        &ulBootDrive, sizeof(ulBootDrive));
        sprintf(g_szSpoolDir, "%c:%s",
                (CHAR)('A' + ulBootDrive - 1),
                SPL_SPOOL_DIR);
        DosCreateDir(g_szSpoolDir, NULL);
    } else {
        /* Termination */
        if (g_hmtxDB != NULLHANDLE) {
            DosCloseMutexSem(g_hmtxDB);
            g_hmtxDB = NULLHANDLE;
        }
    }
    return 1;
}

/* Forward declarations for INI loader helpers defined later in this file */
static VOID Spl1bLoadQueuesFromINI(VOID);
static VOID Spl1bLoadDevicesFromINI(VOID);
static VOID Spl1bLoadPortsFromINI(VOID);

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static PSPLJOB Spl1bFindJob(USHORT uJobId)
{
    USHORT i;
    for (i = 0; i < g_cJobs; i++)
        if (g_jobs[i].uJobId == uJobId)
            return &g_jobs[i];
    return NULL;
}

static PSPLQUEUE Spl1bFindQueue(PSZ pszName)
{
    USHORT i;
    for (i = 0; i < g_cQueues; i++)
        if (stricmp(g_queues[i].szName, pszName) == 0)
            return &g_queues[i];
    return NULL;
}

static PSPLDEVICE Spl1bFindDevice(PSZ pszName)
{
    USHORT i;
    for (i = 0; i < g_cDevices; i++)
        if (stricmp(g_devices[i].szName, pszName) == 0)
            return &g_devices[i];
    return NULL;
}

static USHORT Spl1bAllocJobId(VOID)
{
    USHORT id = g_nextJobId++;
    if (g_nextJobId == 0) g_nextJobId = 1; /* wrap, skip 0 */
    return id;
}

static VOID Spl1bSpoolFileName(USHORT uJobId, PSZ pszBuf, ULONG cbBuf)
{
    snprintf(pszBuf, cbBuf, "%sSPL%04X.TMP", g_szSpoolDir, uJobId);
}

/* ------------------------------------------------------------------ */
/* INI persistence helpers (OS2SYS.INI)                               */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* INI key names for device and port tables                            */
/* ------------------------------------------------------------------ */
#define SPL_INI_DEVICE          "PM_SPOOLER_PRINTER"
#define SPL_INI_DEVICE_LOGADDR  "PM_SPOOLER_PRINTER_ADDR"
#define SPL_INI_DEVICE_COMMENT  "PM_SPOOLER_PRINTER_COMMENT"
#define SPL_INI_DEVICE_DRIVERS  "PM_SPOOLER_PRINTER_DRIVERS"
#define SPL_INI_PORT            "PM_SPOOLER_PORT"
#define SPL_INI_PORT_DRIVER     "PM_SPOOLER_PORT_DRIVER"
#define SPL_INI_PORT_PROTOCOL   "PM_SPOOLER_PORT_PROTOCOL"

/* ------------------------------------------------------------------ */
/* Port table (in-memory + INI-backed)                                 */
/* ------------------------------------------------------------------ */

#define SPL_MAX_PORTS 32

typedef struct _SPLPORT {
    CHAR  szName[PDLEN+1];
    CHAR  szDriver[DRIV_NAME_SIZE+DRIV_DEVICENAME_SIZE+2];
    CHAR  szProtocol[CCHMAXPATH];
    ULONG ulMode;
    ULONG ulPriority;
} SPLPORT;
typedef SPLPORT *PSPLPORT;

static SPLPORT g_ports[SPL_MAX_PORTS];
static USHORT  g_cPorts = 0;

static PSPLPORT Spl1bFindPort(PSZ pszName)
{
    USHORT i;
    for (i = 0; i < g_cPorts; i++)
        if (stricmp(g_ports[i].szName, pszName) == 0)
            return &g_ports[i];
    return NULL;
}

static VOID Spl1bSavePortToINI(PSPLPORT pP)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    if (hini == NULLHANDLE) return;
    PrfWriteProfileString(hini, SPL_INI_PORT,         pP->szName, pP->szDriver);
    PrfWriteProfileString(hini, SPL_INI_PORT_DRIVER,  pP->szName, pP->szDriver);
    PrfWriteProfileString(hini, SPL_INI_PORT_PROTOCOL,pP->szName, pP->szProtocol);
    PrfCloseProfile(hini);
}

static VOID Spl1bDeletePortFromINI(PSZ pszName)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    if (hini == NULLHANDLE) return;
    PrfWriteProfileString(hini, SPL_INI_PORT,         pszName, NULL);
    PrfWriteProfileString(hini, SPL_INI_PORT_DRIVER,  pszName, NULL);
    PrfWriteProfileString(hini, SPL_INI_PORT_PROTOCOL,pszName, NULL);
    PrfCloseProfile(hini);
}

static VOID Spl1bLoadPortsFromINI(VOID)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    CHAR buf[4096];
    PSZ  psz;

    if (hini == NULLHANDLE) return;
    memset(buf, 0, sizeof(buf));
    if (!PrfQueryProfileString(hini, SPL_INI_PORT, NULL, "", buf, sizeof(buf))) {
        /* If no ports saved yet, seed with LPT1 and LPT2 */
        PSPLPORT pP;
        pP = &g_ports[g_cPorts++];
        memset(pP, 0, sizeof(*pP)); strcpy(pP->szName, "LPT1"); strcpy(pP->szDriver, "LPTDD$");
        pP = &g_ports[g_cPorts++];
        memset(pP, 0, sizeof(*pP)); strcpy(pP->szName, "LPT2"); strcpy(pP->szDriver, "LPTDD$");
        pP = &g_ports[g_cPorts++];
        memset(pP, 0, sizeof(*pP)); strcpy(pP->szName, "COM1"); strcpy(pP->szDriver, "SERIAL$");
        PrfCloseProfile(hini);
        return;
    }
    for (psz = buf; *psz && g_cPorts < SPL_MAX_PORTS; psz += strlen(psz) + 1) {
        PSPLPORT pP = &g_ports[g_cPorts++];
        memset(pP, 0, sizeof(*pP));
        strncpy(pP->szName, psz, PDLEN);
        PrfQueryProfileString(hini, SPL_INI_PORT_DRIVER,   psz, "",
                              pP->szDriver,   sizeof(pP->szDriver));
        PrfQueryProfileString(hini, SPL_INI_PORT_PROTOCOL, psz, "",
                              pP->szProtocol, sizeof(pP->szProtocol));
    }
    PrfCloseProfile(hini);
}

/* ------------------------------------------------------------------ */
/* Device INI helpers                                                  */
/* ------------------------------------------------------------------ */

static VOID Spl1bSaveQueueToINI(PSPLQUEUE pQ)
{
    HAB hab = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    if (hini == NULLHANDLE) return;
    PrfWriteProfileString(hini, SPL_INI_QUEUE, pQ->szName, pQ->szPrinters);
    PrfWriteProfileString(hini, SPL_INI_QUEUEDESCR,  pQ->szName, pQ->szComment);
    PrfWriteProfileString(hini, SPL_INI_QUEUEDD,     pQ->szName, pQ->szDriverName);
    PrfCloseProfile(hini);
}

static VOID Spl1bDeleteQueueFromINI(PSZ pszName)
{
    HAB hab = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    if (hini == NULLHANDLE) return;
    PrfWriteProfileString(hini, SPL_INI_QUEUE,      pszName, NULL);
    PrfWriteProfileString(hini, SPL_INI_QUEUEDESCR, pszName, NULL);
    PrfWriteProfileString(hini, SPL_INI_QUEUEDD,    pszName, NULL);
    PrfCloseProfile(hini);
}

static VOID Spl1bLoadQueuesFromINI(VOID)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    CHAR buf[4096];
    ULONG cb = sizeof(buf);
    PSZ  psz;

    if (hini == NULLHANDLE) return;
    memset(buf, 0, sizeof(buf));
    if (!PrfQueryProfileString(hini, SPL_INI_QUEUE, NULL, "", buf, cb)) {
        PrfCloseProfile(hini);
        return;
    }
    /* buf contains null-separated queue names, double-null terminated */
    for (psz = buf; *psz && g_cQueues < SPL_MAX_QUEUES; psz += strlen(psz) + 1) {
        PSPLQUEUE pQ = &g_queues[g_cQueues++];
        memset(pQ, 0, sizeof(*pQ));
        strncpy(pQ->szName, psz, QNLEN);
        PrfQueryProfileString(hini, SPL_INI_QUEUEDESCR,
                              psz, "", pQ->szComment, sizeof(pQ->szComment));
        PrfQueryProfileString(hini, SPL_INI_QUEUEDD,
                              psz, "", pQ->szDriverName, sizeof(pQ->szDriverName));
        pQ->uPriority = PRQ_DEF_PRIORITY;
        pQ->fsStatus  = PRQ_ACTIVE;
    }
    PrfCloseProfile(hini);
}

/* ------------------------------------------------------------------ */
/* Exported backend API (called by SPOOL.EXE dispatcher)              */
/* ------------------------------------------------------------------ */

/*
 * Spl1bInit - called by SPOOL.EXE at startup to load persistent state.
 */
BOOL APIENTRY Spl1bInit(PSZ pszSpoolDir)
{
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    if (pszSpoolDir && *pszSpoolDir)
        strncpy(g_szSpoolDir, pszSpoolDir, sizeof(g_szSpoolDir) - 1);
    Spl1bLoadQueuesFromINI();
    Spl1bLoadDevicesFromINI();
    Spl1bLoadPortsFromINI();
    DosReleaseMutexSem(g_hmtxDB);
    return TRUE;
}

/*
 * Spl1bOpenJob - begin a new spool job; returns job ID or 0 on error.
 */
USHORT APIENTRY Spl1bOpenJob(PSZ pszQueueName, PSZ pszDocName,
                              PSZ pszUserName,  PSZ pszDataType,
                              PSZ pszDriverName, PHFILE phFile)
{
    PSPLQUEUE pQ;
    PSPLJOB   pJ;
    USHORT    uJobId;
    CHAR      szFile[CCHMAXPATH];
    ULONG     ulAction;
    APIRET    rc;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);

    pQ = Spl1bFindQueue(pszQueueName);
    if (!pQ || (pQ->fsStatus & PRQ3_PAUSED)) {
        DosReleaseMutexSem(g_hmtxDB);
        return 0;
    }
    if (g_cJobs >= SPL_MAX_JOBS) {
        DosReleaseMutexSem(g_hmtxDB);
        return 0;
    }

    uJobId = Spl1bAllocJobId();
    Spl1bSpoolFileName(uJobId, szFile, sizeof(szFile));

    rc = DosOpen(szFile, phFile, &ulAction, 0,
                 FILE_NORMAL,
                 OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREADWRITE, NULL);
    if (rc != NO_ERROR) {
        DosReleaseMutexSem(g_hmtxDB);
        return 0;
    }

    pJ = &g_jobs[g_cJobs++];
    memset(pJ, 0, sizeof(*pJ));
    pJ->uJobId     = uJobId;
    pJ->uPriority  = PRJ_MIN_PRIORITY;
    pJ->fsStatus   = PRJ_QS_SPOOLING;
    pJ->ulSubmitted = (ULONG)time(NULL);
    strncpy(pJ->szQueueName,  pszQueueName  ? pszQueueName  : "", QNLEN);
    strncpy(pJ->szDocName,    pszDocName    ? pszDocName    : "", CCHMAXPATH-1);
    strncpy(pJ->szUserName,   pszUserName   ? pszUserName   : "", UNLEN);
    strncpy(pJ->szDataType,   pszDataType   ? pszDataType   : "PM_Q_STD", DTLEN);
    strncpy(pJ->szDriverName, pszDriverName ? pszDriverName : "", sizeof(pJ->szDriverName)-1);
    strncpy(pJ->szSpoolFile,  szFile, CCHMAXPATH-1);
    pQ->cJobs++;

    DosReleaseMutexSem(g_hmtxDB);
    return uJobId;
}

/*
 * Spl1bCloseJob - finish writing spool data; mark job ready to print.
 */
BOOL APIENTRY Spl1bCloseJob(USHORT uJobId, HFILE hFile)
{
    PSPLJOB pJ;

    DosClose(hFile);

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pJ = Spl1bFindJob(uJobId);
    if (pJ) {
        pJ->fsStatus = PRJ_QS_QUEUED;
        /* record file size */
        FILESTATUS3 fs;
        if (DosQueryPathInfo(pJ->szSpoolFile, FIL_STANDARD, &fs, sizeof(fs)) == NO_ERROR)
            pJ->ulSize = fs.cbFile;
    }
    DosReleaseMutexSem(g_hmtxDB);
    return (pJ != NULL);
}

/*
 * Spl1bAbortJob - discard a job being spooled.
 */
BOOL APIENTRY Spl1bAbortJob(USHORT uJobId, HFILE hFile)
{
    PSPLJOB   pJ;
    PSPLQUEUE pQ;
    CHAR      szFile[CCHMAXPATH];

    DosClose(hFile);

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pJ = Spl1bFindJob(uJobId);
    if (pJ) {
        strncpy(szFile, pJ->szSpoolFile, sizeof(szFile)-1);
        pQ = Spl1bFindQueue(pJ->szQueueName);
        if (pQ && pQ->cJobs > 0) pQ->cJobs--;
        /* compact array */
        USHORT idx = (USHORT)(pJ - g_jobs);
        if (idx < g_cJobs - 1)
            memmove(&g_jobs[idx], &g_jobs[idx+1],
                    (g_cJobs - idx - 1) * sizeof(SPLJOB));
        g_cJobs--;
        DosDelete(szFile);
    }
    DosReleaseMutexSem(g_hmtxDB);
    return TRUE;
}

/*
 * Spl1bDeleteJob - remove a queued (non-active) job.
 */
SPLERR APIENTRY Spl1bDeleteJob(USHORT uJobId)
{
    PSPLJOB   pJ;
    PSPLQUEUE pQ;
    CHAR      szFile[CCHMAXPATH];

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pJ = Spl1bFindJob(uJobId);
    if (!pJ) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_INV_JOB_ID;
    }
    strncpy(szFile, pJ->szSpoolFile, sizeof(szFile)-1);
    pQ = Spl1bFindQueue(pJ->szQueueName);
    if (pQ && pQ->cJobs > 0) pQ->cJobs--;
    USHORT idx = (USHORT)(pJ - g_jobs);
    if (idx < g_cJobs - 1)
        memmove(&g_jobs[idx], &g_jobs[idx+1],
                (g_cJobs - idx - 1) * sizeof(SPLJOB));
    g_cJobs--;
    DosDelete(szFile);
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

/*
 * Spl1bHoldJob / Spl1bReleaseJob
 */
SPLERR APIENTRY Spl1bHoldJob(USHORT uJobId)
{
    SPLERR rc = PMERR_SPL_INV_JOB_ID;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLJOB pJ = Spl1bFindJob(uJobId);
    if (pJ) { pJ->fsStatus = PRJ_QS_PAUSED; rc = SPL_OK; }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bReleaseJob(USHORT uJobId)
{
    SPLERR rc = PMERR_SPL_INV_JOB_ID;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLJOB pJ = Spl1bFindJob(uJobId);
    if (pJ) { pJ->fsStatus = PRJ_QS_QUEUED; rc = SPL_OK; }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

/*
 * Spl1bQueryJob - fill caller's buffer with PRJINFO3 for uJobId.
 */
SPLERR APIENTRY Spl1bQueryJob(USHORT uJobId, ULONG ulLevel,
                               PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{
    PSPLJOB pJ;
    SPLERR  rc = PMERR_SPL_INV_JOB_ID;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pJ = Spl1bFindJob(uJobId);
    if (pJ && ulLevel == 3) {
        /* Pack PRJINFO3 + variable strings into pBuf */
        ULONG cbNeed = sizeof(PRJINFO3) +
                       strlen(pJ->szUserName)   + 1 +
                       strlen(pJ->szDocName)    + 1 +
                       strlen(pJ->szQueueName)  + 1 +
                       strlen(pJ->szDataType)   + 1 +
                       strlen(pJ->szDriverName) + 1;
        if (pcbNeeded) *pcbNeeded = cbNeed;
        if (cbBuf >= cbNeed && pBuf) {
            PPRJINFO3 p = (PPRJINFO3)pBuf;
            PSZ       psz = (PSZ)((BYTE *)pBuf + sizeof(PRJINFO3));
            p->uJobId     = pJ->uJobId;
            p->uPriority  = pJ->uPriority;
            p->uPosition  = 1;
            p->fsStatus   = pJ->fsStatus;
            p->ulSubmitted = pJ->ulSubmitted;
            p->ulSize      = pJ->ulSize;
            p->pszUserName = psz; strcpy(psz, pJ->szUserName); psz += strlen(psz)+1;
            p->pszDocument = psz; strcpy(psz, pJ->szDocName);  psz += strlen(psz)+1;
            p->pszQueue    = psz; strcpy(psz, pJ->szQueueName);psz += strlen(psz)+1;
            p->pszDataType = psz; strcpy(psz, pJ->szDataType); psz += strlen(psz)+1;
            p->pszDriverName = psz; strcpy(psz, pJ->szDriverName);
            p->pszComment  = NULL;
            p->pszNotifyName = NULL;
            p->pszParms    = NULL;
            p->pszStatus   = NULL;
            p->pszQProcName = NULL;
            p->pszQProcParms = NULL;
            p->pDriverData = NULL;
            p->pszPrinterName = NULL;
            rc = SPL_OK;
        } else {
            rc = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

/*
 * Spl1bEnumJob - enumerate jobs for a queue into caller's buffer.
 */
SPLERR APIENTRY Spl1bEnumJob(PSZ pszQueueName, ULONG ulLevel,
                              PVOID pBuf, ULONG cbBuf,
                              PULONG pcReturned, PULONG pcTotal,
                              PULONG pcbNeeded)
{
    USHORT i, cFound = 0;
    ULONG  cbNeed = 0;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    for (i = 0; i < g_cJobs; i++) {
        if (!pszQueueName ||
            stricmp(g_jobs[i].szQueueName, pszQueueName) == 0)
            cFound++;
    }
    if (pcTotal)    *pcTotal    = cFound;
    if (pcReturned) *pcReturned = 0;
    cbNeed = cFound * sizeof(PRJINFO3); /* minimum estimate */
    if (pcbNeeded)  *pcbNeeded  = cbNeed;
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

/*
 * Spl1bCreateQueue
 */
SPLERR APIENTRY Spl1bCreateQueue(PSZ pszQueueName, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf)
{
    PSPLQUEUE pQ;

    if (!pszQueueName || strlen(pszQueueName) > QNLEN)
        return PMERR_SPL_INV_QUEUE_NAME;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    if (Spl1bFindQueue(pszQueueName)) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_QUEUE_ALREADY_EXISTS;
    }
    if (g_cQueues >= SPL_MAX_QUEUES) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_TOO_MANY_OPEN_FILES;
    }
    pQ = &g_queues[g_cQueues++];
    memset(pQ, 0, sizeof(*pQ));
    strncpy(pQ->szName, pszQueueName, QNLEN);
    pQ->uPriority = PRQ_DEF_PRIORITY;
    pQ->fsStatus  = PRQ_ACTIVE;

    if (ulLevel == 3 && pBuf) {
        PPRQINFO3 p = (PPRQINFO3)pBuf;
        if (p->pszPrinters)   strncpy(pQ->szPrinters,   p->pszPrinters,   PRINTERNAME_SIZE);
        if (p->pszDriverName) strncpy(pQ->szDriverName, p->pszDriverName, sizeof(pQ->szDriverName)-1);
        if (p->pszPrProc)     strncpy(pQ->szPrProc,     p->pszPrProc,     QNLEN);
        if (p->pszComment)    strncpy(pQ->szComment,    p->pszComment,    MAXCOMMENTSZ);
        pQ->uPriority  = p->uPriority;
        pQ->uStartTime = p->uStartTime;
        pQ->uUntilTime = p->uUntilTime;
        pQ->fsType     = p->fsType;
    }
    Spl1bSaveQueueToINI(pQ);
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

/*
 * Spl1bDeleteQueue
 */
SPLERR APIENTRY Spl1bDeleteQueue(PSZ pszQueueName)
{
    PSPLQUEUE pQ;
    USHORT    idx;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pQ = Spl1bFindQueue(pszQueueName);
    if (!pQ) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_INV_QUEUE_NAME;
    }
    if (pQ->cJobs > 0) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_QUEUE_NOT_EMPTY;
    }
    Spl1bDeleteQueueFromINI(pszQueueName);
    idx = (USHORT)(pQ - g_queues);
    if (idx < g_cQueues - 1)
        memmove(&g_queues[idx], &g_queues[idx+1],
                (g_cQueues - idx - 1) * sizeof(SPLQUEUE));
    g_cQueues--;
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

/*
 * Spl1bHoldQueue / Spl1bReleaseQueue / Spl1bPurgeQueue
 */
SPLERR APIENTRY Spl1bHoldQueue(PSZ pszQueueName)
{
    SPLERR rc = PMERR_SPL_INV_QUEUE_NAME;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLQUEUE pQ = Spl1bFindQueue(pszQueueName);
    if (pQ) { pQ->fsStatus |= PRQ3_PAUSED; rc = SPL_OK; }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bReleaseQueue(PSZ pszQueueName)
{
    SPLERR rc = PMERR_SPL_INV_QUEUE_NAME;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLQUEUE pQ = Spl1bFindQueue(pszQueueName);
    if (pQ) { pQ->fsStatus &= ~PRQ3_PAUSED; rc = SPL_OK; }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bPurgeQueue(PSZ pszQueueName)
{
    USHORT i;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLQUEUE pQ = Spl1bFindQueue(pszQueueName);
    if (!pQ) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_INV_QUEUE_NAME;
    }
    /* Delete all queued/paused jobs belonging to this queue */
    for (i = 0; i < g_cJobs; ) {
        if (stricmp(g_jobs[i].szQueueName, pszQueueName) == 0 &&
            g_jobs[i].fsStatus != PRJ_QS_PRINTING) {
            DosDelete(g_jobs[i].szSpoolFile);
            if (i < g_cJobs - 1)
                memmove(&g_jobs[i], &g_jobs[i+1],
                        (g_cJobs - i - 1) * sizeof(SPLJOB));
            g_cJobs--;
        } else {
            i++;
        }
    }
    pQ->cJobs = 0;
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

/*
 * Spl1bQueryQueue - fill buffer with PRQINFO3 for named queue.
 */
SPLERR APIENTRY Spl1bQueryQueue(PSZ pszQueueName, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{
    SPLERR rc = PMERR_SPL_INV_QUEUE_NAME;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLQUEUE pQ = Spl1bFindQueue(pszQueueName);
    if (pQ && ulLevel == 3) {
        ULONG cbNeed = sizeof(PRQINFO3) +
                       strlen(pQ->szName)       + 1 +
                       strlen(pQ->szPrinters)   + 1 +
                       strlen(pQ->szDriverName) + 1 +
                       strlen(pQ->szPrProc)     + 1 +
                       strlen(pQ->szComment)    + 1;
        if (pcbNeeded) *pcbNeeded = cbNeed;
        if (cbBuf >= cbNeed && pBuf) {
            PPRQINFO3 p = (PPRQINFO3)pBuf;
            PSZ psz = (PSZ)((BYTE *)pBuf + sizeof(PRQINFO3));
            p->pszName       = psz; strcpy(psz, pQ->szName);       psz += strlen(psz)+1;
            p->pszPrinters   = psz; strcpy(psz, pQ->szPrinters);   psz += strlen(psz)+1;
            p->pszDriverName = psz; strcpy(psz, pQ->szDriverName); psz += strlen(psz)+1;
            p->pszPrProc     = psz; strcpy(psz, pQ->szPrProc);     psz += strlen(psz)+1;
            p->pszComment    = psz; strcpy(psz, pQ->szComment);
            p->uPriority     = pQ->uPriority;
            p->uStartTime    = pQ->uStartTime;
            p->uUntilTime    = pQ->uUntilTime;
            p->fsType        = pQ->fsType;
            p->fsStatus      = pQ->fsStatus;
            p->cJobs         = pQ->cJobs;
            p->pszSepFile    = NULL;
            p->pszParms      = NULL;
            p->pDriverData   = NULL;
            rc = SPL_OK;
        } else rc = ERROR_INSUFFICIENT_BUFFER;
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

/*
 * Spl1bEnumQueue - enumerate all queues into caller's buffer.
 */
SPLERR APIENTRY Spl1bEnumQueue(ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                PULONG pcReturned, PULONG pcTotal,
                                PULONG pcbNeeded)
{
    if (pcTotal)    *pcTotal    = g_cQueues;
    if (pcReturned) *pcReturned = 0;
    if (pcbNeeded)  *pcbNeeded  = g_cQueues * sizeof(PRQINFO3);
    return SPL_OK;
}

/*
 * Spl1bGetNextReadyJob - called by SPOOL.EXE scheduler thread to find
 *                        a job ready to print on any non-paused queue.
 *                        Returns job ID or 0 if none available.
 */
USHORT APIENTRY Spl1bGetNextReadyJob(VOID)
{
    USHORT uJobId = 0;
    USHORT i;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    for (i = 0; i < g_cJobs; i++) {
        if (g_jobs[i].fsStatus == PRJ_QS_QUEUED) {
            PSPLQUEUE pQ = Spl1bFindQueue(g_jobs[i].szQueueName);
            if (pQ && !(pQ->fsStatus & PRQ3_PAUSED)) {
                g_jobs[i].fsStatus = PRJ_QS_PRINTING;
                uJobId = g_jobs[i].uJobId;
                break;
            }
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    return uJobId;
}

/*
 * Spl1bMarkJobComplete - called by scheduler after a job has printed.
 */
VOID APIENTRY Spl1bMarkJobComplete(USHORT uJobId)
{
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLJOB pJ = Spl1bFindJob(uJobId);
    if (pJ) {
        PSPLQUEUE pQ = Spl1bFindQueue(pJ->szQueueName);
        DosDelete(pJ->szSpoolFile);
        if (pQ && pQ->cJobs > 0) pQ->cJobs--;
        USHORT idx = (USHORT)(pJ - g_jobs);
        if (idx < g_cJobs - 1)
            memmove(&g_jobs[idx], &g_jobs[idx+1],
                    (g_cJobs - idx - 1) * sizeof(SPLJOB));
        g_cJobs--;
    }
    DosReleaseMutexSem(g_hmtxDB);
}

/*
 * Spl1bGetJobSpoolFile - retrieve the spool file path for a job.
 */
BOOL APIENTRY Spl1bGetJobSpoolFile(USHORT uJobId, PSZ pszBuf, ULONG cbBuf)
{
    BOOL fOk = FALSE;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLJOB pJ = Spl1bFindJob(uJobId);
    if (pJ) {
        strncpy(pszBuf, pJ->szSpoolFile, cbBuf - 1);
        pszBuf[cbBuf-1] = '\0';
        fOk = TRUE;
    }
    DosReleaseMutexSem(g_hmtxDB);
    return fOk;
}

/* ================================================================== */
/* Device (printer destination) management — INI-backed               */
/* ================================================================== */

static VOID Spl1bSaveDeviceToINI(PSPLDEVICE pD)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    if (hini == NULLHANDLE) return;
    PrfWriteProfileString(hini, SPL_INI_DEVICE,         pD->szName, pD->szLogAddr);
    PrfWriteProfileString(hini, SPL_INI_DEVICE_COMMENT, pD->szName, pD->szComment);
    PrfWriteProfileString(hini, SPL_INI_DEVICE_DRIVERS, pD->szName, pD->szDrivers);
    PrfCloseProfile(hini);
}

static VOID Spl1bDeleteDeviceFromINI(PSZ pszName)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    if (hini == NULLHANDLE) return;
    PrfWriteProfileString(hini, SPL_INI_DEVICE,         pszName, NULL);
    PrfWriteProfileString(hini, SPL_INI_DEVICE_COMMENT, pszName, NULL);
    PrfWriteProfileString(hini, SPL_INI_DEVICE_DRIVERS, pszName, NULL);
    PrfCloseProfile(hini);
}

static VOID Spl1bLoadDevicesFromINI(VOID)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    CHAR buf[4096];
    PSZ  psz;

    if (hini == NULLHANDLE) return;
    memset(buf, 0, sizeof(buf));
    if (!PrfQueryProfileString(hini, SPL_INI_DEVICE, NULL, "", buf, sizeof(buf))) {
        PrfCloseProfile(hini);
        return;
    }
    for (psz = buf; *psz && g_cDevices < SPL_MAX_DEVICES; psz += strlen(psz) + 1) {
        PSPLDEVICE pD = &g_devices[g_cDevices++];
        memset(pD, 0, sizeof(*pD));
        strncpy(pD->szName, psz, PDLEN);
        PrfQueryProfileString(hini, SPL_INI_DEVICE,         psz, "",
                              pD->szLogAddr, sizeof(pD->szLogAddr));
        PrfQueryProfileString(hini, SPL_INI_DEVICE_COMMENT, psz, "",
                              pD->szComment, sizeof(pD->szComment));
        PrfQueryProfileString(hini, SPL_INI_DEVICE_DRIVERS, psz, "",
                              pD->szDrivers, sizeof(pD->szDrivers));
        pD->fsStatus  = PRD_ACTIVE;
        pD->usTimeOut = 45;
    }
    PrfCloseProfile(hini);
}

SPLERR APIENTRY Spl1bCreateDevice(PSZ pszComputerName, ULONG ulLevel,
                                   PVOID pBuf, ULONG cbBuf)
{
    PSPLDEVICE pD;
    PRDINFO3  *pIn;

    if (!pBuf || ulLevel != 3) return PMERR_SPL_INV_PARM;
    pIn = (PRDINFO3 *)pBuf;
    if (!pIn->pszPrinterName || strlen(pIn->pszPrinterName) > PDLEN)
        return PMERR_SPL_INV_PARM;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    if (Spl1bFindDevice(pIn->pszPrinterName)) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_PRINTER_ALREADY_EXISTS;
    }
    if (g_cDevices >= SPL_MAX_DEVICES) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_TOO_MANY_OPEN_FILES;
    }
    pD = &g_devices[g_cDevices++];
    memset(pD, 0, sizeof(*pD));
    strncpy(pD->szName, pIn->pszPrinterName, PDLEN);
    if (pIn->pszLogAddr)  strncpy(pD->szLogAddr, pIn->pszLogAddr,  sizeof(pD->szLogAddr)-1);
    if (pIn->pszComment)  strncpy(pD->szComment, pIn->pszComment,  sizeof(pD->szComment)-1);
    if (pIn->pszDrivers)  strncpy(pD->szDrivers, pIn->pszDrivers,  sizeof(pD->szDrivers)-1);
    pD->fsStatus  = PRD_ACTIVE;
    pD->usTimeOut = pIn->usTimeOut ? pIn->usTimeOut : 45;
    Spl1bSaveDeviceToINI(pD);
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

SPLERR APIENTRY Spl1bDeleteDevice(PSZ pszComputerName, PSZ pszDeviceName)
{
    PSPLDEVICE pD;
    USHORT     idx;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pD = Spl1bFindDevice(pszDeviceName);
    if (!pD) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_INV_PRINTER_NAME;
    }
    Spl1bDeleteDeviceFromINI(pszDeviceName);
    idx = (USHORT)(pD - g_devices);
    if (idx < g_cDevices - 1)
        memmove(&g_devices[idx], &g_devices[idx+1],
                (g_cDevices - idx - 1) * sizeof(SPLDEVICE));
    g_cDevices--;
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

SPLERR APIENTRY Spl1bControlDevice(PSZ pszComputerName, PSZ pszPortName,
                                    ULONG ulControl)
{
    PSPLDEVICE pD;
    SPLERR     rc = PMERR_SPL_INV_PRINTER_NAME;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pD = Spl1bFindDevice(pszPortName);
    if (pD) {
        switch (ulControl) {
        case PRD_PAUSE:   pD->fsStatus  = PRD_PAUSED; rc = SPL_OK; break;
        case PRD_CONT:    pD->fsStatus  = PRD_ACTIVE; rc = SPL_OK; break;
        case PRD_RESTART: pD->fsStatus  = PRD_ACTIVE; rc = SPL_OK; break;
        case PRD_DELETE:
            Spl1bDeleteDeviceFromINI(pszPortName);
            USHORT idx = (USHORT)(pD - g_devices);
            if (idx < g_cDevices - 1)
                memmove(&g_devices[idx], &g_devices[idx+1],
                        (g_cDevices - idx - 1) * sizeof(SPLDEVICE));
            g_cDevices--;
            rc = SPL_OK;
            break;
        default: rc = PMERR_SPL_INV_PARM; break;
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bQueryDevice(PSZ pszComputerName, PSZ pszDeviceName,
                                  ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                  PULONG pcbNeeded)
{
    SPLERR rc = PMERR_SPL_INV_PRINTER_NAME;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLDEVICE pD = Spl1bFindDevice(pszDeviceName);
    if (pD) {
        if (ulLevel == 3) {
            ULONG cbNeed = sizeof(PRDINFO3) +
                           strlen(pD->szName)     + 1 +
                           strlen(pD->szLogAddr)  + 1 +
                           strlen(pD->szComment)  + 1 +
                           strlen(pD->szDrivers)  + 1;
            if (pcbNeeded) *pcbNeeded = cbNeed;
            if (cbBuf >= cbNeed && pBuf) {
                PPRDINFO3 p = (PPRDINFO3)pBuf;
                PSZ psz = (PSZ)((BYTE *)pBuf + sizeof(PRDINFO3));
                p->pszPrinterName = psz; strcpy(psz, pD->szName);    psz += strlen(psz)+1;
                p->pszLogAddr     = psz; strcpy(psz, pD->szLogAddr); psz += strlen(psz)+1;
                p->pszComment     = psz; strcpy(psz, pD->szComment); psz += strlen(psz)+1;
                p->pszDrivers     = psz; strcpy(psz, pD->szDrivers);
                p->uJobId         = pD->uJobId;
                p->fsStatus       = pD->fsStatus;
                p->usTimeOut      = pD->usTimeOut;
                p->pszUserName    = NULL;
                p->pszStatus      = NULL;
                p->time           = 0;
                rc = SPL_OK;
            } else rc = ERROR_INSUFFICIENT_BUFFER;
        } else if (ulLevel == 1) {
            ULONG cbNeed = sizeof(PRDINFO) +
                           strlen(pD->szName)    + 1 +
                           strlen(pD->szLogAddr) + 1;
            if (pcbNeeded) *pcbNeeded = cbNeed;
            if (cbBuf >= sizeof(PRDINFO) && pBuf) {
                PPRDINFO p = (PPRDINFO)pBuf;
                strncpy(p->szName,     pD->szName,    PDLEN);
                strncpy(p->szUserName, "",             UNLEN);
                p->uJobId   = pD->uJobId;
                p->fsStatus = pD->fsStatus;
                p->pszStatus = NULL;
                p->time      = 0;
                rc = SPL_OK;
            } else rc = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bSetDevice(PSZ pszComputerName, PSZ pszDeviceName,
                                ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                ULONG ulParmNum)
{
    PSPLDEVICE pD;
    SPLERR     rc = PMERR_SPL_INV_PRINTER_NAME;

    if (!pBuf || ulLevel != 3) return PMERR_SPL_INV_PARM;
    PPRDINFO3 pIn = (PPRDINFO3)pBuf;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pD = Spl1bFindDevice(pszDeviceName);
    if (pD) {
        if (!ulParmNum || ulParmNum == PRD_LOGADDR_PARMNUM)
            if (pIn->pszLogAddr)
                strncpy(pD->szLogAddr, pIn->pszLogAddr, sizeof(pD->szLogAddr)-1);
        if (!ulParmNum || ulParmNum == PRD_COMMENT_PARMNUM)
            if (pIn->pszComment)
                strncpy(pD->szComment, pIn->pszComment, sizeof(pD->szComment)-1);
        if (!ulParmNum || ulParmNum == PRD_DRIVERS_PARMNUM)
            if (pIn->pszDrivers)
                strncpy(pD->szDrivers, pIn->pszDrivers, sizeof(pD->szDrivers)-1);
        if (!ulParmNum || ulParmNum == PRD_TIMEOUT_PARMNUM)
            pD->usTimeOut = pIn->usTimeOut;
        Spl1bSaveDeviceToINI(pD);
        rc = SPL_OK;
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bEnumDevice(PSZ pszComputerName, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf,
                                 PULONG pcReturned, PULONG pcTotal,
                                 PULONG pcbNeeded, PVOID pReserved)
{
    USHORT i;
    ULONG  cbNeed = 0;
    BYTE  *pOut   = (BYTE *)pBuf;
    ULONG  cbLeft = cbBuf;
    ULONG  cRet   = 0;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    if (pcTotal) *pcTotal = g_cDevices;

    for (i = 0; i < g_cDevices; i++) {
        PSPLDEVICE pD = &g_devices[i];
        if (ulLevel == 3) {
            ULONG cbOne = sizeof(PRDINFO3) +
                          strlen(pD->szName)    + 1 +
                          strlen(pD->szLogAddr) + 1 +
                          strlen(pD->szComment) + 1 +
                          strlen(pD->szDrivers) + 1;
            cbNeed += cbOne;
            if (pOut && cbLeft >= cbOne) {
                PPRDINFO3 p = (PPRDINFO3)pOut;
                PSZ psz = (PSZ)(pOut + sizeof(PRDINFO3));
                p->pszPrinterName = psz; strcpy(psz, pD->szName);    psz += strlen(psz)+1;
                p->pszLogAddr     = psz; strcpy(psz, pD->szLogAddr); psz += strlen(psz)+1;
                p->pszComment     = psz; strcpy(psz, pD->szComment); psz += strlen(psz)+1;
                p->pszDrivers     = psz; strcpy(psz, pD->szDrivers);
                p->uJobId         = pD->uJobId;
                p->fsStatus       = pD->fsStatus;
                p->usTimeOut      = pD->usTimeOut;
                p->pszUserName    = NULL;
                p->pszStatus      = NULL;
                p->time           = 0;
                pOut  += cbOne; cbLeft -= cbOne; cRet++;
            }
        } else if (ulLevel == 1) {
            ULONG cbOne = sizeof(PRDINFO);
            cbNeed += cbOne;
            if (pOut && cbLeft >= cbOne) {
                PPRDINFO p = (PPRDINFO)pOut;
                strncpy(p->szName,     pD->szName, PDLEN);
                strncpy(p->szUserName, "",          UNLEN);
                p->uJobId   = pD->uJobId;
                p->fsStatus = pD->fsStatus;
                p->pszStatus = NULL;
                p->time      = 0;
                pOut += cbOne; cbLeft -= cbOne; cRet++;
            }
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    if (pcbNeeded)   *pcbNeeded  = cbNeed;
    if (pcReturned)  *pcReturned = cRet;
    return SPL_OK;
}

/* ================================================================== */
/* Port management — INI-backed                                        */
/* ================================================================== */

SPLERR APIENTRY Spl1bCreatePort(PSZ pszComputerName, PSZ pszPortName,
                                 PSZ pszPortDriver, ULONG ulVersion,
                                 PVOID pBuf, ULONG cbBuf)
{
    PSPLPORT pP;

    if (!pszPortName || strlen(pszPortName) > PDLEN)
        return PMERR_SPL_INV_PARM;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    if (Spl1bFindPort(pszPortName)) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_PORT_ALREADY_EXISTS;
    }
    if (g_cPorts >= SPL_MAX_PORTS) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_TOO_MANY_OPEN_FILES;
    }
    pP = &g_ports[g_cPorts++];
    memset(pP, 0, sizeof(*pP));
    strncpy(pP->szName, pszPortName, PDLEN);
    if (pszPortDriver)
        strncpy(pP->szDriver, pszPortDriver, sizeof(pP->szDriver)-1);
    pP->ulMode     = PRPORT_AUTODETECT;
    pP->ulPriority = 5;
    Spl1bSavePortToINI(pP);
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

SPLERR APIENTRY Spl1bDeletePort(PSZ pszComputerName, PSZ pszPortName)
{
    PSPLPORT pP;
    USHORT   idx;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pP = Spl1bFindPort(pszPortName);
    if (!pP) {
        DosReleaseMutexSem(g_hmtxDB);
        return PMERR_SPL_INV_PORT_NAME;
    }
    Spl1bDeletePortFromINI(pszPortName);
    idx = (USHORT)(pP - g_ports);
    if (idx < g_cPorts - 1)
        memmove(&g_ports[idx], &g_ports[idx+1],
                (g_cPorts - idx - 1) * sizeof(SPLPORT));
    g_cPorts--;
    DosReleaseMutexSem(g_hmtxDB);
    return SPL_OK;
}

SPLERR APIENTRY Spl1bQueryPort(PSZ pszComputerName, PSZ pszPortName,
                                ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                PULONG pcbNeeded)
{
    SPLERR rc = PMERR_SPL_INV_PORT_NAME;
    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    PSPLPORT pP = Spl1bFindPort(pszPortName);
    if (pP) {
        if (ulLevel == 2) {
            ULONG cbNeed = sizeof(PRPORTINFO2) +
                           strlen(pP->szName)     + 1 +
                           strlen(pP->szDriver)   + 1 +
                           strlen(pP->szProtocol) + 1;
            if (pcbNeeded) *pcbNeeded = cbNeed;
            if (cbBuf >= cbNeed && pBuf) {
                PPRPORTINFO2 p = (PPRPORTINFO2)pBuf;
                PSZ psz = (PSZ)((BYTE *)pBuf + sizeof(PRPORTINFO2));
                p->pszPortName          = psz; strcpy(psz, pP->szName);     psz += strlen(psz)+1;
                p->pszPortDriver        = psz; strcpy(psz, pP->szDriver);   psz += strlen(psz)+1;
                p->pszProtocolConverter = psz; strcpy(psz, pP->szProtocol);
                p->ulReserved           = 0;
                p->ulMode               = pP->ulMode;
                p->ulPriority           = pP->ulPriority;
                rc = SPL_OK;
            } else rc = ERROR_INSUFFICIENT_BUFFER;
        } else if (ulLevel == 0) {
            ULONG cbNeed = sizeof(PRPORTINFO);
            if (pcbNeeded) *pcbNeeded = cbNeed;
            if (cbBuf >= cbNeed && pBuf) {
                PPRPORTINFO p = (PPRPORTINFO)pBuf;
                strncpy(p->szPortName, pP->szName, PDLEN);
                rc = SPL_OK;
            } else rc = ERROR_INSUFFICIENT_BUFFER;
        } else if (ulLevel == 1) {
            ULONG cbNeed = sizeof(PRPORTINFO1) +
                           strlen(pP->szName)   + 1 +
                           strlen(pP->szDriver) + 1 + 1;
            if (pcbNeeded) *pcbNeeded = cbNeed;
            if (cbBuf >= cbNeed && pBuf) {
                PPRPORTINFO1 p = (PPRPORTINFO1)pBuf;
                PSZ psz = (PSZ)((BYTE *)pBuf + sizeof(PRPORTINFO1));
                p->pszPortName            = psz; strcpy(psz, pP->szName);   psz += strlen(psz)+1;
                p->pszPortDriverName      = psz; strcpy(psz, pP->szDriver); psz += strlen(psz)+1;
                p->pszPortDriverPathName  = psz; strcpy(psz, "");
                rc = SPL_OK;
            } else rc = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bSetPort(PSZ pszComputerName, PSZ pszPortName,
                              ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                              ULONG ulParmNum)
{
    PSPLPORT pP;
    SPLERR   rc = PMERR_SPL_INV_PORT_NAME;

    if (!pBuf || ulLevel != 2) return PMERR_SPL_INV_PARM;
    PPRPORTINFO2 pIn = (PPRPORTINFO2)pBuf;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    pP = Spl1bFindPort(pszPortName);
    if (pP) {
        if (!ulParmNum || ulParmNum == PRPO_PORT_DRIVER)
            if (pIn->pszPortDriver)
                strncpy(pP->szDriver, pIn->pszPortDriver, sizeof(pP->szDriver)-1);
        if (!ulParmNum || ulParmNum == PRPO_PROTOCOL_CNV)
            if (pIn->pszProtocolConverter)
                strncpy(pP->szProtocol, pIn->pszProtocolConverter, sizeof(pP->szProtocol)-1);
        if (!ulParmNum || ulParmNum == PRPO_MODE)
            pP->ulMode     = pIn->ulMode;
        if (!ulParmNum || ulParmNum == PRPO_PRIORITY)
            pP->ulPriority = pIn->ulPriority;
        Spl1bSavePortToINI(pP);
        rc = SPL_OK;
    }
    DosReleaseMutexSem(g_hmtxDB);
    return rc;
}

SPLERR APIENTRY Spl1bEnumPort(PSZ pszComputerName, ULONG ulLevel,
                               PVOID pBuf, ULONG cbBuf,
                               PULONG pcReturned, PULONG pcTotal,
                               PULONG pcbNeeded, PVOID pReserved)
{
    USHORT i;
    ULONG  cbNeed = 0;
    BYTE  *pOut   = (BYTE *)pBuf;
    ULONG  cbLeft = cbBuf;
    ULONG  cRet   = 0;

    DosRequestMutexSem(g_hmtxDB, SEM_INDEFINITE_WAIT);
    if (pcTotal) *pcTotal = g_cPorts;

    for (i = 0; i < g_cPorts; i++) {
        PSPLPORT pP = &g_ports[i];
        if (ulLevel == 2) {
            ULONG cbOne = sizeof(PRPORTINFO2) +
                          strlen(pP->szName)     + 1 +
                          strlen(pP->szDriver)   + 1 +
                          strlen(pP->szProtocol) + 1;
            cbNeed += cbOne;
            if (pOut && cbLeft >= cbOne) {
                PPRPORTINFO2 p = (PPRPORTINFO2)pOut;
                PSZ psz = (PSZ)(pOut + sizeof(PRPORTINFO2));
                p->pszPortName          = psz; strcpy(psz, pP->szName);     psz += strlen(psz)+1;
                p->pszPortDriver        = psz; strcpy(psz, pP->szDriver);   psz += strlen(psz)+1;
                p->pszProtocolConverter = psz; strcpy(psz, pP->szProtocol);
                p->ulMode               = pP->ulMode;
                p->ulPriority           = pP->ulPriority;
                p->ulReserved           = 0;
                pOut += cbOne; cbLeft -= cbOne; cRet++;
            }
        } else if (ulLevel == 1) {
            ULONG cbOne = sizeof(PRPORTINFO1) +
                          strlen(pP->szName)   + 1 +
                          strlen(pP->szDriver) + 1 + 1;
            cbNeed += cbOne;
            if (pOut && cbLeft >= cbOne) {
                PPRPORTINFO1 p = (PPRPORTINFO1)pOut;
                PSZ psz = (PSZ)(pOut + sizeof(PRPORTINFO1));
                p->pszPortName           = psz; strcpy(psz, pP->szName);   psz += strlen(psz)+1;
                p->pszPortDriverName     = psz; strcpy(psz, pP->szDriver); psz += strlen(psz)+1;
                p->pszPortDriverPathName = psz; strcpy(psz, "");
                pOut += cbOne; cbLeft -= cbOne; cRet++;
            }
        } else { /* level 0 */
            ULONG cbOne = sizeof(PRPORTINFO);
            cbNeed += cbOne;
            if (pOut && cbLeft >= cbOne) {
                PPRPORTINFO p = (PPRPORTINFO)pOut;
                strncpy(p->szPortName, pP->szName, PDLEN);
                pOut += cbOne; cbLeft -= cbOne; cRet++;
            }
        }
    }
    DosReleaseMutexSem(g_hmtxDB);
    if (pcbNeeded)  *pcbNeeded  = cbNeed;
    if (pcReturned) *pcReturned = cRet;
    return SPL_OK;
}
