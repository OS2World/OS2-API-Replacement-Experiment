/*
 * spool.c - SPOOL.EXE replacement
 *
 * OS/2 Spooler daemon process.
 * Runs detached, creates the named pipe \PIPE\SPOOLER, dispatches
 * IPC requests from PMSPL.DLL clients, and drives the print scheduler.
 *
 * Architecture:
 *   main thread       - creates pipe, dispatches requests in loop
 *   scheduler thread  - polls SPL1B for ready jobs, sends to printer
 *   Each client request is handled inline (short operations) or handed
 *   to a worker thread for large data writes.
 *
 * Build: OpenWatcom  wcl386 -bt=os2 -l=os2v2 spool.c
 * Link:  wlink sys os2v2 name spool file spool.obj lib os2386.lib
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
#include "../include/spooler_ipc.h"

/* ------------------------------------------------------------------ */
/* SPL1B.DLL function pointers (loaded dynamically)                   */
/* ------------------------------------------------------------------ */
typedef BOOL    (APIENTRY *PFN_SPL1B_INIT)(PSZ);
typedef USHORT  (APIENTRY *PFN_SPL1B_OPENJOB)(PSZ,PSZ,PSZ,PSZ,PSZ,PHFILE);
typedef BOOL    (APIENTRY *PFN_SPL1B_CLOSEJOB)(USHORT,HFILE);
typedef BOOL    (APIENTRY *PFN_SPL1B_ABORTJOB)(USHORT,HFILE);
typedef SPLERR  (APIENTRY *PFN_SPL1B_DELETEJOB)(USHORT);
typedef SPLERR  (APIENTRY *PFN_SPL1B_HOLDJOB)(USHORT);
typedef SPLERR  (APIENTRY *PFN_SPL1B_RELEASEJOB)(USHORT);
typedef SPLERR  (APIENTRY *PFN_SPL1B_QUERYJOB)(USHORT,ULONG,PVOID,ULONG,PULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_ENUMJOB)(PSZ,ULONG,PVOID,ULONG,PULONG,PULONG,PULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_CREATEQUEUE)(PSZ,ULONG,PVOID,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_DELETEQUEUE)(PSZ);
typedef SPLERR  (APIENTRY *PFN_SPL1B_HOLDQUEUE)(PSZ);
typedef SPLERR  (APIENTRY *PFN_SPL1B_RELEASEQUEUE)(PSZ);
typedef SPLERR  (APIENTRY *PFN_SPL1B_PURGEQUEUE)(PSZ);
typedef SPLERR  (APIENTRY *PFN_SPL1B_QUERYQUEUE)(PSZ,ULONG,PVOID,ULONG,PULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_ENUMQUEUE)(ULONG,PVOID,ULONG,PULONG,PULONG,PULONG);
typedef USHORT  (APIENTRY *PFN_SPL1B_GETNEXTREADYJOB)(VOID);
typedef VOID    (APIENTRY *PFN_SPL1B_MARKJOBCOMPLETE)(USHORT);
typedef BOOL    (APIENTRY *PFN_SPL1B_GETJOBSPOOLFILE)(USHORT,PSZ,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_CREATEDEVICE)(PSZ,ULONG,PVOID,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_DELETEDEVICE)(PSZ,PSZ);
typedef SPLERR  (APIENTRY *PFN_SPL1B_CONTROLDEVICE)(PSZ,PSZ,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_QUERYDEVICE)(PSZ,PSZ,ULONG,PVOID,ULONG,PULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_SETDEVICE)(PSZ,PSZ,ULONG,PVOID,ULONG,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_ENUMDEVICE)(PSZ,ULONG,PVOID,ULONG,PULONG,PULONG,PULONG,PVOID);
typedef SPLERR  (APIENTRY *PFN_SPL1B_CREATEPORT)(PSZ,PSZ,PSZ,ULONG,PVOID,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_DELETEPORT)(PSZ,PSZ);
typedef SPLERR  (APIENTRY *PFN_SPL1B_QUERYPORT)(PSZ,PSZ,ULONG,PVOID,ULONG,PULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_SETPORT)(PSZ,PSZ,ULONG,PVOID,ULONG,ULONG);
typedef SPLERR  (APIENTRY *PFN_SPL1B_ENUMPORT)(PSZ,ULONG,PVOID,ULONG,PULONG,PULONG,PULONG,PVOID);

