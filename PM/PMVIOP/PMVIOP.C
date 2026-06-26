/*
 * PMVIOP.C -- OS/2 PM Internal VIO Support Library stub
 *
 * Replacement for PMVIOP.DLL (LX, 66820 bytes, IBM)
 * Module description: 'OS/2 PM Internal VIO Support Library'
 * Module name: 'pmviop'
 *
 * Build (32-bit):
 *   wcc386 -bt=OS2 -bm -wx -s -fo=PMVIOP.OBJ PMVIOP.C
 *   wlink @PMVIOP.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format:  LX, 5 objects
 *   Obj1: 16-bit CODE  57860 bytes  -- main VIO shield implementation
 *   Obj2: 16-bit CODE   3309 bytes  -- 16-bit helper/thunk stubs
 *   Obj3: 32-bit CODE    265 bytes  -- 32-bit entry bridge (tiny)
 *   Obj4: 16-bit RW SHR 3012 bytes  -- shared console/VIO state
 *   Obj5: 32-bit RO SHR 32792 bytes -- font tables, UI strings
 *
 * Imports: PMMERGE(32), DOSCALLS(10), PMGPI(7), PMSHLTKT(3)
 *
 * Exports (68 functions):
 *   Win* console functions (ords 1-36): WinCreateConsole, WinSyncWithPS,
 *     WinConsoleOpen/CloseKbd, WinConsoleSet/GetStatus, WinConsoleSetCp,
 *     WinConsoleGetCp, WinConsoleKbdXlate, WinConsoleSetCusTxt,
 *     WinConsoleGet/FreeFocus, WinConsoleReadKeyEvent,
 *     WinConsoleFlushKeyEvents, WinConsoleQueuedPointerEvents,
 *     WinConsoleReadPointerEvent, WinConsoleFlushPointerEvents,
 *     WinConsoleGetPointerFilterMask, WinConsoleSetPointerFilterMask,
 *     WinConsoleGetPtrPos, WinConsoleSetPtrPos,
 *     WinShieldAbort, ShieldFrameProc, VioShieldWindowProc,
 *     LockVioPs, UnlockVioPs, MarkingWindowProc, WinDefAVioWindowProc,
 *     WinConsoleGetKbdHandle, WinConsoleAttachMouseUser,
 *     WinConsoleDetachMouseUser, TaskMgrSubclassProc,
 *     WinShieldPreThaw, WinInitShield, Mortician
 *   Misc internal functions (ords 39-57): UnlockSemaphores, ClearData,
 *     QueryConsole, MakeVisible, GetPasteChar, TerminateClipDDE,
 *     VDMClipDDENotify, PM32ClipDDENotify, NotifyAllVDMs,
 *     VDMSwapButtonNotify, GET32DISPLAYTYPE, VDMRegisterHook,
 *     VDMDeregisterHook
 *   Data exports (ords 101-122): ScreenGroupToFPConsole, ApFnVdmServ,
 *     GetSelAccess, NpSzStrings, KeyboardEvent, EstablishKbdEnvironment,
 *     HMyModuleHandle, fSeamlessCaptured, HMQTaskMgr,
 *     pCharXlateTblDefault, HAppSeamlessCommonVDM,
 *     HAppSeamlessCommonVDM31, HAppSeamlessCommonVDM31Enh,
 *     fKbdHook, fMouHook, SelCommonList, USCommonSegSize,
 *     HModSeamless, HFrameWindowFromConsole
 *
 * Architecture notes:
 *   PMVIOP.DLL is the PM VIO "shield" -- it mediates between
 *   VIO text sessions and the PM window system. It creates shield
 *   windows for VIO/DOS sessions, handles keyboard/mouse routing,
 *   and manages the clipboard DDE for VDM sessions.
 *   Some exports (ords 101-122) are DATA symbols, not functions.
 *   In a stub, we export a zero ULONG for each data symbol.
 * ============================================================
 */

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>

/* Return value for failed stubs */
#define VIOP_ERROR  0UL

/* Data stubs -- exported as ULONG variables */
ULONG ScreenGroupToFPConsole = 0;
#pragma aux ScreenGroupToFPConsole "SCREENGROUPTOFPCONSOLE"

ULONG ApFnVdmServ = 0;
#pragma aux ApFnVdmServ "APFNVDMSERV"

ULONG GetSelAccess = 0;
#pragma aux GetSelAccess "GETSELACCESS"

ULONG NpSzStrings = 0;
#pragma aux NpSzStrings "NPSZSTRINGS"

ULONG KeyboardEvent = 0;
#pragma aux KeyboardEvent "KEYBOARDEVENT"

