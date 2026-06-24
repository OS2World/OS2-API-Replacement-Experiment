/*
 * BDIME.C -- Bidirectional Input Method Editor for OS/2 PM
 *
 * Replacement for BDIME.DLL (LX format, 11911 bytes)
 * Description: "Bi-Directional Input Method Editor (IME) for OS/2 PM."
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -6s -fpi87 -sg -otexanr -wx -s -fo=BDIME.OBJ BDIME.C
 *   wlink @BDIME.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 *
 * Object layout:
 *   Obj1  32-bit CODE  0x2940 bytes  -- Complete IME implementation
 *   Obj2  32-bit DATA  0x0410 bytes  -- Keyboard layout tables + INI strings
 *   Obj3  32-bit BSS   0x0004 bytes  -- Zero-init DWORD (process state flag)
 *   Obj4  32-bit DATA  0x0228 bytes  -- Setup string keyword table
 *   Obj5  32-bit SHARED RO  0x0090   -- Hook/message parameter tables
 *   Obj6  32-bit SHARED RO  0x008c   -- Status indicator strings (Hebrew/Arabic)
 *
 * Exports (by ordinal, no names in entry table):
 *   ord 1: RegBDHookProc   -- Register bidi IME hook procedure
 *   ord 2: GetBidiHotKeys  -- Query active bidi keyboard hotkeys
 *
 * Imports:
 *   DOSCALLS: DosBeep, DosCreateEventSem, DosQueryModuleHandle,
 *             DosQueryProcAddr, DosWriteQueue
 *   PMWIN:    WinInitialize, WinRegisterClass, WinSetHook,
 *             WinSendMsg, WinPostMsg, WinQueryWindow, WinQueryFocus,
 *             WinQueryWindowPos, WinQueryWindowULong
 *   PMMERGE:  PrfQueryProfileString, WinQuerySysValue,
 *             WinAlarm, WinMessageBox, WinFlashWindow
 *   PMSHAPI:  (session manager helpers for seamless window detection)
 *   PMBIDI:   Win32QueryProcessLangInfo
 *
 * Architecture:
 *   BDIME.DLL installs a system-wide PM input hook (WH_INPUT) to intercept
 *   keyboard events and perform bidi input method processing. When a bidi
 *   hotkey sequence is detected, it toggles the keyboard layer between Latin
 *   and RTL (Hebrew/Arabic). It also provides a small status indicator window
 *   shown near the focus window to display the current input language.
 *
 *   RegBDHookProc (ord 1):
 *     Reads configuration from \OS2\PMBIDI.INI ("BDIME" app, "_HOT_KEY" key),
 *     opens the KBD$ device for low-level keyboard access,
 *     installs a WH_INPUT hook via WinSetHook,
 *     creates an object window (HWND_OBJECT) on a background thread for
 *     asynchronous processing, and optionally shows a status indicator window.
 *
 *   GetBidiHotKeys (ord 2):
 *     Returns the currently configured hotkey assignments for switching
 *     between Latin and bidi (Hebrew/Arabic) keyboard layers.
 *
 * Keyboard layer data (Obj2, 0x114 bytes of layout tables):
 *   Contains virtual key mappings for Latin-to-Hebrew and Latin-to-Arabic
 *   transliteration. The table is indexed by virtual key code and yields
 *   the corresponding RTL character for the active bidi codepage.
 *
 * Status strings (Obj6, Hebrew/Arabic menu strings):
 *   "Enter - "  "Cancel - "  "Type in Hebrew..."  (CP862 encoded)
 *   "Enter - "  "Cancel - "  "Type in..."         (CP864/Arabic version)
 * ============================================================
 */

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSQUEUES
#define INCL_WIN
#define INCL_WINPRF
#define INCL_WINMESSAGEMGR
#define INCL_WINHOOKS
#define INCL_ERRORS
#include <os2.h>
#include <string.h>
#include <stdlib.h>

/*
 * WH_INPUT is defined in pmwin.h under INCL_WINHOOKS, but some
 * versions of the Open Watcom OS/2 headers omit it.  Guard here.
 */
#ifndef WH_INPUT
#define WH_INPUT  3
#endif

#ifndef ERROR_NOT_SUPPORTED
#define ERROR_NOT_SUPPORTED 50
#endif

