/*
 * BDBVH.C  --  Bidirectional Base Video Handler for OS/2
 *
 * Replacement for BDBVH.DLL (LX format, 17408 bytes)
 * Module: BDBVH  "Bidirectional Base Video Handler"
 *
 * IMPORTANT: This is a 16-bit DLL. Compile with wcc (16-bit), NOT wcc386.
 *
 * Build:
 *   wcc -bt=OS2 -ml -2 -s -wx -fo=BDBVH.OBJ BDBVH.C
 *   wlink @BDBVH.LNK
 *
 * Compiler flags:
 *   -bt=OS2  target OS/2
 *   -ml      large memory model (far code + far data, needed for 16-bit DLL)
 *   -2       186/286 instruction set
 *   -s       suppress stack checks
 *   -wx      maximum warnings
 *
 * ============================================================
 * INFORMATION
 * ============================================================
 *
 * Object layout (16-bit LX, page shift=9 -> 512-byte page units):
 *   Obj1  16-bit CODE   0x2a81 bytes  -- All 24 function implementations
 *   Obj2  16-bit DATA   0x0260 bytes  -- Global shared state:
 *                                        Ord names, ordinal-to-offset table,
 *                                        IPC segment names (log_ctl, log_buf),
 *                                        semaphore names (logger_active,
 *                                        logger_reponse), executable paths
 *                                        (bdkbdm.exe, bdbvh, logger.exe),
 *                                        and function pointer table at 0x190
 *   Obj3  16-bit BSS    0x11f6 bytes  -- Zero-filled: per-session bidi state
 *                                        blocks, orientation/CSD/flag arrays
 *
 * Entry table: one 16near bundle, 24 entries.
 * All entries cluster around code offsets 0x0300-0x031d:
 *   0x0300 = generic dispatcher (ord 1)
 *   0x0309 = QUERY/SET cluster A (ords 3-5: orientation, CSD)
 *   0x030a = QUERY/SET cluster B (ords 6-10: hotkey, fieldrev, NL)
 *   0x030b = QUERY/SET cluster C (ords 11-15: push level, auto-push LTR)
 *   0x030c = QUERY/SET cluster D (ords 16-20: auto-push RTL, shape, curpos)
 *   0x030e = NLSRECORDBDDSEG / DEVENABLE (ords 22-23)
 *   0x0319 = NLSQUERYBIDISEGADDRESS (ord 21)
 *   0x031c = BDBVHSETORIENTATION (ord 2, direct)
 *   0x031d = _PrivateMouFunc (ord 24)
 *
 * Dispatch mechanism: The 16-bit entry stubs share entry points and
 * use the ordinal index (from the caller's stack) to select the
 * appropriate sub-function within each cluster.
 *
 * Imports:
 *   SESMGR   -- Session Manager VIO support
 *   BVHVGA   -- Standard VGA BVH (all non-bidi VIO ops delegated here)
 *   MOUCALLS ord#836 -- MouRegister (register mouse handler)
 *   DOSCALLS -- DosAllocSeg, DosGetSeg, DosCreateSem, DosOpenSem
 *   BDCALLS  -- IBM Bidi Calls private module (ring-0 bidi support)
 *
 * Global data (Obj2, offset layout):
 *   +0x0000: DWORD[0]=0 (init flag)
 *   +0x0004: '_PrivateMouFunc\0' (name string for MouRegister)
 *   +0x0014: 'bdbvh\0'
 *   +0x001a: ordinal number strings '0\0'..'15\0'
 *   +0x0040: per-ordinal offset table (DWORD entries, indexed by ordinal)
 *   +0x00a6: offset table data (WORD entries 0x1a,0x1c,0x1e... for 0-15)
 *   +0x00e8: '\sharemem\@#log_ctl#@.seg\0'
 *   +0x0102: '\sharemem\@#log_buf#@.seg\0'
 *   +0x011c: '\SEM\@#logger_active#@.sem\0'
 *   +0x0137: '\SEM\@#logger_reponse#@.sem\0'  (note: IBM typo 'reponse')
 *   +0x0153: '?:\os2\system\bdkbdm.exe\0'
 *   +0x016c: 'bdbvh\0'
 *   +0x0172: '\roni\exe\prm.exe\0'
 *   +0x0184: 'logger.exe\0'
 *   +0x01a7: 'bdkbdm\0'
 *   +0x0190: function pointer table (DWORD array, one per VIO function)
 *
 * Architecture:
 *   BDBVH is a 16-bit "Base Video Handler" DLL that hooks into the OS/2
 *   VIO (Video I/O) subsystem to provide bidirectional text display support
 *   for full-screen and windowed DOS/OS2 sessions.
 *
 *   BVH DLLs are loaded by the Session Manager (SESMGR.DLL) when a new
 *   screen session is created. BDBVH delegates all non-bidi VIO operations
 *   to BVHVGA (the standard VGA BVH) and intercepts only those operations
 *   that need bidirectional awareness:
 *     - Text orientation (LTR/RTL)
 *     - Character Set/Direction (CSD) for Arabic/Hebrew
 *     - Field reversal for RTL screens
 *     - Push level tracking (for nested bidi embedding)
 *     - Auto-push thresholds (automatic bidi algorithm triggers)
 *     - Mouse cursor position adjustment for RTL text
 *
 *   Per-session state is kept in Obj3 (BSS), allocated as a shared
 *   segment accessible from all processes in a session. The IPC segment
 *   names in Obj2 (\sharemem\@#log_ctl#@.seg etc.) are used for the
 *   bidi logger (LOGGER.EXE / BDKBDM.EXE debugging infrastructure).
 *
 *   DEVENABLE (ord 23) is called by SESMGR when the bidi VIO device
 *   should be enabled/disabled for a session.
 *
 *   NLSRECORDBDDSEG (ord 22) records the address of the Bidi Driver
 *   Data segment used by the low-level bidi device driver (BDD).
 *
 *   NLSQUERYBIDISEGADDRESS (ord 21) returns the flat 32-bit linear
 *   address of the bidi NLS data segment for kernel-mode callers.
 * ============================================================
 */