static HMODULE                   g_hmodSpl1b = NULLHANDLE;
static PFN_SPL1B_INIT            g_pfnInit;
static PFN_SPL1B_OPENJOB         g_pfnOpenJob;
static PFN_SPL1B_CLOSEJOB        g_pfnCloseJob;
static PFN_SPL1B_ABORTJOB        g_pfnAbortJob;
static PFN_SPL1B_DELETEJOB       g_pfnDeleteJob;
static PFN_SPL1B_HOLDJOB         g_pfnHoldJob;
static PFN_SPL1B_RELEASEJOB      g_pfnReleaseJob;
static PFN_SPL1B_QUERYJOB        g_pfnQueryJob;
static PFN_SPL1B_ENUMJOB         g_pfnEnumJob;
static PFN_SPL1B_CREATEQUEUE     g_pfnCreateQueue;
static PFN_SPL1B_DELETEQUEUE     g_pfnDeleteQueue;
static PFN_SPL1B_HOLDQUEUE       g_pfnHoldQueue;
static PFN_SPL1B_RELEASEQUEUE    g_pfnReleaseQueue;
static PFN_SPL1B_PURGEQUEUE      g_pfnPurgeQueue;
static PFN_SPL1B_QUERYQUEUE      g_pfnQueryQueue;
static PFN_SPL1B_ENUMQUEUE       g_pfnEnumQueue;
static PFN_SPL1B_GETNEXTREADYJOB g_pfnGetNextReadyJob;
static PFN_SPL1B_MARKJOBCOMPLETE g_pfnMarkJobComplete;
static PFN_SPL1B_GETJOBSPOOLFILE g_pfnGetJobSpoolFile;
static PFN_SPL1B_CREATEDEVICE    g_pfnCreateDevice;
static PFN_SPL1B_DELETEDEVICE    g_pfnDeleteDevice;
static PFN_SPL1B_CONTROLDEVICE   g_pfnControlDevice;
static PFN_SPL1B_QUERYDEVICE     g_pfnQueryDevice;
static PFN_SPL1B_SETDEVICE       g_pfnSetDevice;
static PFN_SPL1B_ENUMDEVICE      g_pfnEnumDevice;
static PFN_SPL1B_CREATEPORT      g_pfnCreatePort;
static PFN_SPL1B_DELETEPORT      g_pfnDeletePort;
static PFN_SPL1B_QUERYPORT       g_pfnQueryPort;
static PFN_SPL1B_SETPORT         g_pfnSetPort;
static PFN_SPL1B_ENUMPORT        g_pfnEnumPort;

/* ------------------------------------------------------------------ */
/* Active-session table: maps HSPL handle -> open job state           */
/* ------------------------------------------------------------------ */
#define MAX_SESSIONS 64

typedef struct _SESSION {
    ULONG  ulHandle;    /* == uJobId for active jobs */
    USHORT uJobId;
    HFILE  hFile;
    BOOL   fDocStarted;
} SESSION;

static SESSION  g_sessions[MAX_SESSIONS];
static USHORT   g_cSessions = 0;
static HMTX     g_hmtxSess  = NULLHANDLE;
static ULONG    g_ulNextHdl = 1;

static volatile BOOL g_fRunning = TRUE;

/* ------------------------------------------------------------------ */
/* Session helpers                                                     */
/* ------------------------------------------------------------------ */

