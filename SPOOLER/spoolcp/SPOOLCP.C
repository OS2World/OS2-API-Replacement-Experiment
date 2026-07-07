/*
 * spoolcp.c - SPOOLCP.DLL replacement
 *
 * OS/2 Spooler Control Panel DLL.
 * Provides PM dialog boxes for managing print queues and jobs.
 * Loaded by the WPS Printer object and by the System Setup folder.
 *
 * Build: OpenWatcom  wcl386 -bt=os2 -bd -l=os2v2_dll spoolcp.c spoolcp.res
 */

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS
#include <os2.h>
#include <pmspl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/spooler_ipc.h"

/* ------------------------------------------------------------------ */
/* Dialog / window IDs (match .RC resource file)                       */
/* ------------------------------------------------------------------ */
#define DLG_SPOOLER_MAIN      100
#define DLG_QUEUE_PROPS       101
#define DLG_JOB_PROPS         102
#define DLG_PORT_PROPS        103

#define ID_LB_QUEUES          200
#define ID_LB_JOBS            201
#define ID_BTN_HOLD           210
#define ID_BTN_RELEASE        211
#define ID_BTN_DELETE         212
#define ID_BTN_PURGE          213
#define ID_BTN_PROPS          214
#define ID_BTN_CLOSE          215
#define ID_ST_STATUS          220
#define ID_ST_JOBCOUNT        221

#define ID_EF_QNAME           300
#define ID_EF_DRIVER          301
#define ID_EF_PORT            302
#define ID_EF_COMMENT         303
#define ID_CB_PRIORITY        304
#define ID_BTN_OK             305
#define ID_BTN_CANCEL         306

/* ------------------------------------------------------------------ */
/* Module handle (set in DLL init)                                     */
/* ------------------------------------------------------------------ */
static HMODULE g_hmod = NULLHANDLE;