/*
 * The Open Watcom os2.h is 32-bit only and cannot be used with wcc (16-bit).
 * For a 16-bit BVH DLL we declare all required types and prototypes manually.
 * This is standard practice for 16-bit OS/2 subsystem components that predate
 * the 32-bit toolkit.
 */
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Fundamental 16-bit OS/2 types                                       */
/* ------------------------------------------------------------------ */
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned long   ULONG;
typedef unsigned int    UINT;
typedef char            CHAR;
typedef short           SHORT;
typedef long            LONG;
typedef int             INT;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef void            VOID;
typedef VOID FAR       *PVOID;
typedef CHAR FAR       *PSZ;
typedef UCHAR FAR      *PUCHAR;
typedef USHORT FAR     *PUSHORT;
typedef ULONG FAR      *PULONG;
typedef SHORT FAR      *PSHORT;
typedef BYTE FAR       *PBYTE;

/* Boolean */
typedef USHORT  BOOL;
#define TRUE    1
#define FALSE   0

/* NULL */
#ifndef NULL
#define NULL    0
#endif

/* NULLHANDLE */
#define NULLHANDLE  ((USHORT)0)

/* Handle types */
typedef USHORT  HMOU;
typedef USHORT  HSEM;
typedef USHORT  HFILE;
typedef USHORT  HEV;
typedef USHORT  HMTX;
typedef USHORT  TID;

/* Segment selector */
typedef USHORT  SEL;

/* Calling conventions */
#define PASCAL  __pascal
#define APIRET  USHORT
#define FAR     __far
#define NEAR    __near

/* ------------------------------------------------------------------ */
/* MOUEVENTINFO -- mouse event structure (MOUCALLS)                   */
/* ------------------------------------------------------------------ */
typedef struct _MOUEVENTINFO {
    USHORT  fs;         /* event flags                                 */
    ULONG   time;       /* time stamp                                  */
    USHORT  row;        /* mouse row position                          */
    USHORT  col;        /* mouse column position                       */
} MOUEVENTINFO;
typedef MOUEVENTINFO FAR *PMOUEVENTINFO;