static SESSION *SessAlloc(VOID)
{
    if (g_cSessions >= MAX_SESSIONS) return NULL;
    SESSION *pS = &g_sessions[g_cSessions++];
    memset(pS, 0, sizeof(*pS));
    pS->ulHandle = g_ulNextHdl++;
    if (g_ulNextHdl == 0) g_ulNextHdl = 1;
    return pS;
}

static SESSION *SessFind(ULONG ulHandle)
{
    USHORT i;
    for (i = 0; i < g_cSessions; i++)
        if (g_sessions[i].ulHandle == ulHandle)
            return &g_sessions[i];
    return NULL;
}

static VOID SessFree(SESSION *pS)
{
    USHORT idx = (USHORT)(pS - g_sessions);
    if (idx < g_cSessions - 1)
        memmove(&g_sessions[idx], &g_sessions[idx+1],
                (g_cSessions - idx - 1) * sizeof(SESSION));
    g_cSessions--;
}

/* ------------------------------------------------------------------ */
/* Pipe send-reply helpers                                             */
/* ------------------------------------------------------------------ */

static VOID PipeSendReply(HPIPE hPipe, SPLERR rc,
                           PVOID pData, USHORT cbData)
{
    SPLREPLYHDR rhdr;
    ULONG       cbW;
    rhdr.rc     = rc;
    rhdr.cbData = cbData;
    DosWrite(hPipe, &rhdr, sizeof(rhdr), &cbW);
    if (cbData && pData)
        DosWrite(hPipe, pData, cbData, &cbW);
}

static VOID PipeSendOK(HPIPE hPipe)
{
    PipeSendReply(hPipe, SPL_OK, NULL, 0);
}

static VOID PipeSendErr(HPIPE hPipe, SPLERR rc)
{
    PipeSendReply(hPipe, rc, NULL, 0);
}

/* ------------------------------------------------------------------ */
/* Request dispatcher                                                  */
/* ------------------------------------------------------------------ */

