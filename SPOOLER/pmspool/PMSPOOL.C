/*
 * pmspool.c - PMSPOOL.EXE replacement
 *
 * PM Spooler Settings Executable.
 * Launched from the "Printers" object in the System Setup folder, or by
 * the WPS printer open action.  Accepts optional WPS setup arguments:
 *
 *   /SETUP            - open the Add Printer wizard
 *   /QUEUE:<name>     - open properties for the named queue
 *   /INSTALL          - silent install mode (no UI)
 *
 * Without arguments the main Printers window opens, showing all queues
 * and jobs.  Uses PMSPL.DLL directly for all spooler operations.
 *
 * Imports: DOSCALLS, PMWIN, PMSPL  (matches the original binary).
 *
 * Build: OpenWatcom
 *   wcc386 -bt=os2 -mf -3r -d2 -w4 -ze pmspool.c
 *   wrc -r pmspool.rc
 *   wlink system os2v2_pm name pmspool.exe file pmspool.obj ...
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS
#include <os2.h>
#include <pmspl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Resource IDs (must match pmspool.rc)                               */
/* ------------------------------------------------------------------ */
#define IDR_MAINMENU          1
#define IDI_PRINTER           2

/* Menu items */
#define IDM_PRINTER           100
#define IDM_PRINTER_ADD       101
#define IDM_PRINTER_DELETE    102
#define IDM_PRINTER_PROPS     103
#define IDM_PRINTER_EXIT      109

#define IDM_JOB               200
#define IDM_JOB_HOLD          201
#define IDM_JOB_RELEASE       202
#define IDM_JOB_DELETE        203
#define IDM_JOB_PURGE         204

#define IDM_VIEW              300
#define IDM_VIEW_REFRESH      301

#define IDM_HELP              400
#define IDM_HELP_ABOUT        401

/* Dialog IDs */
#define DLG_ADDPRINTER        500
#define DLG_QUEUEPROPS        501
#define DLG_ABOUT             502

/* Controls */
#define ID_LV_PRINTERS        600   /* listbox: queues/printers        */
#define ID_LV_JOBS            601   /* listbox: jobs for selected queue */
#define ID_ST_QSTATUS         602   /* status text for selected queue   */
#define ID_ST_JOBCOUNT        603

/* Add-printer dialog */
#define ID_EF_PNAME           610
#define ID_EF_PDRIVER         611
#define ID_EF_PPORT           612
#define ID_CB_PPORT           613
#define ID_BTN_ADD_OK         614
#define ID_BTN_ADD_CANCEL     615

/* ------------------------------------------------------------------ */
/* Command-line argument parsing (mirrors SETUPARG.ASM logic)         */
/* ------------------------------------------------------------------ */
typedef enum {
    MODE_NORMAL,      /* show full Printers window                    */
    MODE_SETUP,       /* open Add Printer wizard                      */
    MODE_QUEUE_PROPS, /* open properties for a specific queue         */
    MODE_INSTALL      /* silent install                               */
} RUNMODE;

typedef struct _APPARGS {
    RUNMODE mode;
    CHAR    szQueueName[QNLEN+1];  /* valid when MODE_QUEUE_PROPS    */
} APPARGS;

static VOID ParseArgs(PSZ pszCmdLine, APPARGS *pArgs)
{
    PSZ p = pszCmdLine;
    memset(pArgs, 0, sizeof(*pArgs));
    pArgs->mode = MODE_NORMAL;
    if (!p || !*p) return;

    /* Skip leading whitespace */
    while (*p == ' ' || *p == '\t') p++;

    if (strnicmp(p, "/SETUP", 6) == 0)
        pArgs->mode = MODE_SETUP;
    else if (strnicmp(p, "/INSTALL", 8) == 0)
        pArgs->mode = MODE_INSTALL;
    else if (strnicmp(p, "/QUEUE:", 7) == 0) {
        pArgs->mode = MODE_QUEUE_PROPS;
        strncpy(pArgs->szQueueName, p + 7, QNLEN);
        /* strip trailing whitespace */
        PSZ e = pArgs->szQueueName + strlen(pArgs->szQueueName);
        while (e > pArgs->szQueueName && *(e-1) == ' ') *--e = '\0';
    }
}