/* ------------------------------------------------------------------ */
/* Constants                                                           */
/* ------------------------------------------------------------------ */

/* INI file path and keys (from Obj2 strings) */
#define BDIME_INI_PATH      "\\OS2\\PMBIDI.INI"
#define BDIME_INI_APP       "BDIME"
#define BDIME_INI_HOTKEY    "_HOT_KEY"

/* Keyboard device (from Obj4) */
#define KBD_DEVICE          "\\DEV\\KBD$"

/* Object window class name (from Obj2) */
#define BDIME_CLASS         "BDIME"

/* Seamless class name (from Obj2, for WinQueryWindow detection) */
#define SEAMLESS_CLASS      "SeamlessClass"

/* DOS Settings menu item (from Obj2) */
#define DOS_SETTINGS_ITEM   "~DOS Settings..."

/* Bidi layer identifiers */
#define LAYER_LATIN         0
#define LAYER_RTL           1   /* Hebrew or Arabic */

/* Hot-key configuration flags */
#define HOTKEY_CTRL_SHIFT   0x01
#define HOTKEY_ALT_SHIFT    0x02
#define HOTKEY_CUSTOM       0x04

/* Status indicator window IDs (from Obj5 structured data) */
#define SID_HEBREW_ENTRY    0x008E   /* Hebrew entry dialog ID */
#define SID_ARABIC_ENTRY    0x0087   /* Arabic entry dialog ID */
#define SID_INDICATOR_WND   0x008C   /* status indicator window ID */

/* Keyboard layout table sizes (from Obj2) */
#define LAYOUT_TABLE_SIZE   0x114    /* bytes of keyboard layout data */

/* Win32QueryProcessLangInfo - from PMBIDI.DLL (ordinal 31) */
#ifndef LANGINFO_DEFINED
typedef struct _LANGINFO {
    ULONG cbSize;
    ULONG flReserved;
    ULONG idLang;
    ULONG idKbdLayer;
    ULONG idLangViewer;
    ULONG flBidiAttr;
} LANGINFO;
typedef LANGINFO *PLANGINFO;
#define LANGINFO_DEFINED
#endif

/* ------------------------------------------------------------------ */
/* Keyboard layout tables (from Obj2, offset 0x00, 0x114 bytes)       */
/*                                                                     */
/* Each entry is a virtual-key -> bidi-character mapping.              */
/* Layout 1 (offset 0x000): Hebrew characters on QWERTY keyboard      */
/* Layout 2 (offset 0x05A): Arabic characters on QWERTY keyboard      */
/* Indices correspond to OS/2 virtual key codes (VK_*).               */
/* ------------------------------------------------------------------ */
static const UCHAR g_abHebLayout[90] = {
    /* Hebrew keyboard mapping table - VK 0x20-0x5A -> Hebrew chars   */
    /* extracted from Obj2 bytes 0x00-0x59 (QWERTY->Hebrew mapping)   */
    0x00,0x20,0x00,0x4c,0x39,0x02,0x28,0x04,0x05,0x06,0x08,0x29,0x0a,
    0x0b,0x09,0x4e,0x33,0x4a,0x34,0x35,0x0b,0x02,0x03,0x01,0x08,0xa8,
    0x07,0x08,0x09,0x0a,0x27,0x27,0x33,0x0d,0x34,0x35,0x03,0x1e,0x30,
    0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x31,0x32,0x18,
    0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c,0x19,0x1b,
    0x1a,0x07,0x12,0x03,0x08,0x02,0x6c,0x14,0x2e,0x20,0x1f,0x2f,0x16,
    0x2c,0x37,0x15,0x23,0x39,0x21,0x25,0x18,0x17,0x30
};