ULONG EstablishKbdEnvironment = 0;
#pragma aux EstablishKbdEnvironment "ESTABLISHKBDENVIRONMENT"

ULONG HMyModuleHandle = 0;
#pragma aux HMyModuleHandle "HMYMODULEHANDLE"

ULONG fSeamlessCaptured = 0;
#pragma aux fSeamlessCaptured "FSEAMLESSCAPTURED"

ULONG HMQTaskMgr = 0;
#pragma aux HMQTaskMgr "HMQTASKMGR"

ULONG pCharXlateTblDefault = 0;
#pragma aux pCharXlateTblDefault "PCHARXLATETBLDEFAULT"

ULONG HAppSeamlessCommonVDM = 0;
#pragma aux HAppSeamlessCommonVDM "HAPPSEAMLESSCOMMONVDM"

ULONG HAppSeamlessCommonVDM31 = 0;
#pragma aux HAppSeamlessCommonVDM31 "HAPPSEAMLESSCOMMONVDM31"

ULONG HAppSeamlessCommonVDM31Enh = 0;
#pragma aux HAppSeamlessCommonVDM31Enh "HAPPSEAMLESSCOMMONVDM31ENH"

ULONG fKbdHook = 0;
#pragma aux fKbdHook "FKBDHOOK"

ULONG fMouHook = 0;
#pragma aux fMouHook "FMOUHOOK"

ULONG SelCommonList = 0;
#pragma aux SelCommonList "SELCOMMONLIST"

ULONG USCommonSegSize = 0;
#pragma aux USCommonSegSize "USCOMMONSEGSIZE"

ULONG HModSeamless = 0;
#pragma aux HModSeamless "HMODSEAMLESS"

ULONG HFrameWindowFromConsole = 0;
#pragma aux HFrameWindowFromConsole "HFRAMEWINDOWFROMCONSOLE"

/* ================================================================== */
/* Win console functions (ords 1-36)                                  */
/* ================================================================== */

HWND APIENTRY WINCREATECONSOLE(HAB hab, ULONG ulGroup)
{ (void)hab;(void)ulGroup; return NULLHANDLE; }
#pragma aux WINCREATECONSOLE "WINCREATECONSOLE"