/* ------------------------------------------------------------------ */
/* Main window class and data                                         */
/* ------------------------------------------------------------------ */
#define WC_PMSPOOL  "PMSpoolerWnd"

typedef struct _WNDDATA {
    CHAR szCurrentQueue[QNLEN+1];
} WNDDATA;

static HAB  g_hab   = NULLHANDLE;
static HMQ  g_hmq   = NULLHANDLE;
static HWND g_hwndFrame = NULLHANDLE;
static HWND g_hwndClient = NULLHANDLE;

/* ------------------------------------------------------------------ */
/* Helpers: populate listboxes from PMSPL                             */
/* ------------------------------------------------------------------ */

static VOID RefreshPrinterList(HWND hwnd)
{
    BYTE   buf[8192];
    ULONG  cRet = 0, cTot = 0, cbNeed = 0;
    HWND   hwndLB = WinWindowFromID(hwnd, ID_LV_PRINTERS);
    SPLERR rc;
    USHORT i;

    WinSendMsg(hwndLB, LM_DELETEALL, 0, 0);
    rc = SplEnumQueue(NULL, 3, buf, sizeof(buf),
                      &cRet, &cTot, &cbNeed, NULL);
    if (rc != SPL_OK || cRet == 0) return;

    PPRQINFO3 pQ = (PPRQINFO3)buf;
    for (i = 0; i < (USHORT)cRet; i++) {
        CHAR szItem[QNLEN + 64];
        CHAR szStatus[24];
        if (pQ->fsStatus & PRQ3_PAUSED)
            strcpy(szStatus, "(paused)");
        else if (pQ->fsStatus & PRQ3_PENDING)
            strcpy(szStatus, "(pending delete)");
        else
            strcpy(szStatus, "");
        sprintf(szItem, "%-13s  %2u job(s)  %s",
                pQ->pszName ? pQ->pszName : "?",
                pQ->cJobs, szStatus);
        WinSendMsg(hwndLB, LM_INSERTITEM,
                   MPFROMSHORT(LIT_SORTASCENDING), MPFROMP(szItem));
        pQ = (PPRQINFO3)((BYTE *)pQ + sizeof(PRQINFO3));
    }
}

static VOID RefreshJobList(HWND hwnd, PSZ pszQueue)
{
    BYTE   buf[8192];
    ULONG  cRet = 0, cTot = 0, cbNeed = 0;
    HWND   hwndLB = WinWindowFromID(hwnd, ID_LV_JOBS);
    SPLERR rc;
    USHORT i;

    WinSendMsg(hwndLB, LM_DELETEALL, 0, 0);
    if (!pszQueue || !*pszQueue) return;

    rc = SplEnumJob(NULL, pszQueue, 3, buf, sizeof(buf),
                    &cRet, &cTot, &cbNeed, NULL);
    if (rc != SPL_OK || cRet == 0) return;

    PPRJINFO3 pJ = (PPRJINFO3)buf;
    for (i = 0; i < (USHORT)cRet; i++) {
        CHAR szItem[128];
        CHAR *pszSt = "queued";
        USHORT qs = pJ->fsStatus & PRJ_QSTATUS;
        if (qs == PRJ_QS_PAUSED)   pszSt = "paused";
        if (qs == PRJ_QS_SPOOLING) pszSt = "spooling";
        if (qs == PRJ_QS_PRINTING) pszSt = "printing";
        sprintf(szItem, "#%-4u  %-28s  %8lu B  [%s]",
                pJ->uJobId,
                pJ->pszDocument ? pJ->pszDocument : "(unnamed)",
                pJ->ulSize, pszSt);
        WinSendMsg(hwndLB, LM_INSERTITEM,
                   MPFROMSHORT(LIT_END), MPFROMP(szItem));
        pJ = (PPRJINFO3)((BYTE *)pJ + sizeof(PRJINFO3));
    }

    CHAR szCount[64];
    sprintf(szCount, "%lu job(s) in queue", cRet);
    WinSetDlgItemText(hwnd, ID_ST_JOBCOUNT, szCount);
}