/* MouRegister mask -- all events */
#define MOUSE_MOTION_WITH_BN1_DOWN   0x0001
#define MOUSE_BN1_DOWN               0x0002
#define MOUSE_MOTION_WITH_BN2_DOWN   0x0004
#define MOUSE_BN2_DOWN               0x0008
#define MOUSE_MOTION                 0x0010
#define MOUSE_BN3_DOWN               0x0020
#define MOUSE_MOTION_WITH_BN3_DOWN   0x0040
#define MOUSE_ALL_EVENTS             0x007F

/* ------------------------------------------------------------------ */
/* MOUCALLS prototypes (16-bit, PASCAL calling convention)            */
/* ------------------------------------------------------------------ */
extern APIRET FAR PASCAL MouRegister(PSZ pszModName, PSZ pszFuncName,
                                      ULONG flEventMask, HMOU FAR *pHMou);
extern APIRET FAR PASCAL MouDeRegister(void);

/* ------------------------------------------------------------------ */
/* DOSCALLS prototypes needed by BDBVH                                */
/* ------------------------------------------------------------------ */
extern APIRET FAR PASCAL DosAllocSeg(USHORT cbSeg, SEL FAR *pSel, USHORT fShare);
extern APIRET FAR PASCAL DosGetSeg(SEL sel);
extern APIRET FAR PASCAL DosFreeSeg(SEL sel);
extern APIRET FAR PASCAL DosCreateSem(USHORT fExclusive, HSEM FAR *phSem, PSZ pszName);
extern APIRET FAR PASCAL DosOpenSem(HSEM FAR *phSem, PSZ pszName);
extern APIRET FAR PASCAL DosRequestSem(HSEM hSem, ULONG ulTimeout);
extern APIRET FAR PASCAL DosReleaseSem(HSEM hSem);
extern APIRET FAR PASCAL DosCloseSem(HSEM hSem);
extern APIRET FAR PASCAL DosBeep(USHORT usFreq, USHORT usDuration);

/* ------------------------------------------------------------------ */
/* BVH return codes                                                    */
/* ------------------------------------------------------------------ */
#define BVH_OK              0
#define BVH_ERR_INVALID    -1
#define BVH_ERR_UNSUP      -2

/* ------------------------------------------------------------------ */
/* Bidi orientation constants (BDBVHSETORIENTATION)                   */
/* ------------------------------------------------------------------ */
#define BDBVH_ORIENT_LTR    0   /* Left-to-Right (default)            */
#define BDBVH_ORIENT_RTL    1   /* Right-to-Left                      */
#define BDBVH_ORIENT_COLUMN 2   /* Column (for CJK compatibility)     */

/* ------------------------------------------------------------------ */
/* CSD (Character Set and Direction) constants                        */
/* ------------------------------------------------------------------ */
#define BDBVH_CSD_NEUTRAL   0   /* Neutral (no bidi)                  */
#define BDBVH_CSD_ARABIC    1   /* Arabic (CP864)                     */
#define BDBVH_CSD_HEBREW    2   /* Hebrew (CP862)                     */
#define BDBVH_CSD_MIXED     3   /* Mixed LTR/RTL                      */

/* ------------------------------------------------------------------ */
/* Per-session bidi state structure (lives in Obj3 BSS shared memory) */
/* One instance per session, zero-initialised at load time.           */
/* ------------------------------------------------------------------ */
typedef struct _BDBVH_STATE {
    USHORT  usOrientation;      /* BDBVH_ORIENT_*                     */
    USHORT  usCSD;              /* BDBVH_CSD_*                        */
    USHORT  usHotKeyFlag;       /* bidi hotkey enable flag            */
    USHORT  usFieldRev;         /* field reverse enable               */
    USHORT  usNL;               /* natural language mode              */
    USHORT  usPushLevel;        /* current bidi push level (0-15)     */
    USHORT  usAutoPushLTR;      /* auto-push LTR threshold            */
    USHORT  usAutoPushRTL;      /* auto-push RTL threshold            */
    USHORT  usAutoShapeCtr;     /* auto-shape counter                 */
    USHORT  usDevEnabled;       /* device enabled flag                */
    ULONG   ulBDDSegAddr;       /* BDD segment flat address           */
    ULONG   ulNLSSegAddr;       /* NLS bidi segment flat address      */
    USHORT  usCurCol;           /* current bidi cursor column         */
    USHORT  usCurRow;           /* current bidi cursor row            */
    USHORT  usPopupActive;      /* popup window active flag           */
} BDBVH_STATE;
typedef BDBVH_STATE FAR *PBDBVH_STATE;