BOOL APIENTRY WINDESTROYCONSOLE(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux WINDESTROYCONSOLE "WINDESTROYCONSOLE"

BOOL APIENTRY WINSYNCWITHPS(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux WINSYNCWITHPS "WINSYNCWITHPS"

BOOL APIENTRY WINCONSOLEOPENKBD(HWND hwndConsole, PULONG phKbd)
{ (void)hwndConsole;(void)phKbd; return FALSE; }
#pragma aux WINCONSOLEOPENKBD "WINCONSOLEOPENKBD"

BOOL APIENTRY WINCONSOLECLOSEKBD(HWND hwndConsole, ULONG hKbd)
{ (void)hwndConsole;(void)hKbd; return FALSE; }
#pragma aux WINCONSOLECLOSEKBD "WINCONSOLECLOSEKBD"

BOOL APIENTRY WINCONSOLESETSTATUS(HWND hwndConsole, ULONG ulStatus)
{ (void)hwndConsole;(void)ulStatus; return FALSE; }
#pragma aux WINCONSOLESETSTATUS "WINCONSOLESETSTATUS"

ULONG APIENTRY WINCONSOLEGETSTATUS(HWND hwndConsole)
{ (void)hwndConsole; return VIOP_ERROR; }
#pragma aux WINCONSOLEGETSTATUS "WINCONSOLEGETSTATUS"

BOOL APIENTRY WINCONSOLESETCP(HWND hwndConsole, ULONG ulCodePage)
{ (void)hwndConsole;(void)ulCodePage; return FALSE; }
#pragma aux WINCONSOLESETCP "WINCONSOLESETCP"

ULONG APIENTRY WINCONSOLEGETCP(HWND hwndConsole)
{ (void)hwndConsole; return VIOP_ERROR; }
#pragma aux WINCONSOLEGETCP "WINCONSOLEGETCP"

BOOL APIENTRY WINCONSOLEKBDXLATE(HWND hwndConsole, PVOID pXlateTbl)
{ (void)hwndConsole;(void)pXlateTbl; return FALSE; }
#pragma aux WINCONSOLEKBDXLATE "WINCONSOLEKBDXLATE"

BOOL APIENTRY WINCONSOLESETCUSTXT(HWND hwndConsole, PVOID pCusTxt)
{ (void)hwndConsole;(void)pCusTxt; return FALSE; }
#pragma aux WINCONSOLESETCUSTXT "WINCONSOLESETCUSTXT"

BOOL APIENTRY WINCONSOLEGETFOCUS(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux WINCONSOLEGETFOCUS "WINCONSOLEGETFOCUS"

BOOL APIENTRY WINCONSOLEFREEFOCUS(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux WINCONSOLEFREEFOCUS "WINCONSOLEFREEFOCUS"

BOOL APIENTRY WINCONSOLEREADKEYEVENT(HWND hwndConsole, PVOID pKeyEvent)
{ (void)hwndConsole;(void)pKeyEvent; return FALSE; }
#pragma aux WINCONSOLEREADKEYEVENT "WINCONSOLEREADKEYEVENT"

BOOL APIENTRY WINCONSOLEFLUSHKEYEVENTS(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux WINCONSOLEFLUSHKEYEVENTS "WINCONSOLEFLUSHKEYEVENTS"

ULONG APIENTRY WINCONSOLEQUEUEDPOINTEREVENTS(HWND hwndConsole)
{ (void)hwndConsole; return VIOP_ERROR; }
#pragma aux WINCONSOLEQUEUEDPOINTEREVENTS "WINCONSOLEQUEUEDPOINTEREVENTS"

BOOL APIENTRY WINCONSOLEREADPOINTEREVENT(HWND hwndConsole, PVOID pPtrEvent)
{ (void)hwndConsole;(void)pPtrEvent; return FALSE; }
#pragma aux WINCONSOLEREADPOINTEREVENT "WINCONSOLEREADPOINTEREVENT"

BOOL APIENTRY WINCONSOLEFLUSHPOINTEREVENTS(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux WINCONSOLEFLUSHPOINTEREVENTS "WINCONSOLEFLUSHPOINTEREVENTS"

ULONG APIENTRY WINCONSOLEGETPOINTERFILTERMASK(HWND hwndConsole)
{ (void)hwndConsole; return VIOP_ERROR; }
#pragma aux WINCONSOLEGETPOINTERFILTERMASK "WINCONSOLEGETPOINTERFILTERMASK"

BOOL APIENTRY WINCONSOLESETPOINTERFILTERMASK(HWND hwndConsole, ULONG ulMask)
{ (void)hwndConsole;(void)ulMask; return FALSE; }
#pragma aux WINCONSOLESETPOINTERFILTERMASK "WINCONSOLESETPOINTERFILTERMASK"

BOOL APIENTRY WINCONSOLEGETPTRPOS(HWND hwndConsole, PPOINTL pptl)
{ (void)hwndConsole;(void)pptl; return FALSE; }
#pragma aux WINCONSOLEGETPTRPOS "WINCONSOLEGETPTRPOS"

BOOL APIENTRY WINCONSOLESETPTRPOS(HWND hwndConsole, PPOINTL pptl)
{ (void)hwndConsole;(void)pptl; return FALSE; }
#pragma aux WINCONSOLESETPTRPOS "WINCONSOLESETPTRPOS"

BOOL APIENTRY WINSHIELDABORT(HWND hwndConsole, ULONG ulReason)
{ (void)hwndConsole;(void)ulReason; return FALSE; }
#pragma aux WINSHIELDABORT "WINSHIELDABORT"

MRESULT APIENTRY SHIELDFRAMEPROC(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ (void)hwnd;(void)msg;(void)mp1;(void)mp2; return (MRESULT)0; }
#pragma aux SHIELDFRAMEPROC "SHIELDFRAMEPROC"

MRESULT APIENTRY VIOSHIELDWINDOWPROC(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ (void)hwnd;(void)msg;(void)mp1;(void)mp2; return (MRESULT)0; }
#pragma aux VIOSHIELDWINDOWPROC "VIOSHIELDWINDOWPROC"

BOOL APIENTRY LOCKVIOPS(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux LOCKVIOPS "LOCKVIOPS"

BOOL APIENTRY UNLOCKVIOPS(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux UNLOCKVIOPS "UNLOCKVIOPS"

MRESULT APIENTRY MARKINGWINDOWPROC(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ (void)hwnd;(void)msg;(void)mp1;(void)mp2; return (MRESULT)0; }
#pragma aux MARKINGWINDOWPROC "MARKINGWINDOWPROC"

MRESULT APIENTRY WINDEFAVIOWINDOWPROC(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ (void)hwnd;(void)msg;(void)mp1;(void)mp2;
  return WinDefWindowProc(hwnd, msg, mp1, mp2); }
#pragma aux WINDEFAVIOWINDOWPROC "WINDEFAVIOWINDOWPROC"

ULONG APIENTRY WINCONSOLEGETKBDHANDLE(HWND hwndConsole)
{ (void)hwndConsole; return VIOP_ERROR; }
#pragma aux WINCONSOLEGETKBDHANDLE "WINCONSOLEGETKBDHANDLE"

BOOL APIENTRY WINCONSOLEATTACHMOUSEUSER(HWND hwndConsole, ULONG ulUser)
{ (void)hwndConsole;(void)ulUser; return FALSE; }
#pragma aux WINCONSOLEATTACHMOUSEUSER "WINCONSOLEATTACHMOUSEUSER"

BOOL APIENTRY WINCONSOLEDETACHMOUSEUSER(HWND hwndConsole, ULONG ulUser)
{ (void)hwndConsole;(void)ulUser; return FALSE; }
#pragma aux WINCONSOLEDETACHMOUSEUSER "WINCONSOLEDETACHMOUSEUSER"

MRESULT APIENTRY TASKMGRSUBCLASSPROC(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{ (void)hwnd;(void)msg;(void)mp1;(void)mp2; return (MRESULT)0; }
#pragma aux TASKMGRSUBCLASSPROC "TASKMGRSUBCLASSPROC"

BOOL APIENTRY WINSHIELDPRETHAW(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux WINSHIELDPRETHAW "WINSHIELDPRETHAW"

BOOL APIENTRY WININITSHIELD(HAB hab)
{ (void)hab; return FALSE; }
#pragma aux WININITSHIELD "WININITSHIELD"

BOOL APIENTRY MORTICIAN(HWND hwndConsole, ULONG ulReason)
{ (void)hwndConsole;(void)ulReason; return FALSE; }
#pragma aux MORTICIAN "MORTICIAN"

/* ================================================================== */
/* Misc internal functions (ords 39-57)                               */
/* ================================================================== */

BOOL APIENTRY UNLOCKSEMAPHORES(ULONG ulFlags)
{ (void)ulFlags; return FALSE; }
#pragma aux UNLOCKSEMAPHORES "UNLOCKSEMAPHORES"

BOOL APIENTRY CLEARDATA(PVOID pData, ULONG cbData)
{ (void)pData;(void)cbData; return FALSE; }
#pragma aux CLEARDATA "CLEARDATA"

ULONG APIENTRY QUERYCONSOLE(HWND hwnd)
{ (void)hwnd; return VIOP_ERROR; }
#pragma aux QUERYCONSOLE "QUERYCONSOLE"

BOOL APIENTRY MAKEVISIBLE(HWND hwndConsole)
{ (void)hwndConsole; return FALSE; }
#pragma aux MAKEVISIBLE "MAKEVISIBLE"

ULONG APIENTRY GETPASTECHAR(HWND hwndConsole)
{ (void)hwndConsole; return VIOP_ERROR; }
#pragma aux GETPASTECHAR "GETPASTECHAR"

BOOL APIENTRY TERMINATECLIPDDE(HWND hwnd)
{ (void)hwnd; return FALSE; }
#pragma aux TERMINATECLIPDDE "TERMINATECLIPDDE"

BOOL APIENTRY VDMCLIPDDENOTIFY(HWND hwnd, ULONG ulType, PVOID pData)
{ (void)hwnd;(void)ulType;(void)pData; return FALSE; }
#pragma aux VDMCLIPDDENOTIFY "VDMCLIPDDENOTIFY"

BOOL APIENTRY PM32CLIPDDENOTIFY(HWND hwnd, ULONG ulType, PVOID pData)
{ (void)hwnd;(void)ulType;(void)pData; return FALSE; }
#pragma aux PM32CLIPDDENOTIFY "PM32CLIPDDENOTIFY"

BOOL APIENTRY NOTIFYALLVDMS(ULONG ulMsg, PVOID pData)
{ (void)ulMsg;(void)pData; return FALSE; }
#pragma aux NOTIFYALLVDMS "NOTIFYALLVDMS"

BOOL APIENTRY VDMSWAPBUTTONNOTIFY(ULONG fSwapped)
{ (void)fSwapped; return FALSE; }
#pragma aux VDMSWAPBUTTONNOTIFY "VDMSWAPBUTTONNOTIFY"

ULONG APIENTRY GET32DISPLAYTYPE(VOID)
{ return VIOP_ERROR; }
#pragma aux GET32DISPLAYTYPE "GET32DISPLAYTYPE"

BOOL APIENTRY VDMREGISTERHOOK(ULONG ulHookType, PVOID pfnHook)
{ (void)ulHookType;(void)pfnHook; return FALSE; }
#pragma aux VDMREGISTERHOOK "VDMREGISTERHOOK"

BOOL APIENTRY VDMDEREGISTERHOOK(ULONG ulHookType, PVOID pfnHook)
{ (void)ulHookType;(void)pfnHook; return FALSE; }
#pragma aux VDMDEREGISTERHOOK "VDMDEREGISTERHOOK"