static VOID GetSelectedQueueName(HWND hwnd, PSZ pszBuf, ULONG cbBuf)
{
    HWND  hwndLB = WinWindowFromID(hwnd, ID_LV_PRINTERS);
    SHORT sIdx   = (SHORT)WinSendMsg(hwndLB, LM_QUERYSELECTION,
                                      MPFROMSHORT(LIT_FIRST), 0);
    CHAR  szItem[128];
    pszBuf[0] = '\0';
    if (sIdx == LIT_NONE) return;
    WinSendMsg(hwndLB, LM_QUERYITEMTEXT,
               MPFROM2SHORT(sIdx, sizeof(szItem)), MPFROMP(szItem));
    sscanf(szItem, "%13s", pszBuf);
}

static USHORT GetSelectedJobId(HWND hwnd)
{
    HWND  hwndLB = WinWindowFromID(hwnd, ID_LV_JOBS);
    SHORT sIdx   = (SHORT)WinSendMsg(hwndLB, LM_QUERYSELECTION,
                                      MPFROMSHORT(LIT_FIRST), 0);
    CHAR  szItem[128];
    USHORT uId = 0;
    if (sIdx == LIT_NONE) return 0;
    WinSendMsg(hwndLB, LM_QUERYITEMTEXT,
               MPFROM2SHORT(sIdx, sizeof(szItem)), MPFROMP(szItem));
    sscanf(szItem, "#%hu", &uId);
    return uId;
}

/* ------------------------------------------------------------------ */
/* Add-Printer dialog                                                  */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY AddPrinterDlgProc(HWND hwnd, ULONG msg,
                                           MPARAM mp1, MPARAM mp2)
{
    switch (msg) {
    case WM_INITDLG:
        {
            /* Populate port combobox with available ports */
            BYTE   buf[4096];
            ULONG  cRet = 0, cTot = 0, cbNeed = 0;
            HWND   hwndCB = WinWindowFromID(hwnd, ID_CB_PPORT);
            WinSendMsg(hwndCB, LM_DELETEALL, 0, 0);
            if (SplEnumPort(NULL, 0, buf, sizeof(buf),
                            &cRet, &cTot, &cbNeed, NULL) == SPL_OK) {
                PPRPORTINFO p = (PPRPORTINFO)buf;
                USHORT i;
                for (i = 0; i < (USHORT)cRet; i++) {
                    WinSendMsg(hwndCB, LM_INSERTITEM,
                               MPFROMSHORT(LIT_SORTASCENDING),
                               MPFROMP(p->szPortName));
                    p++;
                }
            }
            /* Select first entry */
            WinSendMsg(hwndCB, LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ID_EF_PNAME));
        }
        return (MRESULT)FALSE;

    case WM_COMMAND:
        if (SHORT1FROMMP(mp1) == ID_BTN_ADD_OK) {
            CHAR szName[QNLEN+1], szDriver[DRIV_NAME_SIZE+DRIV_DEVICENAME_SIZE+2];
            CHAR szPort[PDLEN+1];
            PRQINFO3 qi;

            WinQueryDlgItemText(hwnd, ID_EF_PNAME,   sizeof(szName),   szName);
            WinQueryDlgItemText(hwnd, ID_EF_PDRIVER, sizeof(szDriver), szDriver);

            /* Port comes from the combobox selection */
            SHORT sIdx = (SHORT)WinSendMsg(
                WinWindowFromID(hwnd, ID_CB_PPORT),
                LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), 0);
            szPort[0] = '\0';
            if (sIdx != LIT_NONE)
                WinSendMsg(WinWindowFromID(hwnd, ID_CB_PPORT),
                           LM_QUERYITEMTEXT,
                           MPFROM2SHORT(sIdx, sizeof(szPort)),
                           MPFROMP(szPort));

            if (!szName[0]) {
                WinMessageBox(HWND_DESKTOP, hwnd,
                              "Please enter a printer name.",
                              "Add Printer", 0, MB_OK | MB_ICONEXCLAMATION);
                return (MRESULT)0;
            }
            if (!szDriver[0]) {
                WinMessageBox(HWND_DESKTOP, hwnd,
                              "Please enter a printer driver name.",
                              "Add Printer", 0, MB_OK | MB_ICONEXCLAMATION);
                return (MRESULT)0;
            }

            memset(&qi, 0, sizeof(qi));
            qi.pszName       = szName;
            qi.pszDriverName = szDriver;
            qi.pszPrinters   = szPort;
            qi.pszPrProc     = "PMPRINT";
            qi.uPriority     = PRQ_DEF_PRIORITY;
            qi.fsStatus      = PRQ_ACTIVE;
            qi.fsType        = 0;

            SPLERR rc = SplCreateQueue(NULL, 3, &qi, sizeof(qi));
            if (rc != SPL_OK) {
                CHAR szMsg[128];
                sprintf(szMsg, "Could not create printer queue (error %lu).", rc);
                WinMessageBox(HWND_DESKTOP, hwnd, szMsg,
                              "Add Printer", 0, MB_OK | MB_ERROR);
                return (MRESULT)0;
            }
            WinDismissDlg(hwnd, DID_OK);
        } else if (SHORT1FROMMP(mp1) == ID_BTN_ADD_CANCEL) {
            WinDismissDlg(hwnd, DID_CANCEL);
        }
        return (MRESULT)0;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------------ */