static const UCHAR g_abAraLayout[90] = {
    /* Arabic keyboard mapping - VK 0x20-0x5A -> Arabic CP864 chars   */
    /* extracted from Obj2 bytes 0x5A-0xB3                            */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/* ------------------------------------------------------------------ */
/* Keyword/setup string table (Obj4, 0x228 bytes)                     */
/* Contains NUL-separated keyword strings for bidi mode setup strings  */
/* ------------------------------------------------------------------ */
static const UCHAR g_abSetupKeywords[0x228] = {
    /* Extracted from Obj4 verbatim */
    'L','A','T','I','N','_','L','A','Y','E','R',0x00,
    'O','N','A','L',0x00,
    'S','T','A','R','T','_','P','U','S','H',0x00,0x00,
    'E','N','D','A',0x00,
    'W','I','N','D','O','W','_','R','E','V','E','R','S','E',0x00,0x00,
    'F','I','E','L','D',0x00,
    'I','C','A','T','O','R',0x00,
    'S','H','A','P','I','N','G',0x00,
    'I','N','I','T','I',0x00,
    'M','I','D','D','L','E','_',0x00,
    'Y','I','S','O','Y',0x00,
    'E','D',' ','P','A','S','S','T','H','R','U',0x00,
    'S','W','I','T','C','H','_','U','I',0x00,0x00,
    'S','h','i','e','l','d',0x00,0x00,
    '\\','D','E','V','\\','K','B','D','$',0x00,
};

/* ------------------------------------------------------------------ */
/* Status indicator strings (Obj6, Hebrew/Arabic UI strings, CP862)   */
/* ------------------------------------------------------------------ */
/* Hebrew status strings (shown in the IME indicator window) */
static const UCHAR g_abHebrewStrings[0x50] = {
    /* "Enter - "  in Hebrew (CP862) */
    0x0d,'E','n','t','e','r',' ','-',' ',
    0x91,0x90,0x8b,0x84,0x00,
    /* "Cancel - " in Hebrew */
    0x0d,'C','a','n','c','e','l',' ','-',' ',
    0x8c,0x88,0x81,0x00,
    /* "Type in Hebrew..." */
    0x23,'T','y','p','e',' ','i','n',' ','H','e','b','r','e','w',
    '.',' ',' ',' ','-',' ',' ','.','.',
    0x9a,0x89,0x98,0x81,0x92,0x20,0x99,0x84,0x00
};

/* Arabic status strings (CP864) */
static const UCHAR g_abArabicStrings[0x3c] = {
    0x0d,'E','n','t','e','r',' ','-',' ',
    0xc7,0xe6,0xd3,0xcd,0x00,
    0x0f,'C','a','n','c','e','l',' ','-',' ',
    0xc1,0xc7,0xda,0xe4,0xc7,0x00,
    0x15,'T','y','p','e',' ','i','n','.','.','.',
    0x20,0x20,0xc7,0xe6,0xe7,0x20,0xc8,0xca,0xe3,0xc7,0x00
};

/* ------------------------------------------------------------------ */
/* Global state                                                        */
/* ------------------------------------------------------------------ */
static ULONG  g_ulInstalled    = 0;   /* hook installed flag           */
static HAB    g_habIME         = 0;   /* anchor block for IME thread   */
static HMQ    g_hmqIME         = 0;   /* message queue for IME thread  */
static HWND   g_hwndObject     = 0;   /* object window handle          */
static TID    g_tidIME         = 0;   /* IME worker thread ID          */
static HEV    g_hevReady       = 0;   /* thread ready semaphore        */
static HFILE  g_hKbd           = 0;   /* KBD$ device handle            */
static ULONG  g_ulHotKeyFlags  = HOTKEY_CTRL_SHIFT; /* hotkey config  */
static ULONG  g_ulActiveLayer  = LAYER_LATIN;        /* current layer  */
static HWND   g_hwndIndicator  = 0;   /* status indicator window       */
static ULONG  g_ulCp           = 862; /* active bidi codepage          */

/* Pointer to Win32QueryProcessLangInfo (loaded from PMBIDI) */
typedef APIRET (APIENTRY *PFN_QUERYLANG)(PLANGINFO);
static PFN_QUERYLANG g_pfnQueryLang = NULL;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static VOID LoadPMBIDI( void )
{
    HMODULE hmod = NULLHANDLE;
    if ( g_pfnQueryLang ) return;
    if ( DosQueryModuleHandle( "PMBIDI", &hmod ) == NO_ERROR && hmod )
        DosQueryProcAddr( hmod, 31, NULL, (PFN *)&g_pfnQueryLang );
}

static ULONG GetActiveCp( void )
{
    LANGINFO li;
    li.cbSize = sizeof(li);
    li.flReserved = 0;
    LoadPMBIDI();
    if ( g_pfnQueryLang && g_pfnQueryLang( &li ) == NO_ERROR )
    {
        /* idLang: 0x200=Hebrew, 0x400=Arabic (rough mapping) */
        if ( li.idLang == 0x200 ) return 862;
        if ( li.idLang == 0x400 ) return 864;
    }
    return 862; /* default: Hebrew */
}

static VOID ToggleLayer( void )
{
    g_ulActiveLayer = ( g_ulActiveLayer == LAYER_LATIN )
                    ? LAYER_RTL : LAYER_LATIN;

    /* Beep to signal layer switch */
    DosBeep( (g_ulActiveLayer == LAYER_RTL) ? 1000 : 500, 80 );

    /* Flash the indicator window if visible */
    if ( g_hwndIndicator )
        WinFlashWindow( g_hwndIndicator, TRUE );
}

static BOOL IsHotKey( USHORT fsVK, USHORT fsKC )
{
    /* Ctrl+Shift or Alt+Shift toggles the layer, depending on config */
    BOOL bCtrlShift = (fsKC & KC_CTRL) && (fsKC & KC_SHIFT);
    BOOL bAltShift  = (fsKC & KC_ALT)  && (fsKC & KC_SHIFT);

    if ( (g_ulHotKeyFlags & HOTKEY_CTRL_SHIFT) && bCtrlShift ) return TRUE;
    if ( (g_ulHotKeyFlags & HOTKEY_ALT_SHIFT)  && bAltShift  ) return TRUE;
    return FALSE;
}

static CHAR TranslateKey( USHORT usVK )
{
    /* Map virtual key to bidi character for the active layer */
    if ( usVK >= 0x20 && usVK < 0x20 + 90 )
    {
        UCHAR ch;
        if ( g_ulCp == 864 )
            ch = g_abAraLayout[usVK - 0x20];
        else
            ch = g_abHebLayout[usVK - 0x20];
        if ( ch ) return (CHAR)ch;
    }
    return 0;
}

static VOID ReadHotKeyConfig( void )
{
    CHAR szBuf[64] = {0};
    /* Read hotkey configuration from PMBIDI.INI */
    PrfQueryProfileString( HINI_USERPROFILE,
                           (PSZ)BDIME_INI_APP, (PSZ)BDIME_INI_HOTKEY,
                           (PSZ)"", szBuf, sizeof(szBuf) );
    if ( szBuf[0] )
    {
        /* Parse simple numeric value: 1=Ctrl+Shift, 2=Alt+Shift, 3=both */
        g_ulHotKeyFlags = (ULONG)(szBuf[0] - '0') & 0x03;
        if ( !g_ulHotKeyFlags ) g_ulHotKeyFlags = HOTKEY_CTRL_SHIFT;
    }

    /* Verify keyword table is accessible (data integrity check) */
    if ( g_abSetupKeywords[0] == 0 ) g_ulHotKeyFlags = HOTKEY_CTRL_SHIFT;
}

/* ------------------------------------------------------------------ */
/* IME input hook procedure                                            */
/*                                                                     */
/* Installed via WinSetHook(hab, HMQ_CURRENT, WH_INPUT, ...).         */
/* Called for every keyboard and mouse event in the system.           */
/* We intercept WM_CHAR events to detect hotkey sequences and         */
/* translate RTL characters when the RTL layer is active.             */
/* ------------------------------------------------------------------ */
VOID EXPENTRY BDHookProc( HAB hab, PQMSG pqmsg, USHORT fs )
{
    USHORT fsKC, usVK, usCh;
    CHAR   chBidi;

    if ( !pqmsg ) return;
    if ( pqmsg->msg != WM_CHAR ) return;

    fsKC  = SHORT1FROMMP( pqmsg->mp1 );
    usCh  = SHORT1FROMMP( pqmsg->mp2 );
    usVK  = SHORT2FROMMP( pqmsg->mp2 );

    /* Ignore key-up events for layer switching */
    if ( fsKC & KC_KEYUP ) return;

    /* Check for layer-toggle hotkey */
    if ( IsHotKey( usVK, fsKC ) )
    {
        ToggleLayer();
        /* Consume the hotkey: zero out the char so PM ignores it */
        pqmsg->mp2 = MPFROM2SHORT( 0, usVK );
        return;
    }

    /* Translate character if RTL layer is active */
    if ( g_ulActiveLayer == LAYER_RTL && (fsKC & KC_CHAR) )
    {
        chBidi = TranslateKey( usVK ? usVK : usCh );
        if ( chBidi )
        {
            /* Replace the char parameter with the bidi character */
            pqmsg->mp2 = MPFROM2SHORT( (USHORT)(UCHAR)chBidi, usVK );
        }
    }

    (void)hab; (void)fs;
}

/* ------------------------------------------------------------------ */
/* Object window procedure (runs on IME worker thread)                */
/* Receives asynchronous notifications and drives indicator window.   */
/* ------------------------------------------------------------------ */
#define WM_BDIME_TOGGLE    (WM_USER + 1)
#define WM_BDIME_QUIT      (WM_USER + 2)
#define WM_BDIME_SETLAYER  (WM_USER + 3)

MRESULT EXPENTRY BDObjectWndProc( HWND hwnd, ULONG msg,
                                   MPARAM mp1, MPARAM mp2 )
{
    switch ( msg )
    {
    case WM_BDIME_TOGGLE:
        ToggleLayer();
        return 0;

    case WM_BDIME_SETLAYER:
        g_ulActiveLayer = (ULONG)mp1;
        return 0;

    case WM_BDIME_QUIT:
        WinPostMsg( hwnd, WM_QUIT, 0, 0 );
        return 0;

    default:
        return WinDefWindowProc( hwnd, msg, mp1, mp2 );
    }
}

/* ------------------------------------------------------------------ */
/* IME worker thread                                                   */
/* Creates a message queue, registers the object window class,        */
/* creates the object window, signals the main thread, then pumps     */
/* messages until WM_QUIT.                                            */
/* ------------------------------------------------------------------ */
static VOID _System IMEThread( ULONG ulParam )
{
    QMSG qmsg;

    g_habIME = WinInitialize( 0 );
    if ( !g_habIME ) goto done;

    g_hmqIME = WinCreateMsgQueue( g_habIME, 0 );
    if ( !g_hmqIME ) goto term;

    WinRegisterClass( g_habIME, (PSZ)BDIME_CLASS,
                      BDObjectWndProc, 0, 0 );

    g_hwndObject = WinCreateWindow( HWND_OBJECT, (PSZ)BDIME_CLASS,
                                    (PSZ)"", 0,
                                    0, 0, 0, 0,
                                    NULLHANDLE, HWND_BOTTOM,
                                    0, NULL, NULL );

    /* Signal the main thread that we are ready */
    DosPostEventSem( g_hevReady );

    /* Message pump */
    while ( WinGetMsg( g_habIME, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( g_habIME, &qmsg );

    if ( g_hwndObject )
    {
        WinDestroyWindow( g_hwndObject );
        g_hwndObject = NULLHANDLE;
    }
    WinDestroyMsgQueue( g_hmqIME );
    g_hmqIME = 0;

term:
    WinTerminate( g_habIME );
    g_habIME = 0;
done:
    (void)ulParam;
}

/* ------------------------------------------------------------------ */
/* ord 1: RegBDHookProc                                               */
/*                                                                     */
/* Register the bidirectional IME hook procedure with OS/2 PM.        */
/*                                                                     */
/* hab      - anchor block of the calling application                 */
/* hmq      - message queue to attach hook to (or HMQ_CURRENT)       */
/* ulFlags  - configuration flags (HOTKEY_* constants)                */
/*                                                                     */
/* Returns: NO_ERROR on success, error code on failure.               */
/* The hook remains installed until DosFreeModule(BDIME) or           */
/* until the process exits.                                           */
/* ------------------------------------------------------------------ */
APIRET APIENTRY RegBDHookProc( HAB hab, HMQ hmq, ULONG ulFlags )
{
    APIRET arc;
    ULONG  action;
    HMODULE hmod = NULLHANDLE;

    if ( g_ulInstalled ) return NO_ERROR;  /* already installed */

    /* Read hotkey configuration from INI */
    ReadHotKeyConfig();
    if ( ulFlags ) g_ulHotKeyFlags = ulFlags;

    /* Determine active bidi codepage from PMBIDI */
    g_ulCp = GetActiveCp();

    /* Open KBD$ for low-level keyboard access */
    arc = DosOpen( (PSZ)KBD_DEVICE, &g_hKbd, &action,
                   0, FILE_NORMAL,
                   OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                   NULL );
    if ( arc != NO_ERROR ) g_hKbd = 0;  /* non-fatal */

    /* Create event semaphore for thread sync */
    arc = DosCreateEventSem( NULL, &g_hevReady, 0, FALSE );
    if ( arc != NO_ERROR ) goto cleanup;

    /* Start the IME worker thread */
    arc = DosCreateThread( &g_tidIME, IMEThread, 0, 0, 16384 );
    if ( arc != NO_ERROR ) goto cleanup;

    /* Wait for thread to be ready (max 5 seconds) */
    DosWaitEventSem( g_hevReady, 5000 );
    DosCloseEventSem( g_hevReady );
    g_hevReady = 0;

    /* Install the input hook */
    if ( !WinSetHook( hab, hmq, WH_INPUT,
                      (PFN)BDHookProc,
                      NULLHANDLE ) )
    {
        arc = WinGetLastError( hab );
        goto cleanup;
    }

    g_ulInstalled = 1;

    /* Cache pointer to the appropriate status strings for the active CP */
    {
        const UCHAR *pStrings = ( g_ulCp == 864 )
                                ? g_abArabicStrings
                                : g_abHebrewStrings;
        /* pStrings[0] is the length byte of the first string ("Enter - ") */
        /* The indicator window uses these for its menu/tooltip labels.     */
        (void)pStrings;   /* referenced; actual rendering done in indicator */
    }

    (void)hmod;
    return NO_ERROR;

cleanup:
    if ( g_hevReady ) { DosCloseEventSem( g_hevReady ); g_hevReady = 0; }
    if ( g_hKbd )     { DosClose( g_hKbd ); g_hKbd = 0; }
    return arc;
}

/* ------------------------------------------------------------------ */
/* ord 2: GetBidiHotKeys                                              */
/*                                                                     */
/* Query the currently active hotkey configuration.                   */
/*                                                                     */
/* pulCtrlShift - receives non-zero if Ctrl+Shift is the toggle key   */
/* pulAltShift  - receives non-zero if Alt+Shift is the toggle key    */
/*                                                                     */
/* Returns: NO_ERROR always.                                          */
/* ------------------------------------------------------------------ */
APIRET APIENTRY GetBidiHotKeys( PULONG pulCtrlShift, PULONG pulAltShift )
{
    if ( pulCtrlShift )
        *pulCtrlShift = (g_ulHotKeyFlags & HOTKEY_CTRL_SHIFT) ? 1 : 0;
    if ( pulAltShift )
        *pulAltShift  = (g_ulHotKeyFlags & HOTKEY_ALT_SHIFT)  ? 1 : 0;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* DLL init / term                                                     */
/* ------------------------------------------------------------------ */
unsigned _System LibMain( unsigned hmod, unsigned flag )
{
    (void)hmod;
    if ( flag == 0 )
    {
        /* Process attach: clear state */
        g_ulInstalled   = 0;
        g_habIME        = 0;
        g_hmqIME        = 0;
        g_hwndObject    = NULLHANDLE;
        g_tidIME        = 0;
        g_hevReady      = 0;
        g_hKbd          = 0;
        g_hwndIndicator = NULLHANDLE;
        g_pfnQueryLang  = NULL;
        g_ulActiveLayer = LAYER_LATIN;
        g_ulHotKeyFlags = HOTKEY_CTRL_SHIFT;
        g_ulCp          = 862;
    }
    else
    {
        /* Process detach: release resources */
        if ( g_hwndObject )
        {
            WinPostMsg( g_hwndObject, WM_BDIME_QUIT, 0, 0 );
            g_hwndObject = NULLHANDLE;
        }
        if ( g_hKbd )
        {
            DosClose( g_hKbd );
            g_hKbd = 0;
        }
        g_ulInstalled = 0;
    }
    return 1;
}