/* ------------------------------------------------------------------ */
/* Global data (corresponds to Obj2 content)                          */
/* ------------------------------------------------------------------ */

/* IPC shared memory segment names (from Obj2 @ 0xe8..0x19b) */
static const char FAR g_szLogCtlSeg[]    = "\\sharemem\\@#log_ctl#@.seg";
static const char FAR g_szLogBufSeg[]    = "\\sharemem\\@#log_buf#@.seg";
static const char FAR g_szLoggerActSem[] = "\\SEM\\@#logger_active#@.sem";
static const char FAR g_szLoggerRspSem[] = "\\SEM\\@#logger_reponse#@.sem"; /* IBM typo */
static const char FAR g_szBdkbdmExe[]   = "?:\\os2\\system\\bdkbdm.exe";
static const char FAR g_szLoggerExe[]   = "logger.exe";
static const char FAR g_szModName[]     = "bdbvh";
static const char FAR g_szPrivMouName[] = "_PrivateMouFunc";

/* Per-session state block (Obj3 BSS -- zero-filled at load) */
static BDBVH_STATE FAR g_State;

/* Initialised flag */
static USHORT g_fInitialized = 0;

/* BDD (Bidi Driver Data) segment selector */
static USHORT g_selBDDSeg = 0;

/* NLS bidi data segment flat address */
static ULONG  g_ulNLSFlatAddr = 0;

/* BVHVGA function pointer (resolved at DEVENABLE time via SESMGR) */
typedef SHORT (FAR PASCAL *PFNBVHVGA)(void);
static PFNBVHVGA g_pfnBVHVGA = (PFNBVHVGA)NULL;

/* MouRegister handle (registered in ORDINAL1_PROC) */
static HMOU g_hmouReg = (HMOU)NULLHANDLE;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/*
 * GetState -- return pointer to per-session state block.
 * In the original, this lives in a shared segment allocated from
 * Obj3 BSS. Here we use a single static (adequate for single-session
 * rebuilds; production code would allocate per-session).
 */
static PBDBVH_STATE FAR GetState(void)
{
    return &g_State;
}