/* About dialog                                                        */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg,
                                      MPARAM mp1, MPARAM mp2)
{
    if (msg == WM_COMMAND) {
        WinDismissDlg(hwnd, DID_OK);
        return (MRESULT)0;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------------ */
/* Client window procedure                                             */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg,
                                       MPARAM mp1, MPARAM mp2)
{
    WNDDATA *pData = (WNDDATA *)WinQueryWindowPtr(hwnd, QWL_USER);

    switch (msg) {

    case WM_CREATE:
        {
            WNDDATA *pd = (WNDDATA *)malloc(sizeof(WNDDATA));
            if (pd) {
                memset(pd, 0, sizeof(*pd));
                WinSetWindowPtr(hwnd, QWL_USER, pd);
            }

            SWP swp;
            WinQueryWindowPos(hwnd, &swp);
            LONG cx = swp.cx ? swp.cx : 600;
            LONG cy = swp.cy ? swp.cy : 400;

            /* Left pane: printer list */
            WinCreateWindow(hwnd, WC_LISTBOX, "",
                            WS_VISIBLE | LS_NOADJUSTPOS,
                            0, cy/2, cx/2, cy/2,
                            hwnd, HWND_TOP, ID_LV_PRINTERS, NULL, NULL);

            /* Right pane: job list */
            WinCreateWindow(hwnd, WC_LISTBOX, "",
                            WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL,
                            cx/2, cy/2, cx/2, cy/2,
                            hwnd, HWND_TOP, ID_LV_JOBS, NULL, NULL);

            /* Bottom status bar */
            WinCreateWindow(hwnd, WC_STATIC, "Select a printer to view its jobs.",
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER,
                            0, 0, cx/2, cy/2,
                            hwnd, HWND_TOP, ID_ST_QSTATUS, NULL, NULL);

            WinCreateWindow(hwnd, WC_STATIC, "",
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER,
                            cx/2, 0, cx/2, cy/2,
                            hwnd, HWND_TOP, ID_ST_JOBCOUNT, NULL, NULL);

            RefreshPrinterList(hwnd);
        }
        return (MRESULT)0;

    case WM_SIZE:
        {
            LONG cx = SHORT1FROMMP(mp2);
            LONG cy = SHORT2FROMMP(mp2);
            LONG barH = 20;
            LONG listH = cy - barH;

            WinSetWindowPos(WinWindowFromID(hwnd, ID_LV_PRINTERS),
                            NULLHANDLE, 0, barH, cx/2, listH,
                            SWP_MOVE | SWP_SIZE);
            WinSetWindowPos(WinWindowFromID(hwnd, ID_LV_JOBS),
                            NULLHANDLE, cx/2, barH, cx/2, listH,
                            SWP_MOVE | SWP_SIZE);
            WinSetWindowPos(WinWindowFromID(hwnd, ID_ST_QSTATUS),
                            NULLHANDLE, 0, 0, cx/2, barH,
                            SWP_MOVE | SWP_SIZE);
            WinSetWindowPos(WinWindowFromID(hwnd, ID_ST_JOBCOUNT),
                            NULLHANDLE, cx/2, 0, cx/2, barH,
                            SWP_MOVE | SWP_SIZE);
        }
        return (MRESULT)0;

    case WM_CONTROL:
        if (SHORT1FROMMP(mp1) == ID_LV_PRINTERS &&
            SHORT2FROMMP(mp1) == LN_SELECT) {
            CHAR szQ[QNLEN+1];
            GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
            if (pData) strncpy(pData->szCurrentQueue, szQ, QNLEN);
            /* Update status */
            CHAR szSt[QNLEN+32];
            if (szQ[0]) {
                sprintf(szSt, "Printer: %s", szQ);
                WinSetDlgItemText(hwnd, ID_ST_QSTATUS, szSt);
            }
            RefreshJobList(hwnd, szQ);
        }
        return (MRESULT)0;

    case WM_COMMAND:
        switch (SHORT1FROMMP(mp1)) {

        case IDM_PRINTER_ADD:
            if (WinDlgBox(HWND_DESKTOP, hwnd, AddPrinterDlgProc,
                          NULLHANDLE, DLG_ADDPRINTER, NULL) == DID_OK)
                RefreshPrinterList(hwnd);
            break;

        case IDM_PRINTER_DELETE:
            {
                CHAR szQ[QNLEN+1];
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                if (!szQ[0]) break;
                CHAR szMsg[QNLEN+64];
                sprintf(szMsg, "Delete printer \"%s\"?", szQ);
                if (WinMessageBox(HWND_DESKTOP, hwnd, szMsg,
                                  "Delete Printer", 0,
                                  MB_YESNO | MB_ICONQUESTION) == MBID_YES) {
                    SplDeleteQueue(NULL, szQ);
                    RefreshPrinterList(hwnd);
                    WinSendMsg(WinWindowFromID(hwnd, ID_LV_JOBS),
                               LM_DELETEALL, 0, 0);
                    if (pData) pData->szCurrentQueue[0] = '\0';
                }
            }
            break;

        case IDM_PRINTER_PROPS:
            /* Show queue properties - reuse SPOOLCP if available,
               otherwise show a minimal info box. */
            {
                CHAR szQ[QNLEN+1];
                BYTE buf[4096];
                ULONG cbNeed = 0;
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                if (!szQ[0]) break;
                if (SplQueryQueue(NULL, szQ, 3, buf, sizeof(buf),
                                  &cbNeed) == SPL_OK) {
                    PPRQINFO3 pQ = (PPRQINFO3)buf;
                    CHAR szInfo[512];
                    sprintf(szInfo,
                            "Name:    %s\n"
                            "Driver:  %s\n"
                            "Port:    %s\n"
                            "Jobs:    %u\n"
                            "Status:  %s",
                            pQ->pszName       ? pQ->pszName       : "",
                            pQ->pszDriverName ? pQ->pszDriverName : "",
                            pQ->pszPrinters   ? pQ->pszPrinters   : "",
                            pQ->cJobs,
                            (pQ->fsStatus & PRQ3_PAUSED) ? "Paused" : "Active");
                    WinMessageBox(HWND_DESKTOP, hwnd, szInfo,
                                  "Printer Properties", 0,
                                  MB_OK | MB_INFORMATION | MB_MOVEABLE);
                }
            }
            break;

        case IDM_PRINTER_EXIT:
            WinPostMsg(hwnd, WM_QUIT, 0, 0);
            break;

        case IDM_JOB_HOLD:
            {
                CHAR   szQ[QNLEN+1];
                USHORT uJob;
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                uJob = GetSelectedJobId(hwnd);
                if (uJob && szQ[0]) {
                    SplHoldJob(NULL, szQ, uJob);
                    RefreshJobList(hwnd, szQ);
                } else if (szQ[0]) {
                    SplHoldQueue(NULL, szQ);
                    RefreshPrinterList(hwnd);
                }
            }
            break;

        case IDM_JOB_RELEASE:
            {
                CHAR   szQ[QNLEN+1];
                USHORT uJob;
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                uJob = GetSelectedJobId(hwnd);
                if (uJob && szQ[0]) {
                    SplReleaseJob(NULL, szQ, uJob);
                    RefreshJobList(hwnd, szQ);
                } else if (szQ[0]) {
                    SplReleaseQueue(NULL, szQ);
                    RefreshPrinterList(hwnd);
                }
            }
            break;

        case IDM_JOB_DELETE:
            {
                CHAR   szQ[QNLEN+1];
                USHORT uJob;
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                uJob = GetSelectedJobId(hwnd);
                if (uJob && szQ[0]) {
                    if (WinMessageBox(HWND_DESKTOP, hwnd,
                                      "Delete this print job?",
                                      "Delete Job", 0,
                                      MB_YESNO | MB_ICONQUESTION) == MBID_YES) {
                        SplDeleteJob(NULL, szQ, uJob);
                        RefreshJobList(hwnd, szQ);
                        RefreshPrinterList(hwnd);
                    }
                }
            }
            break;

        case IDM_JOB_PURGE:
            {
                CHAR szQ[QNLEN+1];
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                if (szQ[0]) {
                    CHAR szMsg[QNLEN+48];
                    sprintf(szMsg, "Delete all jobs in \"%s\"?", szQ);
                    if (WinMessageBox(HWND_DESKTOP, hwnd, szMsg,
                                      "Purge Queue", 0,
                                      MB_YESNO | MB_ICONQUESTION) == MBID_YES) {
                        SplPurgeQueue(NULL, szQ);
                        RefreshJobList(hwnd, szQ);
                        RefreshPrinterList(hwnd);
                    }
                }
            }
            break;

        case IDM_VIEW_REFRESH:
            {
                CHAR szQ[QNLEN+1];
                GetSelectedQueueName(hwnd, szQ, sizeof(szQ));
                RefreshPrinterList(hwnd);
                if (szQ[0]) RefreshJobList(hwnd, szQ);
            }
            break;

        case IDM_HELP_ABOUT:
            WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc,
                      NULLHANDLE, DLG_ABOUT, NULL);
            break;
        }
        return (MRESULT)0;

    case WM_ERASEBACKGROUND:
        return (MRESULT)TRUE;

    case WM_DESTROY:
        if (pData) {
            free(pData);
            WinSetWindowPtr(hwnd, QWL_USER, NULL);
        }
        return (MRESULT)0;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------------ */
/* Queue Properties dialog (MODE_QUEUE_PROPS direct entry)            */
/* ------------------------------------------------------------------ */

static VOID RunQueuePropsMode(HWND hwndOwner, PSZ pszQueueName)
{
    BYTE  buf[4096];
    ULONG cbNeed = 0;
    CHAR  szInfo[512];

    if (SplQueryQueue(NULL, pszQueueName, 3,
                      buf, sizeof(buf), &cbNeed) != SPL_OK) {
        CHAR szMsg[QNLEN+64];
        sprintf(szMsg, "Cannot find printer queue \"%s\".", pszQueueName);
        WinMessageBox(HWND_DESKTOP, hwndOwner, szMsg,
                      "Printers", 0, MB_OK | MB_ERROR);
        return;
    }
    PPRQINFO3 pQ = (PPRQINFO3)buf;
    sprintf(szInfo,
            "Name:    %s\n"
            "Driver:  %s\n"
            "Port:    %s\n"
            "Jobs:    %u\n"
            "Status:  %s",
            pQ->pszName       ? pQ->pszName       : "",
            pQ->pszDriverName ? pQ->pszDriverName : "",
            pQ->pszPrinters   ? pQ->pszPrinters   : "",
            pQ->cJobs,
            (pQ->fsStatus & PRQ3_PAUSED) ? "Paused" : "Active");
    WinMessageBox(HWND_DESKTOP, hwndOwner, szInfo,
                  "Printer Properties", 0,
                  MB_OK | MB_INFORMATION | MB_MOVEABLE);
}

/* ------------------------------------------------------------------ */
/* WinMain                                                             */
/* ------------------------------------------------------------------ */

int main(void)
{
    APPARGS args;
    QMSG    qmsg;
    ULONG   flCreate;
    PSZ     pszCmdLine;

    /* Get the command line from the process info block */
    {
        PTIB  ptib;
        PPIB  ppib;
        DosGetInfoBlocks(&ptib, &ppib);
        pszCmdLine = ppib->pib_pchcmd;
        /* skip over argv[0] (program name) */
        while (*pszCmdLine) pszCmdLine++;
        pszCmdLine++;
    }

    ParseArgs(pszCmdLine, &args);

    /* Silent install mode: just ensure the spooler is started */
    if (args.mode == MODE_INSTALL)
        return 0;

    g_hab = WinInitialize(0);
    if (!g_hab) return 1;

    g_hmq = WinCreateMsgQueue(g_hab, 0);
    if (!g_hmq) {
        WinTerminate(g_hab);
        return 1;
    }

    /* Register the client window class */
    WinRegisterClass(g_hab, WC_PMSPOOL, ClientWndProc,
                     CS_SIZEREDRAW | CS_MOVENOTIFY, sizeof(PVOID));

    /* For MODE_SETUP, show the Add Printer dialog immediately after
       the main window is created (posted as a command message). */

    flCreate = FCF_SYSMENU | FCF_TITLEBAR | FCF_MINMAX | FCF_SIZEBORDER |
               FCF_TASKLIST | FCF_MENU | FCF_ICON | FCF_SHELLPOSITION;

    g_hwndFrame = WinCreateStdWindow(
                      HWND_DESKTOP,
                      WS_VISIBLE,
                      &flCreate,
                      WC_PMSPOOL,
                      "OS/2 Printers",
                      0L,
                      NULLHANDLE,
                      IDR_MAINMENU,
                      &g_hwndClient);

    if (!g_hwndFrame) {
        WinDestroyMsgQueue(g_hmq);
        WinTerminate(g_hab);
        return 1;
    }

    /* Handle special startup modes */
    if (args.mode == MODE_SETUP) {
        /* Post command to open Add Printer dialog after message loop starts */
        WinPostMsg(g_hwndClient, WM_COMMAND,
                   MPFROMSHORT(IDM_PRINTER_ADD), 0);
    } else if (args.mode == MODE_QUEUE_PROPS) {
        RunQueuePropsMode(g_hwndFrame, args.szQueueName);
        WinDestroyWindow(g_hwndFrame);
        WinDestroyMsgQueue(g_hmq);
        WinTerminate(g_hab);
        return 0;
    }

    /* Standard PM message loop */
    while (WinGetMsg(g_hab, &qmsg, NULLHANDLE, 0, 0))
        WinDispatchMsg(g_hab, &qmsg);

    WinDestroyWindow(g_hwndFrame);
    WinDestroyMsgQueue(g_hmq);
    WinTerminate(g_hab);
    return 0;
}