/* ------------------------------------------------------------------ */
/* DLL init/term                                                       */
/* ------------------------------------------------------------------ */
ULONG _System _DLL_InitTerm(ULONG hModule, ULONG ulFlag)
{
    if (ulFlag == 0)
        g_hmod = (HMODULE)hModule;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static VOID SplCpRefreshJobList(HWND hwndLB, PSZ pszQueueName)
{
    BYTE    buf[8192];
    ULONG   cReturned = 0, cTotal = 0, cbNeeded = 0;
    SPLERR  rc;
    USHORT  i;

    WinSendMsg(hwndLB, LM_DELETEALL, 0, 0);

    rc = SplEnumJob(NULL, pszQueueName, 3,
                    buf, sizeof(buf),
                    &cReturned, &cTotal, &cbNeeded, NULL);
    if (rc != SPL_OK || cReturned == 0)
        return;

    PPRJINFO3 pJob = (PPRJINFO3)buf;
    for (i = 0; i < (USHORT)cReturned; i++) {
        CHAR szItem[128];
        sprintf(szItem, "#%u  %-32s  %lu bytes",
                pJob->uJobId,
                pJob->pszDocument ? pJob->pszDocument : "(unnamed)",
                pJob->ulSize);
        WinSendMsg(hwndLB, LM_INSERTITEM,
                   MPFROMSHORT(LIT_END),
                   MPFROMP(szItem));
        /* advance to next packed entry */
        pJob = (PPRJINFO3)((BYTE *)pJob + sizeof(PRJINFO3));
    }
}

static VOID SplCpRefreshQueueList(HWND hwndLB)
{
    BYTE   buf[8192];
    ULONG  cReturned = 0, cTotal = 0, cbNeeded = 0;
    SPLERR rc;
    USHORT i;

    WinSendMsg(hwndLB, LM_DELETEALL, 0, 0);

    rc = SplEnumQueue(NULL, 3, buf, sizeof(buf),
                      &cReturned, &cTotal, &cbNeeded, NULL);
    if (rc != SPL_OK || cReturned == 0)
        return;

    PPRQINFO3 pQ = (PPRQINFO3)buf;
    for (i = 0; i < (USHORT)cReturned; i++) {
        CHAR szItem[QNLEN + 32];
        sprintf(szItem, "%-13s  [%u jobs]",
                pQ->pszName ? pQ->pszName : "?",
                pQ->cJobs);
        WinSendMsg(hwndLB, LM_INSERTITEM,
                   MPFROMSHORT(LIT_END), MPFROMP(szItem));
        pQ = (PPRQINFO3)((BYTE *)pQ + sizeof(PRQINFO3));
    }
}

static USHORT SplCpGetSelectedJobId(HWND hwndLB)
{
    SHORT  sIdx;
    CHAR   szItem[128];

    sIdx = (SHORT)WinSendMsg(hwndLB, LM_QUERYSELECTION,
                             MPFROMSHORT(LIT_FIRST), 0);
    if (sIdx == LIT_NONE) return 0;
    WinSendMsg(hwndLB, LM_QUERYITEMTEXT,
               MPFROM2SHORT(sIdx, sizeof(szItem)), MPFROMP(szItem));
    /* Job ID is the first token after '#' */
    USHORT uId = 0;
    sscanf(szItem, "#%hu", &uId);
    return uId;
}

static VOID SplCpGetSelectedQueueName(HWND hwndLB, PSZ pszBuf, ULONG cbBuf)
{
    SHORT sIdx;
    CHAR  szItem[128];

    pszBuf[0] = '\0';
    sIdx = (SHORT)WinSendMsg(hwndLB, LM_QUERYSELECTION,
                             MPFROMSHORT(LIT_FIRST), 0);
    if (sIdx == LIT_NONE) return;
    WinSendMsg(hwndLB, LM_QUERYITEMTEXT,
               MPFROM2SHORT(sIdx, sizeof(szItem)), MPFROMP(szItem));
    /* Queue name is the first whitespace-delimited token */
    sscanf(szItem, "%12s", pszBuf);
}

/* ------------------------------------------------------------------ */
/* Main spooler dialog procedure                                       */
/* ------------------------------------------------------------------ */

typedef struct _DLGDATA {
    CHAR szCurrentQueue[QNLEN+1];
} DLGDATA;

static MRESULT EXPENTRY SplCpMainDlgProc(HWND hwnd, ULONG msg,
                                          MPARAM mp1, MPARAM mp2)
{
    DLGDATA *pData = (DLGDATA *)WinQueryWindowPtr(hwnd, QWL_USER);

    switch (msg) {
    case WM_INITDLG:
        {
            DLGDATA *pd = (DLGDATA *)malloc(sizeof(DLGDATA));
            if (pd) {
                memset(pd, 0, sizeof(*pd));
                WinSetWindowPtr(hwnd, QWL_USER, pd);
            }
            SplCpRefreshQueueList(WinWindowFromID(hwnd, ID_LB_QUEUES));
            WinSetWindowText(hwnd, "OS/2 Spooler");
        }
        return (MRESULT)FALSE;

    case WM_CONTROL:
        if (SHORT1FROMMP(mp1) == ID_LB_QUEUES &&
            SHORT2FROMMP(mp1) == LN_SELECT) {
            CHAR szQ[QNLEN+1];
            SplCpGetSelectedQueueName(WinWindowFromID(hwnd, ID_LB_QUEUES),
                                      szQ, sizeof(szQ));
            if (pData) strncpy(pData->szCurrentQueue, szQ, QNLEN);
            SplCpRefreshJobList(WinWindowFromID(hwnd, ID_LB_JOBS), szQ);
        }
        return (MRESULT)0;

    case WM_COMMAND:
        switch (SHORT1FROMMP(mp1)) {
        case ID_BTN_HOLD:
            {
                USHORT uJob = SplCpGetSelectedJobId(
                                  WinWindowFromID(hwnd, ID_LB_JOBS));
                if (uJob && pData)
                    SplHoldJob(NULL, pData->szCurrentQueue, uJob);
                else if (pData && pData->szCurrentQueue[0])
                    SplHoldQueue(NULL, pData->szCurrentQueue);
                SplCpRefreshJobList(WinWindowFromID(hwnd, ID_LB_JOBS),
                                    pData ? pData->szCurrentQueue : NULL);
            }
            break;
        case ID_BTN_RELEASE:
            {
                USHORT uJob = SplCpGetSelectedJobId(
                                  WinWindowFromID(hwnd, ID_LB_JOBS));
                if (uJob && pData)
                    SplReleaseJob(NULL, pData->szCurrentQueue, uJob);
                else if (pData && pData->szCurrentQueue[0])
                    SplReleaseQueue(NULL, pData->szCurrentQueue);
                SplCpRefreshJobList(WinWindowFromID(hwnd, ID_LB_JOBS),
                                    pData ? pData->szCurrentQueue : NULL);
            }
            break;
        case ID_BTN_DELETE:
            {
                USHORT uJob = SplCpGetSelectedJobId(
                                  WinWindowFromID(hwnd, ID_LB_JOBS));
                if (uJob && pData) {
                    if (WinMessageBox(HWND_DESKTOP, hwnd,
                                      "Delete selected print job?",
                                      "Spooler", 0,
                                      MB_YESNO | MB_ICONQUESTION) == MBID_YES)
                        SplDeleteJob(NULL, pData->szCurrentQueue, uJob);
                }
                SplCpRefreshJobList(WinWindowFromID(hwnd, ID_LB_JOBS),
                                    pData ? pData->szCurrentQueue : NULL);
            }
            break;
        case ID_BTN_PURGE:
            if (pData && pData->szCurrentQueue[0]) {
                if (WinMessageBox(HWND_DESKTOP, hwnd,
                                  "Delete all jobs in this queue?",
                                  "Spooler", 0,
                                  MB_YESNO | MB_ICONQUESTION) == MBID_YES) {
                    SplPurgeQueue(NULL, pData->szCurrentQueue);
                    SplCpRefreshJobList(WinWindowFromID(hwnd, ID_LB_JOBS),
                                        pData->szCurrentQueue);
                }
            }
            break;
        case ID_BTN_CLOSE:
            WinDismissDlg(hwnd, DID_OK);
            break;
        }
        return (MRESULT)0;

    case WM_DESTROY:
        if (pData) {
            free(pData);
            WinSetWindowPtr(hwnd, QWL_USER, NULL);
        }
        return (MRESULT)0;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------------ */
/* Queue Properties dialog                                             */
/* ------------------------------------------------------------------ */

typedef struct _QPROPDATA {
    CHAR szQueueName[QNLEN+1];
    BOOL fNew;
} QPROPDATA;

static MRESULT EXPENTRY SplCpQueuePropsDlgProc(HWND hwnd, ULONG msg,
                                                MPARAM mp1, MPARAM mp2)
{
    QPROPDATA *pData = (QPROPDATA *)WinQueryWindowPtr(hwnd, QWL_USER);

    switch (msg) {
    case WM_INITDLG:
        {
            pData = (QPROPDATA *)PVOIDFROMMP(mp2);
            WinSetWindowPtr(hwnd, QWL_USER, pData);
            if (pData && !pData->fNew) {
                BYTE    buf[4096];
                ULONG   cbNeeded = 0;
                if (SplQueryQueue(NULL, pData->szQueueName, 3,
                                  buf, sizeof(buf), &cbNeeded) == SPL_OK) {
                    PPRQINFO3 pQ = (PPRQINFO3)buf;
                    WinSetDlgItemText(hwnd, ID_EF_QNAME,
                                      pQ->pszName ? pQ->pszName : "");
                    WinSetDlgItemText(hwnd, ID_EF_DRIVER,
                                      pQ->pszDriverName ? pQ->pszDriverName : "");
                    WinSetDlgItemText(hwnd, ID_EF_PORT,
                                      pQ->pszPrinters ? pQ->pszPrinters : "");
                    WinSetDlgItemText(hwnd, ID_EF_COMMENT,
                                      pQ->pszComment ? pQ->pszComment : "");
                }
            }
        }
        return (MRESULT)FALSE;

    case WM_COMMAND:
        if (SHORT1FROMMP(mp1) == ID_BTN_OK) {
            CHAR szName[QNLEN+1], szDriver[DRIV_NAME_SIZE+DRIV_DEVICENAME_SIZE+2];
            CHAR szPort[PRINTERNAME_SIZE+1], szComment[MAXCOMMENTSZ+1];
            PRQINFO3 qi;

            WinQueryDlgItemText(hwnd, ID_EF_QNAME,   sizeof(szName),   szName);
            WinQueryDlgItemText(hwnd, ID_EF_DRIVER,  sizeof(szDriver), szDriver);
            WinQueryDlgItemText(hwnd, ID_EF_PORT,    sizeof(szPort),   szPort);
            WinQueryDlgItemText(hwnd, ID_EF_COMMENT, sizeof(szComment), szComment);

            memset(&qi, 0, sizeof(qi));
            qi.pszName       = szName;
            qi.pszDriverName = szDriver;
            qi.pszPrinters   = szPort;
            qi.pszComment    = szComment;
            qi.uPriority     = PRQ_DEF_PRIORITY;
            qi.fsStatus      = PRQ_ACTIVE;

            if (pData && pData->fNew)
                SplCreateQueue(NULL, 3, &qi, sizeof(qi));
            else
                SplSetQueue(NULL, szName, 3, &qi, sizeof(qi), 0);

            WinDismissDlg(hwnd, DID_OK);
        } else if (SHORT1FROMMP(mp1) == ID_BTN_CANCEL) {
            WinDismissDlg(hwnd, DID_CANCEL);
        }
        return (MRESULT)0;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------------ */
/* Exported API                                                        */
/* ------------------------------------------------------------------ */

/*
 * SplCpOpen - open the main spooler control panel dialog.
 * Called by the WPS or user code to show the spooler UI.
 */
ULONG APIENTRY SplCpOpen(HWND hwndOwner, ULONG ulReserved)
{
    /* WinDlgBox uses the owner window's anchor block / message queue. */
    return (ULONG)WinDlgBox(HWND_DESKTOP, hwndOwner,
                             SplCpMainDlgProc,
                             g_hmod, DLG_SPOOLER_MAIN, NULL);
}

/*
 * SplCpOpenQueueProps - open the queue properties dialog.
 * pszQueueName == NULL means create a new queue.
 */
ULONG APIENTRY SplCpOpenQueueProps(HWND hwndOwner, PSZ pszQueueName)
{
    QPROPDATA data;
    memset(&data, 0, sizeof(data));
    if (pszQueueName) {
        strncpy(data.szQueueName, pszQueueName, QNLEN);
        data.fNew = FALSE;
    } else {
        data.fNew = TRUE;
    }
    return (ULONG)WinDlgBox(HWND_DESKTOP, hwndOwner,
                             SplCpQueuePropsDlgProc,
                             g_hmod, DLG_QUEUE_PROPS, &data);
}

/*
 * SplCpInstall - called from the WPS printer installation wizard.
 *               Creates a queue with the given name and driver.
 */
SPLERR APIENTRY SplCpInstall(HWND hwndOwner, PSZ pszQueueName,
                               PSZ pszDriverName, PSZ pszPortName)
{
    PRQINFO3 qi;
    memset(&qi, 0, sizeof(qi));
    qi.pszName       = pszQueueName;
    qi.pszDriverName = pszDriverName;
    qi.pszPrinters   = pszPortName;
    qi.uPriority     = PRQ_DEF_PRIORITY;
    qi.fsStatus      = PRQ_ACTIVE;
    return SplCreateQueue(NULL, 3, &qi, sizeof(qi));
}

/*
 * SplCpGetDefaultQueue - returns the name of the application-default queue
 *                        as stored in OS2SYS.INI.
 */
BOOL APIENTRY SplCpGetDefaultQueue(PSZ pszBuf, ULONG cbBuf)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    BOOL fOk  = FALSE;

    if (hini == NULLHANDLE) return FALSE;
    fOk = PrfQueryProfileString(hini, SPL_INI_SPOOLER, "QUEUE",
                                NULL, pszBuf, cbBuf);
    PrfCloseProfile(hini);
    return fOk;
}

/*
 * SplCpSetDefaultQueue - saves the application-default queue name.
 */
BOOL APIENTRY SplCpSetDefaultQueue(PSZ pszQueueName)
{
    HAB  hab  = WinQueryAnchorBlock(HWND_DESKTOP);
    HINI hini = PrfOpenProfile(hab, "OS2SYS.INI");
    BOOL fOk  = FALSE;

    if (hini == NULLHANDLE) return FALSE;
    fOk = PrfWriteProfileString(hini, SPL_INI_SPOOLER, "QUEUE", pszQueueName);
    PrfCloseProfile(hini);
    return fOk;
}