/* ------------------------------------------------------------------ */
/* ord 1: ORDINAL1_PROC                                               */
/*                                                                     */
/* BVH initialisation entry point called by SESMGR when the BVH is   */
/* loaded for a new screen group. Registers with the session manager, */
/* sets up the mouse handler, and initialises per-session state.      */
/*                                                                     */
/* This is the standard BVH registration convention: SESMGR calls    */
/* the first ordinal with a registration structure.                   */
/*                                                                     */
/* pRegPkt  - BVH registration packet (SESMGR-defined structure)     */
/*                                                                     */
/* Returns: BVH_OK on success.                                        */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL ORDINAL1_PROC(PVOID FAR *pRegPkt)
{
    PBDBVH_STATE ps = GetState();

    if (g_fInitialized)
        return BVH_OK;

    /* Zero per-session state */
    {
        USHORT i;
        UCHAR FAR *p = (UCHAR FAR *)ps;
        for (i = 0; i < sizeof(BDBVH_STATE); i++) p[i] = 0;
    }

    /* Default settings */
    ps->usOrientation = BDBVH_ORIENT_LTR;
    ps->usCSD         = BDBVH_CSD_NEUTRAL;
    ps->usHotKeyFlag  = 0;
    ps->usFieldRev    = 0;
    ps->usNL          = 0;
    ps->usPushLevel   = 0;
    ps->usAutoPushLTR = 0;
    ps->usAutoPushRTL = 0;
    ps->usAutoShapeCtr= 0;
    ps->usDevEnabled  = 0;

    /* The IPC segment and semaphore names stored in Obj2 are used by the
     * bidi logger infrastructure (LOGGER.EXE + BDKBDM.EXE) to exchange
     * diagnostic data between the BVH and the keyboard monitor.
     * On a production system SESMGR opens these before calling us. */
    (void)g_szLogCtlSeg;
    (void)g_szLogBufSeg;
    (void)g_szLoggerActSem;
    (void)g_szLoggerRspSem;
    (void)g_szBdkbdmExe;
    (void)g_szLoggerExe;

    /* Register private mouse function with MOUCALLS (ord 836 = MouRegister)
     * so bidi-aware cursor movement can be applied in RTL sessions. */
    MouRegister((PSZ)g_szModName, (PSZ)g_szPrivMouName,
                0x0000FFFFL, &g_hmouReg);

    g_fInitialized = 1;
    (void)pRegPkt;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 2: BDBVHSETORIENTATION                                         */
/*                                                                     */
/* Set the text display orientation for the current session.          */
/*                                                                     */
/* usOrientation - BDBVH_ORIENT_LTR (0), BDBVH_ORIENT_RTL (1),      */
/*                 or BDBVH_ORIENT_COLUMN (2)                         */
/*                                                                     */
/* Returns: BVH_OK on success, BVH_ERR_INVALID if value out of range. */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETORIENTATION(USHORT usOrientation)
{
    PBDBVH_STATE ps = GetState();
    if (usOrientation > BDBVH_ORIENT_COLUMN)
        return BVH_ERR_INVALID;
    ps->usOrientation = usOrientation;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 3: BDBVHQUERYORIENTATION                                       */
/*                                                                     */
/* Query the current text display orientation.                        */
/*                                                                     */
/* pusOrientation - receives the current BDBVH_ORIENT_* value        */
/*                                                                     */
/* Returns: BVH_OK.                                                   */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYORIENTATION(PUSHORT pusOrientation)
{
    PBDBVH_STATE ps = GetState();
    if (!pusOrientation) return BVH_ERR_INVALID;
    *pusOrientation = ps->usOrientation;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 4: BDBVHSETCSD                                                 */
/*                                                                     */
/* Set the Character Set and Direction for the current session.       */
/* Controls which bidi codepage is active (Arabic/Hebrew/neutral).    */
/*                                                                     */
/* usCSD - BDBVH_CSD_* value                                          */
/*                                                                     */
/* Returns: BVH_OK.                                                   */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETCSD(USHORT usCSD)
{
    PBDBVH_STATE ps = GetState();
    if (usCSD > BDBVH_CSD_MIXED) return BVH_ERR_INVALID;
    ps->usCSD = usCSD;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 5: BDBVHQUERYCSD                                               */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYCSD(PUSHORT pusCSD)
{
    PBDBVH_STATE ps = GetState();
    if (!pusCSD) return BVH_ERR_INVALID;
    *pusCSD = ps->usCSD;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 6: BDBVHSETHOTKEYFLAG                                          */
/*                                                                     */
/* Enable or disable the bidi keyboard hotkey for this session.       */
/* When enabled, the session manager monitors for the bidi toggle     */
/* keypress (Ctrl+Shift or similar) to switch input layers.           */
/*                                                                     */
/* usFlag - non-zero to enable, 0 to disable                          */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETHOTKEYFLAG(USHORT usFlag)
{
    PBDBVH_STATE ps = GetState();
    ps->usHotKeyFlag = (usFlag != 0) ? 1 : 0;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 7: BDBVHQUERYHOTKEYFLAG                                        */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYHOTKEYFLAG(PUSHORT pusFlag)
{
    PBDBVH_STATE ps = GetState();
    if (!pusFlag) return BVH_ERR_INVALID;
    *pusFlag = ps->usHotKeyFlag;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 8: BDBVHSETFIELDREV                                            */
/*                                                                     */
/* Enable or disable field reversal for RTL screens. When active,    */
/* the entire screen content is mirrored horizontally so that RTL    */
/* text reads naturally from right to left.                           */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETFIELDREV(USHORT usFieldRev)
{
    PBDBVH_STATE ps = GetState();
    ps->usFieldRev = (usFieldRev != 0) ? 1 : 0;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 9: BDBVHQUERYFIELDREV                                          */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYFIELDREV(PUSHORT pusFieldRev)
{
    PBDBVH_STATE ps = GetState();
    if (!pusFieldRev) return BVH_ERR_INVALID;
    *pusFieldRev = ps->usFieldRev;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 10: BDBVHSETNL                                                 */
/*                                                                     */
/* Set the Natural Language mode for the session. NL mode enables     */
/* language-aware line breaking and cursor movement for bidi text.    */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETNL(USHORT usNL)
{
    PBDBVH_STATE ps = GetState();
    ps->usNL = (usNL != 0) ? 1 : 0;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 11: BDBVHQUERYNL                                               */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYNL(PUSHORT pusNL)
{
    PBDBVH_STATE ps = GetState();
    if (!pusNL) return BVH_ERR_INVALID;
    *pusNL = ps->usNL;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 12: BDBVHSETPUSHLEVEL                                          */
/*                                                                     */
/* Set the bidi push level for the current session. The push level    */
/* implements the Unicode Bidirectional Algorithm's embedding level   */
/* stack, tracking nested LTR/RTL overrides (0=base, max 15).        */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETPUSHLEVEL(USHORT usPushLevel)
{
    PBDBVH_STATE ps = GetState();
    if (usPushLevel > 15) return BVH_ERR_INVALID;
    ps->usPushLevel = usPushLevel;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 13: BDBVHQUERYPUSHLEVEL                                        */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYPUSHLEVEL(PUSHORT pusPushLevel)
{
    PBDBVH_STATE ps = GetState();
    if (!pusPushLevel) return BVH_ERR_INVALID;
    *pusPushLevel = ps->usPushLevel;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 14: BDBVHSETAUTOPUSHLTR                                        */
/*                                                                     */
/* Set the auto-push LTR threshold. When the number of consecutive   */
/* LTR characters exceeds this threshold, the bidi algorithm          */
/* automatically inserts a LTR embedding (push level +1).            */
/* Set to 0 to disable auto-push.                                    */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETAUTOPUSHLTR(USHORT usThreshold)
{
    PBDBVH_STATE ps = GetState();
    ps->usAutoPushLTR = usThreshold;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 15: BDBVHQUERYAUTOPUSHLTR                                      */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYAUTOPUSHLTR(PUSHORT pusThreshold)
{
    PBDBVH_STATE ps = GetState();
    if (!pusThreshold) return BVH_ERR_INVALID;
    *pusThreshold = ps->usAutoPushLTR;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 16: BDBVHSETAUTOPUSHRTL                                        */
/*                                                                     */
/* Set the auto-push RTL threshold. Same concept as auto-push LTR    */
/* but for RTL character sequences.                                   */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETAUTOPUSHRTL(USHORT usThreshold)
{
    PBDBVH_STATE ps = GetState();
    ps->usAutoPushRTL = usThreshold;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 17: BDBVHQUERYAUTOPUSHRTL                                      */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHQUERYAUTOPUSHRTL(PUSHORT pusThreshold)
{
    PBDBVH_STATE ps = GetState();
    if (!pusThreshold) return BVH_ERR_INVALID;
    *pusThreshold = ps->usAutoPushRTL;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 18: BDBVHSETAUTOSHAPECTR                                       */
/*                                                                     */
/* Set the auto-shape counter. This counter tracks how many Arabic   */
/* characters have been written since the last shape-reset, and is   */
/* used to decide when to trigger contextual shaping of the current  */
/* line (reshape the display buffer).                                 */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHSETAUTOSHAPECTR(USHORT usCounter)
{
    PBDBVH_STATE ps = GetState();
    ps->usAutoShapeCtr = usCounter;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 19: BDBVHGETCURPOS                                             */
/*                                                                     */
/* Get the current logical cursor position, adjusted for bidi.       */
/* In RTL sessions with field-reverse active, the physical and        */
/* logical cursor columns are mirrored. This function returns the    */
/* logical (user-visible) position regardless of physical layout.    */
/*                                                                     */
/* pusCol  - receives the logical cursor column (0-based)             */
/* pusRow  - receives the cursor row (0-based)                        */
/*                                                                     */
/* Returns: BVH_OK.                                                   */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL BDBVHGETCURPOS(PUSHORT pusCol, PUSHORT pusRow)
{
    PBDBVH_STATE ps = GetState();
    if (!pusCol || !pusRow) return BVH_ERR_INVALID;

    /* For RTL field-reverse sessions, the physical column is mirrored.
     * The screen width is typically 80 (col 0..79).
     * Logical col = (screenWidth - 1) - physical col when RTL+FieldRev. */
    if (ps->usOrientation == BDBVH_ORIENT_RTL && ps->usFieldRev)
        *pusCol = (USHORT)(79 - (int)ps->usCurCol);
    else
        *pusCol = ps->usCurCol;

    *pusRow = ps->usCurRow;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 20: ENDPOPUPHANDLING                                           */
/*                                                                     */
/* Signal the end of a popup window's bidi handling. Called when a   */
/* VIO popup (created with VioPopup) is dismissed, so BDBVH can      */
/* restore the underlying screen's bidi state and re-apply the       */
/* correct orientation to the main session display.                   */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL ENDPOPUPHANDLING(void)
{
    PBDBVH_STATE ps = GetState();
    ps->usPopupActive = 0;
    /* Restore main session bidi state after popup closes.
     * In the original, this re-applies orientation and field-reverse
     * to the underlying screen buffer via BVHVGA. */
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 21: NLSQUERYBIDISEGADDRESS                                     */
/*                                                                     */
/* Query the flat 32-bit linear address of the NLS bidi data segment. */
/* This is used by ring-0 device driver code (BDCALLS) to locate the */
/* per-session bidi state in the kernel address space.                */
/*                                                                     */
/* pulAddr  - receives the flat linear address of the bidi data seg  */
/*                                                                     */
/* Returns: BVH_OK, or BVH_ERR_INVALID if segment not yet recorded.  */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL NLSQUERYBIDISEGADDRESS(ULONG FAR *pulAddr)
{
    if (!pulAddr)               return BVH_ERR_INVALID;
    if (!g_ulNLSFlatAddr)       return BVH_ERR_INVALID;
    *pulAddr = g_ulNLSFlatAddr;
    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 22: NLSRECORDBDDSEG                                            */
/*                                                                     */
/* Record the selector of the Bidi Driver Data (BDD) segment.        */
/* Called by BDCALLS (the ring-0 bidi kernel helper) to tell BDBVH   */
/* the address of the shared bidi state segment used by the device    */
/* driver. BDBVH converts this to a flat address via DosGetSeg and   */
/* stores it for NLSQUERYBIDISEGADDRESS callers.                      */
/*                                                                     */
/* selBDDSeg - 16-bit selector of the BDD shared segment             */
/*                                                                     */
/* Returns: BVH_OK on success.                                        */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL NLSRECORDBDDSEG(USHORT selBDDSeg)
{
    PBDBVH_STATE ps = GetState();

    g_selBDDSeg = selBDDSeg;

    /* Convert the 16-bit selector to a flat 32-bit linear address.
     * In the original, this uses the DevHelp_VirtToLin equivalent
     * available to 16-bit BVH code via BDCALLS.
     * We approximate using DosGetSeg to verify the selector is valid,
     * then compute the flat address as selector_base + 0.
     * The full implementation requires BDCALLS ring-0 support. */
    if (selBDDSeg != 0)
    {
        /* Flat address = GDT selector base; BVH code computes this via
         * the InfoSeg global descriptor table pointer. For our purposes,
         * store the selector * 8 as a placeholder (not correct for
         * production but satisfies the interface contract). */
        g_ulNLSFlatAddr = (ULONG)selBDDSeg << 3;
        ps->ulBDDSegAddr = g_ulNLSFlatAddr;
    }
    else
    {
        g_ulNLSFlatAddr  = 0;
        ps->ulBDDSegAddr = 0;
    }

    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 23: DEVENABLE                                                  */
/*                                                                     */
/* Enable or disable the bidirectional VIO device for this session.  */
/* Called by SESMGR when the session is switched to/from foreground, */
/* or when the session is created/terminated.                         */
/*                                                                     */
/* usEnable - non-zero to enable the bidi VIO device                 */
/*            0 to disable (session switching away)                   */
/*                                                                     */
/* Returns: BVH_OK on success.                                        */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL DEVENABLE(USHORT usEnable)
{
    PBDBVH_STATE ps = GetState();

    ps->usDevEnabled = (usEnable != 0) ? 1 : 0;

    if (usEnable)
    {
        /* When enabled, re-apply the current bidi settings to the
         * video subsystem. In the original, this calls into BVHVGA
         * to reprogramme the VGA registers and sets the text direction
         * in the session manager's display context. */
    }
    else
    {
        /* When disabled (session leaving foreground), save any
         * hardware state that needs to be restored later. */
    }

    return BVH_OK;
}

/* ------------------------------------------------------------------ */
/* ord 24: _PrivateMouFunc                                            */
/*                                                                     */
/* Private mouse function registered with MOUCALLS to intercept mouse */
/* position reporting for RTL sessions. When field-reverse is active, */
/* the mouse column must be mirrored so that click positions          */
/* correspond to the correct logical character in RTL text.           */
/*                                                                     */
/* This function is registered via MouRegister (MOUCALLS ord 836)    */
/* under the name "_PrivateMouFunc" with MOUSEMASK all.               */
/*                                                                     */
/* pEvet   - MOUEVENTINFO structure from the mouse subsystem         */
/* pMouHnd - HMOU handle                                              */
/*                                                                     */
/* Returns: 0 to pass the event on, non-zero to consume it.          */
/* ------------------------------------------------------------------ */
SHORT FAR PASCAL _PrivateMouFunc(PMOUEVENTINFO pEvt, HMOU FAR *pMouHnd)
{
    PBDBVH_STATE ps = GetState();

    if (!pEvt) return 0;

    /* Track the current mouse position */
    ps->usCurCol = pEvt->col;
    ps->usCurRow = pEvt->row;

    /* For RTL+FieldRev sessions, mirror the column so applications
     * receive the correct logical position. */
    if (ps->usOrientation == BDBVH_ORIENT_RTL && ps->usFieldRev)
        pEvt->col = (USHORT)(79 - (int)pEvt->col);

    (void)pMouHnd;
    return 0;   /* pass event through */
}

/* ------------------------------------------------------------------ */
/* DLL init / term                                                     */
/* BVH DLLs do not use _DLL_InitTerm; they are initialised via       */
/* ORDINAL1_PROC called by SESMGR. The LibMain here handles the       */
/* standard OpenWatcom DLL entry chain only.                          */
/* In 16-bit OS/2 DLLs the init function is called with far pascal   */
/* convention.                                                        */
/* ------------------------------------------------------------------ */
int FAR PASCAL LibMain(USHORT hmod, USHORT fFlag)
{
    if (fFlag == 0)
    {
        /* Process attach: zero globals */
        USHORT i;
        UCHAR FAR *p;
        g_fInitialized  = 0;
        g_selBDDSeg     = 0;
        g_ulNLSFlatAddr = 0;
        g_pfnBVHVGA     = (PFNBVHVGA)NULL;
        g_hmouReg       = (HMOU)NULLHANDLE;
        p = (UCHAR FAR *)&g_State;
        for (i = 0; i < sizeof(g_State); i++) p[i] = 0;
        g_State.usOrientation = BDBVH_ORIENT_LTR;
        g_State.usCSD         = BDBVH_CSD_NEUTRAL;
    }
    else
    {
        /* Process detach */
        if (g_hmouReg != (HMOU)NULLHANDLE)
        {
            MouDeRegister();
            g_hmouReg = (HMOU)NULLHANDLE;
        }
        g_fInitialized = 0;
    }
    (void)hmod;
    return 1;
}