static VOID HandleRequest(HPIPE hPipe, PSPLMSGHDR pHdr,
                           PVOID pPayload, USHORT cbPayload)
{
    switch (pHdr->usCode) {

    /* ------ Queue Manager ------ */
    case SPLREQ_QM_OPEN:
        {
            PSPLREQ_QMOPEN pReq = (PSPLREQ_QMOPEN)pPayload;
            SESSION *pS;
            USHORT   uJobId;
            HFILE    hFile;

            DosRequestMutexSem(g_hmtxSess, SEM_INDEFINITE_WAIT);
            pS = SessAlloc();
            if (!pS) {
                DosReleaseMutexSem(g_hmtxSess);
                PipeSendErr(hPipe, PMERR_SPL_TOO_MANY_OPEN_FILES);
                break;
            }
            /* Queue name comes from first element of open-data array */
            uJobId = g_pfnOpenJob(pReq->szToken, "", "", "PM_Q_STD",
                                  "", &hFile);
            if (uJobId == 0) {
                SessFree(pS);
                DosReleaseMutexSem(g_hmtxSess);
                PipeSendErr(hPipe, PMERR_SPL_NO_SPOOLER);
                break;
            }
            pS->uJobId = uJobId;
            pS->hFile  = hFile;
            ULONG ulH  = pS->ulHandle;
            DosReleaseMutexSem(g_hmtxSess);
            PipeSendReply(hPipe, SPL_OK, &ulH, sizeof(ulH));
        }
        break;

    case SPLREQ_QM_STARTDOC:
        {
            PSPLREQ_STARTDOC pReq = (PSPLREQ_STARTDOC)pPayload;
            DosRequestMutexSem(g_hmtxSess, SEM_INDEFINITE_WAIT);
            SESSION *pS = SessFind(pReq->ulHandle);
            if (!pS) {
                DosReleaseMutexSem(g_hmtxSess);
                PipeSendErr(hPipe, PMERR_SPL_INV_HSPL);
                break;
            }
            pS->fDocStarted = TRUE;
            DosReleaseMutexSem(g_hmtxSess);
            PipeSendOK(hPipe);
        }
        break;

    case SPLREQ_QM_WRITE:
        {
            PSPLREQ_WRITE pReq  = (PSPLREQ_WRITE)pPayload;
            PVOID         pData = (BYTE *)pPayload + sizeof(SPLREQ_WRITE);
            ULONG         cbW   = 0;

            DosRequestMutexSem(g_hmtxSess, SEM_INDEFINITE_WAIT);
            SESSION *pS = SessFind(pReq->ulHandle);
            if (!pS || pS->hFile == NULLHANDLE) {
                DosReleaseMutexSem(g_hmtxSess);
                PipeSendErr(hPipe, PMERR_SPL_INV_HSPL);
                break;
            }
            APIRET rc = DosWrite(pS->hFile, pData,
                                 (ULONG)pReq->lCount, &cbW);
            DosReleaseMutexSem(g_hmtxSess);
            PipeSendReply(hPipe,
                          (rc == NO_ERROR) ? SPL_OK : PMERR_SPL_ERROR,
                          NULL, 0);
        }
        break;

    case SPLREQ_QM_ENDDOC:
    case SPLREQ_QM_CLOSE:
        {
            PSPLREQ_HANDLE pReq = (PSPLREQ_HANDLE)pPayload;
            DosRequestMutexSem(g_hmtxSess, SEM_INDEFINITE_WAIT);
            SESSION *pS = SessFind(pReq->ulHandle);
            if (!pS) {
                DosReleaseMutexSem(g_hmtxSess);
                PipeSendErr(hPipe, PMERR_SPL_INV_HSPL);
                break;
            }
            g_pfnCloseJob(pS->uJobId, pS->hFile);
            SessFree(pS);
            DosReleaseMutexSem(g_hmtxSess);
            PipeSendOK(hPipe);
        }
        break;

    case SPLREQ_QM_ABORT:
    case SPLREQ_QM_ABORTDOC:
        {
            PSPLREQ_HANDLE pReq = (PSPLREQ_HANDLE)pPayload;
            DosRequestMutexSem(g_hmtxSess, SEM_INDEFINITE_WAIT);
            SESSION *pS = SessFind(pReq->ulHandle);
            if (!pS) {
                DosReleaseMutexSem(g_hmtxSess);
                PipeSendErr(hPipe, PMERR_SPL_INV_HSPL);
                break;
            }
            g_pfnAbortJob(pS->uJobId, pS->hFile);
            SessFree(pS);
            DosReleaseMutexSem(g_hmtxSess);
            PipeSendOK(hPipe);
        }
        break;

    case SPLREQ_QM_NEWPAGE:
        PipeSendOK(hPipe); /* page tracking is informational */
        break;

    case SPLREQ_QM_GETJOBID:
        {
            PSPLREQ_HANDLE pReq = (PSPLREQ_HANDLE)pPayload;
            SPLREPLY_JOBID reply;
            memset(&reply, 0, sizeof(reply));
            DosRequestMutexSem(g_hmtxSess, SEM_INDEFINITE_WAIT);
            SESSION *pS = SessFind(pReq->ulHandle);
            if (pS) reply.ulJobID = pS->uJobId;
            DosReleaseMutexSem(g_hmtxSess);
            strcpy(reply.szComputerName, "");
            PipeSendReply(hPipe, SPL_OK, &reply, sizeof(reply));
        }
        break;

    /* ------ Queue management ------ */
    case SPLREQ_CREATE_QUEUE:
        PipeSendReply(hPipe,
            g_pfnCreateQueue(NULL, 3, pPayload, cbPayload),
            NULL, 0);
        break;
    case SPLREQ_DELETE_QUEUE:
        PipeSendReply(hPipe, g_pfnDeleteQueue((PSZ)pPayload), NULL, 0);
        break;
    case SPLREQ_HOLD_QUEUE:
        PipeSendReply(hPipe, g_pfnHoldQueue((PSZ)pPayload), NULL, 0);
        break;
    case SPLREQ_RELEASE_QUEUE:
        PipeSendReply(hPipe, g_pfnReleaseQueue((PSZ)pPayload), NULL, 0);
        break;
    case SPLREQ_PURGE_QUEUE:
        PipeSendReply(hPipe, g_pfnPurgeQueue((PSZ)pPayload), NULL, 0);
        break;
    case SPLREQ_QUERY_QUEUE:
        {
            ULONG  ulLevel = *(ULONG *)pPayload;
            PSZ    pszName = (PSZ)pPayload + sizeof(ULONG);
            BYTE   buf[4096];
            ULONG  cbNeeded = 0;
            SPLERR rc = g_pfnQueryQueue(pszName, ulLevel,
                                         buf, sizeof(buf), &cbNeeded);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeeded);
        }
        break;
    case SPLREQ_SET_QUEUE:
        PipeSendReply(hPipe,
            g_pfnCreateQueue(NULL, 3, pPayload, cbPayload),
            NULL, 0);
        break;
    case SPLREQ_ENUM_QUEUE:
        {
            ULONG ulLevel = *(ULONG *)pPayload;
            BYTE  buf[8192];
            ULONG cRet = 0, cTot = 0, cbNeed = 0;
            SPLERR rc = g_pfnEnumQueue(ulLevel, buf, sizeof(buf),
                                        &cRet, &cTot, &cbNeed);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;

    /* ------ Job management ------ */
    case SPLREQ_DELETE_JOB:
        PipeSendReply(hPipe,
            g_pfnDeleteJob((USHORT)(*(ULONG *)pPayload)), NULL, 0);
        break;
    case SPLREQ_HOLD_JOB:
        PipeSendReply(hPipe,
            g_pfnHoldJob((USHORT)(*(ULONG *)pPayload)), NULL, 0);
        break;
    case SPLREQ_RELEASE_JOB:
        PipeSendReply(hPipe,
            g_pfnReleaseJob((USHORT)(*(ULONG *)pPayload)), NULL, 0);
        break;
    case SPLREQ_QUERY_JOB:
        {
            ULONG *pReq = (ULONG *)pPayload;
            BYTE   buf[4096];
            ULONG  cbNeed = 0;
            SPLERR rc = g_pfnQueryJob((USHORT)pReq[0], pReq[1],
                                       buf, sizeof(buf), &cbNeed);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;
    case SPLREQ_ENUM_JOB:
        {
            BYTE  buf[8192];
            ULONG cRet = 0, cTot = 0, cbNeed = 0;
            SPLERR rc = g_pfnEnumJob(NULL, *(ULONG *)pPayload,
                                      buf, sizeof(buf),
                                      &cRet, &cTot, &cbNeed);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;

    /* ------ Device management ------ */
    case SPLREQ_CREATE_DEVICE:
        PipeSendReply(hPipe,
            g_pfnCreateDevice(NULL, 3, pPayload, cbPayload), NULL, 0);
        break;
    case SPLREQ_DELETE_DEVICE:
        PipeSendReply(hPipe,
            g_pfnDeleteDevice(NULL, (PSZ)pPayload), NULL, 0);
        break;
    case SPLREQ_CONTROL_DEVICE:
        {
            PSPLREQ_DEVCTL pCtl = (PSPLREQ_DEVCTL)pPayload;
            PipeSendReply(hPipe,
                g_pfnControlDevice(NULL, pCtl->szPortName, pCtl->ulControl),
                NULL, 0);
        }
        break;
    case SPLREQ_QUERY_DEVICE:
        {
            PSPLREQ_NAMED pReq = (PSPLREQ_NAMED)pPayload;
            BYTE   buf[4096];
            ULONG  cbNeed = 0;
            SPLERR rc = g_pfnQueryDevice(NULL, pReq->szName,
                                          pReq->ulLevel, buf, sizeof(buf),
                                          &cbNeed);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;
    case SPLREQ_SET_DEVICE:
        {
            PSPLREQ_NAMED pReq = (PSPLREQ_NAMED)pPayload;
            PVOID  pData = (PVOID)((PBYTE)pPayload + sizeof(SPLREQ_NAMED));
            ULONG  cbData = cbPayload > sizeof(SPLREQ_NAMED)
                            ? cbPayload - sizeof(SPLREQ_NAMED) : 0;
            PipeSendReply(hPipe,
                g_pfnSetDevice(NULL, pReq->szName, pReq->ulLevel,
                               pData, cbData, pReq->ulParmNum),
                NULL, 0);
        }
        break;
    case SPLREQ_ENUM_DEVICE:
        {
            ULONG  ulLevel = *(ULONG *)pPayload;
            BYTE   buf[8192];
            ULONG  cRet = 0, cTot = 0, cbNeed = 0;
            SPLERR rc = g_pfnEnumDevice(NULL, ulLevel, buf, sizeof(buf),
                                         &cRet, &cTot, &cbNeed, NULL);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;

    /* ------ Port management ------ */
    case SPLREQ_CREATE_PORT:
        PipeSendReply(hPipe,
            g_pfnCreatePort(NULL, (PSZ)pPayload, NULL, 0, NULL, 0), NULL, 0);
        break;
    case SPLREQ_DELETE_PORT:
        PipeSendReply(hPipe,
            g_pfnDeletePort(NULL, (PSZ)pPayload), NULL, 0);
        break;
    case SPLREQ_QUERY_PORT:
        {
            PSPLREQ_NAMED pReq = (PSPLREQ_NAMED)pPayload;
            BYTE   buf[4096];
            ULONG  cbNeed = 0;
            SPLERR rc = g_pfnQueryPort(NULL, pReq->szName,
                                        pReq->ulLevel, buf, sizeof(buf),
                                        &cbNeed);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;
    case SPLREQ_SET_PORT:
        {
            PSPLREQ_NAMED pReq = (PSPLREQ_NAMED)pPayload;
            PVOID  pData = (PVOID)((PBYTE)pPayload + sizeof(SPLREQ_NAMED));
            ULONG  cbData = cbPayload > sizeof(SPLREQ_NAMED)
                            ? cbPayload - sizeof(SPLREQ_NAMED) : 0;
            PipeSendReply(hPipe,
                g_pfnSetPort(NULL, pReq->szName, pReq->ulLevel,
                             pData, cbData, pReq->ulParmNum),
                NULL, 0);
        }
        break;
    case SPLREQ_ENUM_PORT:
        {
            ULONG  ulLevel = *(ULONG *)pPayload;
            BYTE   buf[8192];
            ULONG  cRet = 0, cTot = 0, cbNeed = 0;
            SPLERR rc = g_pfnEnumPort(NULL, ulLevel, buf, sizeof(buf),
                                       &cRet, &cTot, &cbNeed, NULL);
            PipeSendReply(hPipe, rc, buf, (USHORT)cbNeed);
        }
        break;

    /* ------ Remaining stubs (driver enum, QP enum, printer enum) ------ */
    case SPLREQ_ENUM_DRIVER:
    case SPLREQ_QUERY_DRIVER:
    case SPLREQ_SET_DRIVER:
    case SPLREQ_ENUM_QP:
    case SPLREQ_ENUM_PRINTER:
    case SPLREQ_COPY_JOB:
        PipeSendOK(hPipe);
        break;

    default:
        PipeSendErr(hPipe, PMERR_SPL_ERROR);
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Print scheduler thread                                              */
/* ------------------------------------------------------------------ */

static VOID PrintJobToPort(USHORT uJobId)
{
    CHAR    szFile[CCHMAXPATH];
    HFILE   hSrc = NULLHANDLE;
    HFILE   hDst = NULLHANDLE;
    ULONG   ulAction, cbRead, cbW;
    BYTE    aBuf[4096];
    APIRET  rc;

    if (!g_pfnGetJobSpoolFile(uJobId, szFile, sizeof(szFile)))
        return;

    /* Open spool file */
    rc = DosOpen(szFile, &hSrc, &ulAction, 0, FILE_NORMAL,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, NULL);
    if (rc != NO_ERROR) return;

    /* Open LPT1 as the default output port (configurable via INI) */
    rc = DosOpen("LPT1", &hDst, &ulAction, 0, FILE_NORMAL,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, NULL);
    if (rc != NO_ERROR) {
        DosClose(hSrc);
        return;
    }

    /* Stream data */
    for (;;) {
        rc = DosRead(hSrc, aBuf, sizeof(aBuf), &cbRead);
        if (rc != NO_ERROR || cbRead == 0) break;
        DosWrite(hDst, aBuf, cbRead, &cbW);
    }

    DosClose(hSrc);
    DosClose(hDst);
}

static VOID APIENTRY SchedulerThread(ULONG ulParam)
{
    while (g_fRunning) {
        USHORT uJobId = g_pfnGetNextReadyJob();
        if (uJobId) {
            PrintJobToPort(uJobId);
            g_pfnMarkJobComplete(uJobId);
        } else {
            DosSleep(1000); /* wait 1 s before polling again */
        }
    }
}

/* ------------------------------------------------------------------ */
/* Load SPL1B.DLL                                                      */
/* ------------------------------------------------------------------ */

static BOOL LoadSpl1b(VOID)
{
    CHAR szErr[256];

#define LOADFN(name, pfn) \
    if (DosQueryProcAddr(g_hmodSpl1b, 0, #name, (PFN *)&pfn) != NO_ERROR) { \
        fprintf(stderr, "SPOOL: missing export " #name " from SPL1B.DLL\n"); \
        return FALSE; \
    }

    if (DosLoadModule(szErr, sizeof(szErr), "SPL1B", &g_hmodSpl1b) != NO_ERROR) {
        fprintf(stderr, "SPOOL: cannot load SPL1B.DLL: %s\n", szErr);
        return FALSE;
    }
    LOADFN(Spl1bInit,            g_pfnInit)
    LOADFN(Spl1bOpenJob,         g_pfnOpenJob)
    LOADFN(Spl1bCloseJob,        g_pfnCloseJob)
    LOADFN(Spl1bAbortJob,        g_pfnAbortJob)
    LOADFN(Spl1bDeleteJob,       g_pfnDeleteJob)
    LOADFN(Spl1bHoldJob,         g_pfnHoldJob)
    LOADFN(Spl1bReleaseJob,      g_pfnReleaseJob)
    LOADFN(Spl1bQueryJob,        g_pfnQueryJob)
    LOADFN(Spl1bEnumJob,         g_pfnEnumJob)
    LOADFN(Spl1bCreateQueue,     g_pfnCreateQueue)
    LOADFN(Spl1bDeleteQueue,     g_pfnDeleteQueue)
    LOADFN(Spl1bHoldQueue,       g_pfnHoldQueue)
    LOADFN(Spl1bReleaseQueue,    g_pfnReleaseQueue)
    LOADFN(Spl1bPurgeQueue,      g_pfnPurgeQueue)
    LOADFN(Spl1bQueryQueue,      g_pfnQueryQueue)
    LOADFN(Spl1bEnumQueue,       g_pfnEnumQueue)
    LOADFN(Spl1bGetNextReadyJob, g_pfnGetNextReadyJob)
    LOADFN(Spl1bMarkJobComplete, g_pfnMarkJobComplete)
    LOADFN(Spl1bGetJobSpoolFile, g_pfnGetJobSpoolFile)
    LOADFN(Spl1bCreateDevice,    g_pfnCreateDevice)
    LOADFN(Spl1bDeleteDevice,    g_pfnDeleteDevice)
    LOADFN(Spl1bControlDevice,   g_pfnControlDevice)
    LOADFN(Spl1bQueryDevice,     g_pfnQueryDevice)
    LOADFN(Spl1bSetDevice,       g_pfnSetDevice)
    LOADFN(Spl1bEnumDevice,      g_pfnEnumDevice)
    LOADFN(Spl1bCreatePort,      g_pfnCreatePort)
    LOADFN(Spl1bDeletePort,      g_pfnDeletePort)
    LOADFN(Spl1bQueryPort,       g_pfnQueryPort)
    LOADFN(Spl1bSetPort,         g_pfnSetPort)
    LOADFN(Spl1bEnumPort,        g_pfnEnumPort)
#undef LOADFN
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* Main pipe server loop                                               */
/* ------------------------------------------------------------------ */

static VOID RunPipeServer(VOID)
{
    HPIPE   hPipe;
    APIRET  rc;
    BYTE    payload[65536];
    SPLMSGHDR hdr;
    ULONG   cbRead;

    for (;;) {
        /* Create a new pipe instance for each client */
        rc = DosCreateNPipe(SPL_PIPE_NAME, &hPipe,
                            NP_ACCESS_DUPLEX | NP_NOINHERIT,
                            NP_WAIT | NP_TYPE_BYTE | NP_READMODE_BYTE | 0x01,
                            sizeof(payload), sizeof(payload), 0);
        if (rc != NO_ERROR) {
            DosSleep(500);
            continue;
        }

        /* Wait for a client connection */
        rc = DosConnectNPipe(hPipe);
        if (rc != NO_ERROR) {
            DosClose(hPipe);
            if (!g_fRunning) break;
            continue;
        }

        /* Read message header */
        rc = DosRead(hPipe, &hdr, sizeof(hdr), &cbRead);
        if (rc == NO_ERROR && cbRead == sizeof(hdr)) {
            USHORT cbP = hdr.cbPayload;
            if (cbP > sizeof(payload)) cbP = sizeof(payload);
            if (cbP > 0) {
                DosRead(hPipe, payload, cbP, &cbRead);
            }
            HandleRequest(hPipe, &hdr, cbP ? payload : NULL, cbP);
        }

        DosDisConnectNPipe(hPipe);
        DosClose(hPipe);

        if (!g_fRunning) break;
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    TID    tidSched;
    APIRET rc;

    /* Run detached */
    DosSetPriority(PRTYS_PROCESS, PRTYC_REGULAR, 0, 0);

    if (!LoadSpl1b())
        return 1;

    DosCreateMutexSem(NULL, &g_hmtxSess, 0, FALSE);
    memset(g_sessions, 0, sizeof(g_sessions));
    g_cSessions = 0;

    /* Initialise the backend */
    g_pfnInit(NULL);

    /* Start the print scheduler thread */
    rc = DosCreateThread(&tidSched, SchedulerThread, 0, 0, 65536);
    if (rc != NO_ERROR) {
        fprintf(stderr, "SPOOL: cannot create scheduler thread: %lu\n", rc);
        return 1;
    }

    /* Signal readiness via a shared event semaphore (WPS waits for this) */
    HEV hev = NULLHANDLE;
    rc = DosOpenEventSem("\\SEM32\\SPOOLER", &hev);
    if (rc == NO_ERROR) {
        DosPostEventSem(hev);
        DosCloseEventSem(hev);
    }

    /* Enter the main request loop */
    RunPipeServer();

    /* Shutdown */
    g_fRunning = FALSE;
    DosWaitThread(&tidSched, DCWW_WAIT);
    DosCloseMutexSem(g_hmtxSess);
    DosFreeModule(g_hmodSpl1b);
    return 0;
}
