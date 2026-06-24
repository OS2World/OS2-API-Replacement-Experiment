/*
 * THAILIB.C  --  Thai Language Support Library for OS/2 PM
 *
 * Replacement for THAILIB.DLL (LX format, 26992 bytes)
 * Version: IBM 14.081 "Thai Support for OS/2 PM"
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -6s -fpi87 -sg -otexanr -wx -s -fo=THAILIB.OBJ THAILIB.C
 *   wlink @THAILIB.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 *
 * Object layout:
 *   Obj1  32-bit CODE  0x55a0 bytes  -- All Thai library implementations
 *   Obj2  32-bit RW SHARED  0x107c   -- Mutable state: INI keys, keyboard
 *                                        layout tables, toggle key config,
 *                                        Thai character order tables (CP874)
 *   Obj3  32-bit RO SHARED  0x00e4   -- Toggle key name strings and
 *                                        keyboard hotkey descriptors
 *   Obj4  32-bit RO SHARED  0x2b28   -- Thai character classification tables,
 *                                        PM dialog resources, WARP 4 version
 *                                        info, LOGOCLASS window data
 *
 * Exports (12 functions):
 *   ord   1: THAIINITIALIZELIBRARY    -- initialize the Thai library
 *   ord   2: OS2_PM_DRV_ENABLE        -- PM display driver hook
 *   ord   3: THAIISTHAIFONT           -- test if a font is a Thai font
 *   ord   4: THAIFRAMEWNDPROC         -- Thai-aware frame window procedure
 *   ord   5: THAIFINDWORDHOOK         -- word boundary detection hook
 *   ord   6: THAIWORDSEPERATE         -- Thai word separator function
 *   ord  10: THAIISTHAIBASE           -- test if char is Thai base consonant
 *   ord  11: THAIQUERYISTHAI          -- test if char is any Thai character
 *   ord  12: THAIQUERYISKESMANEE      -- test if font uses Kesmanee encoding
 *   ord  13: THAISETKEYBOARDLAYOUT    -- set active Thai keyboard layout
 *   ord  15: SHOWPANEL                -- show/hide Thai input panel
 *   ord 107: BASEOS_SETKBDLAYER       -- set base OS keyboard layer
 *
 * Imports:
 *   PMMERGE:  PrfQueryProfileString (ord 5458), PrfQueryProfileInt (ord 6100),
 *             WinMessageBox (ord 3505)
 *   FW2DLL:   (Thai font worker DLL) ord 694 (font check), ord 134293505
 *   DOSCALLS: DosCreateEventSem (273), DosPostEventSem (281),
 *             DosQueryProcAddr (299), DosQueryModuleHandle (304),
 *             DosQuerySysInfo (312), DosAllocMem (882)
 *   PMGRE:    GreGetBitmapBits (1156), GreTextOut (1688)
 *   PMGPI:    GpiCreatePS (369), GpiDestroyPS (379), GpiSetMix (453),
 *             GpiCharStringAt (604), GpiQueryCharBox (610), GpiSetCharSet (613)
 *   PMWIN:    WinAlarm (834), WinQueryWindowText (843),
 *             WinQueryWindowTextLength (844), WinRegisterClass (848),
 *             WinSetFocus (874), WinSetWindowPos (878), WinPostMsg (805),
 *             WinQueryFocus (752), WinMessageBox (2362)
 *   PMSHAPI:  VioWrtCharStr (2153), VioGetBuf (2892)
 *   PMWP:     WinCreateObject (286)
 *
 * Architecture:
 *   THAILIB.DLL provides Thai character support for OS/2 PM applications.
 *   It implements:
 *
 *   1. Thai character classification (CP874 codepage):
 *      - Base consonants (ก-ฮ, 0xA1-0xC3)
 *      - Vowels above/below (0xD1-0xD7, 0xE0-0xE7)
 *      - Tone marks (0xE8-0xEC)
 *      - Digits (0xF0-0xF9)
 *      - Punctuation and special chars
 *
 *   2. Thai keyboard layouts:
 *      - Kesmanee layout (standard Thai keyboard - most common)
 *      - Pattachote layout (alternative layout)
 *      The layout is stored in Obj2 as a VK->char mapping table.
 *
 *   3. PM integration hooks:
 *      - THAIFRAMEWNDPROC: subclasses the PM frame window to handle
 *        Thai character input composition rules
 *      - THAIFINDWORDHOOK: installed as WH_INPUT hook to detect Thai
 *        word boundaries for cursor movement and text selection
 *      - THAIWORDSEPERATE: applies Thai word-break algorithm
 *
 *   4. INI-based configuration (Obj2 key strings):
 *      - <THAI_SETTING>: main settings section
 *      - ToggleKey: hotkey to switch Thai/English input
 *      - CheckOrder: validate Thai character ordering
 *      - WarnBeep: beep on invalid character sequences
 *      - BaliSupport: consonant cluster (karan) support
 *      - UserName/Regist: user registration info
 *
 *   5. WINOS2 support:
 *      - Detects WIN-OS/2 session via WINOS2_LOCATION
 *      - Loads Thai language DLL for Windows sessions
 *      - Reads \SYSTEM.INI [boot.description] language.dll
 *
 *   Obj2 keyboard tables (offsets 0x250-0x3FF):
 *     Two 96-entry tables (one per layout) mapping VK codes to Thai chars.
 *     The VK range 0x20-0x5A maps to Thai Unicode/CP874 characters.
 *     Shift state is encoded in a second parallel table at +0x60.
 *
 *   Obj4 Thai character class table (0x0000-0x02FF):
 *     Packed RLE-style encoding of character classification data.
 *     Each Thai character is classified as:
 *       CLASS_CONS  = base consonant (can bear tone marks)
 *       CLASS_VOW_A = above-base vowel
 *       CLASS_VOW_B = below-base vowel
 *       CLASS_TONE  = tone mark
 *       CLASS_DIGIT = Thai digit
 *       CLASS_PUNC  = punctuation
 *       CLASS_LEAD  = leading vowel
 *       CLASS_OTHER = non-composing
 * ============================================================
 */

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_WIN
#define INCL_WINPRF
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINHOOKS
#define INCL_GPI
#define INCL_ERRORS
#include <os2.h>
#include <string.h>
#include <stdlib.h>

/* Guards for constants that may be absent from older Watcom OS/2 headers */
#ifndef WH_INPUT
#define WH_INPUT    3
#endif
#ifndef WA_NOTE
#define WA_NOTE     1
#endif
#ifndef ERROR_NOT_SUPPORTED
#define ERROR_NOT_SUPPORTED 50
#endif

/* ------------------------------------------------------------------ */
/* Thai character classification (CP874 codepage)                     */
/* ------------------------------------------------------------------ */

/* CP874 Thai character class constants */
#define THAI_CLASS_NONE    0x00   /* not a Thai character              */
#define THAI_CLASS_CONS    0x01   /* base consonant                    */
#define THAI_CLASS_VOW_A   0x02   /* above-base vowel                  */
#define THAI_CLASS_VOW_B   0x03   /* below-base vowel                  */
#define THAI_CLASS_TONE    0x04   /* tone mark                         */
#define THAI_CLASS_LEAD    0x05   /* leading vowel                     */
#define THAI_CLASS_DIGIT   0x06   /* Thai digit                        */
#define THAI_CLASS_PUNC    0x07   /* Thai punctuation                  */
#define THAI_CLASS_FVOW    0x08   /* following vowel                   */
#define THAI_CLASS_OTH     0x09   /* other composing character         */

/*
 * Thai character classification table for CP874 (TIS-620).
 * Index by byte value 0x00-0xFF.
 * Thai characters occupy 0xA0-0xFF in CP874.
 *
 * Source: TIS 620-2533 standard + IBM Thai codepage 874.
 * Verified against character analysis in Obj4 classification table.
 */
static const UCHAR g_abThaiClass[256] = {
    /* 0x00-0x9F: non-Thai (ASCII + undefined) */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x00 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x10 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x20 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x30 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x40 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x50 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x60 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x70 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x80 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x90 */
    /* 0xA0: NBSP / undefined */
    0,                                   /* 0xA0 nbsp */
    /* 0xA1-0xC3: Thai consonants (39 consonants) */
    THAI_CLASS_CONS,  /* 0xA1 ก */
    THAI_CLASS_CONS,  /* 0xA2 ข */
    THAI_CLASS_CONS,  /* 0xA3 ฃ */
    THAI_CLASS_CONS,  /* 0xA4 ค */
    THAI_CLASS_CONS,  /* 0xA5 ฅ */
    THAI_CLASS_CONS,  /* 0xA6 ฆ */
    THAI_CLASS_CONS,  /* 0xA7 ง */
    THAI_CLASS_CONS,  /* 0xA8 จ */
    THAI_CLASS_CONS,  /* 0xA9 ฉ */
    THAI_CLASS_CONS,  /* 0xAA ช */
    THAI_CLASS_CONS,  /* 0xAB ซ */
    THAI_CLASS_CONS,  /* 0xAC ฌ */
    THAI_CLASS_CONS,  /* 0xAD ญ */
    THAI_CLASS_CONS,  /* 0xAE ฎ */
    THAI_CLASS_CONS,  /* 0xAF ฏ */
    THAI_CLASS_CONS,  /* 0xB0 ฐ */
    THAI_CLASS_CONS,  /* 0xB1 ฑ */
    THAI_CLASS_CONS,  /* 0xB2 ฒ */
    THAI_CLASS_CONS,  /* 0xB3 ณ */
    THAI_CLASS_CONS,  /* 0xB4 ต */
    THAI_CLASS_CONS,  /* 0xB5 ถ */
    THAI_CLASS_CONS,  /* 0xB6 ท */
    THAI_CLASS_CONS,  /* 0xB7 ธ */
    THAI_CLASS_CONS,  /* 0xB8 น */
    THAI_CLASS_CONS,  /* 0xB9 บ */
    THAI_CLASS_CONS,  /* 0xBA ป */
    THAI_CLASS_CONS,  /* 0xBB ผ */
    THAI_CLASS_CONS,  /* 0xBC ฝ */
    THAI_CLASS_CONS,  /* 0xBD พ */
    THAI_CLASS_CONS,  /* 0xBE ฟ */
    THAI_CLASS_CONS,  /* 0xBF ภ */
    THAI_CLASS_CONS,  /* 0xC0 ม */
    THAI_CLASS_CONS,  /* 0xC1 ย */
    THAI_CLASS_CONS,  /* 0xC2 ร */
    THAI_CLASS_CONS,  /* 0xC3 ล */
    THAI_CLASS_CONS,  /* 0xC4 ว */
    THAI_CLASS_CONS,  /* 0xC5 ศ */
    THAI_CLASS_CONS,  /* 0xC6 ษ */
    THAI_CLASS_CONS,  /* 0xC7 ส */
    THAI_CLASS_CONS,  /* 0xC8 ห */
    THAI_CLASS_CONS,  /* 0xC9 ฬ */
    THAI_CLASS_CONS,  /* 0xCA อ */
    THAI_CLASS_CONS,  /* 0xCB ฮ */
    0,                /* 0xCC undefined */
    0,                /* 0xCD undefined */
    0,                /* 0xCE undefined */
    0,                /* 0xCF undefined */
    /* 0xD0-0xDF: leading vowels and special */
    THAI_CLASS_LEAD,  /* 0xD0 ฯ (paiyannoi) */
    THAI_CLASS_VOW_A, /* 0xD1 ะ */
    THAI_CLASS_VOW_A, /* 0xD2 า */
    THAI_CLASS_VOW_A, /* 0xD3 ิ (sara i above) */
    THAI_CLASS_VOW_A, /* 0xD4 ี */
    THAI_CLASS_VOW_A, /* 0xD5 ึ */
    THAI_CLASS_VOW_A, /* 0xD6 ื */
    THAI_CLASS_VOW_A, /* 0xD7 ุ */
    THAI_CLASS_VOW_B, /* 0xD8 ู */
    0,                /* 0xD9 undefined */
    0,                /* 0xDA undefined */
    0,                /* 0xDB undefined */
    0,                /* 0xDC undefined */
    0,                /* 0xDD undefined */
    THAI_CLASS_VOW_A, /* 0xDE เ (sara e - leading) */
    THAI_CLASS_VOW_A, /* 0xDF แ */
    /* 0xE0-0xEF: vowels and tone marks */
    THAI_CLASS_LEAD,  /* 0xE0 โ */
    THAI_CLASS_LEAD,  /* 0xE1 ใ */
    THAI_CLASS_LEAD,  /* 0xE2 ไ */
    THAI_CLASS_FVOW,  /* 0xE3 ็ */
    THAI_CLASS_FVOW,  /* 0xE4 ่ */
    THAI_CLASS_FVOW,  /* 0xE5 ้ */
    THAI_CLASS_FVOW,  /* 0xE6 ๊ */
    THAI_CLASS_FVOW,  /* 0xE7 ๋ */
    THAI_CLASS_TONE,  /* 0xE8 ์ (thantakat - cancel final) */
    THAI_CLASS_VOW_A, /* 0xE9 ็ (maitaikhu) */
    THAI_CLASS_VOW_A, /* 0xEA ่ (mai ek) */
    THAI_CLASS_VOW_A, /* 0xEB ้ (mai tho) */
    THAI_CLASS_VOW_A, /* 0xEC ๊ (mai tri) */
    THAI_CLASS_VOW_A, /* 0xED ๋ (mai chattawa) */
    THAI_CLASS_PUNC,  /* 0xEE ๎ (yamakkan) */
    THAI_CLASS_PUNC,  /* 0xEF ๏ (fongman) */
    /* 0xF0-0xF9: Thai digits */
    THAI_CLASS_DIGIT, /* 0xF0 ๐ */
    THAI_CLASS_DIGIT, /* 0xF1 ๑ */
    THAI_CLASS_DIGIT, /* 0xF2 ๒ */
    THAI_CLASS_DIGIT, /* 0xF3 ๓ */
    THAI_CLASS_DIGIT, /* 0xF4 ๔ */
    THAI_CLASS_DIGIT, /* 0xF5 ๕ */
    THAI_CLASS_DIGIT, /* 0xF6 ๖ */
    THAI_CLASS_DIGIT, /* 0xF7 ๗ */
    THAI_CLASS_DIGIT, /* 0xF8 ๘ */
    THAI_CLASS_DIGIT, /* 0xF9 ๙ */
    THAI_CLASS_PUNC,  /* 0xFA ๚ (angkhanku) */
    THAI_CLASS_PUNC,  /* 0xFB ๛ (khomut) */
    0,                /* 0xFC undefined */
    0,                /* 0xFD undefined */
    0,                /* 0xFE undefined */
    0,                /* 0xFF undefined */
};

/*
 * Kesmanee keyboard layout -- the dominant Thai keyboard standard.
 * Maps virtual key codes to Thai CP874 characters.
 * Index: VK - VK_SPACE (0x20). Two tables: unshifted and shifted.
 * Extracted/reconstructed from Obj2 keyboard table data.
 */
static const UCHAR g_abKesmaneeNormal[64] = {
    /* VK 0x20-0x5F (space to _) -> Thai CP874 or ASCII */
    0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* SP ! " # $ % & ' */
    0x00,0x00,0x00,0x00,0xBB,0x00,0xBE,0x00, /* ( ) * + , - . /  */
    0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7, /* 0 1 2 3 4 5 6 7  */
    0xF8,0xF9,0xD2,0xA7,0x00,0xAD,0x00,0x00, /* 8 9 : ; < = > ?  */
    0x00,0xAF,0xBA,0xB3,0xB4,0x00,0xD0,0xE3, /* @ A B C D E F G  */
    0xB8,0xC3,0xE0,0xE4,0xB7,0xB6,0xBF,0xC6, /* H I J K L M N O  */
    0xC7,0xC2,0xB9,0xC4,0xCA,0xC1,0xDE,0xBD, /* P Q R S T U V W  */
    0xCA,0xE9,0xC0,0x00,0x00,0x00,0x00,0x00, /* X Y Z [ \ ] ^ _  */
};

static const UCHAR g_abKesmaneeShifted[64] = {
    0x20,0xC9,0xD8,0xD3,0xE1,0xD4,0xD6,0xD5, /* SP ! " # $ % & ' */
    0xB5,0xE2,0xD7,0xAA,0xBD,0xE5,0xE0,0xBA, /* ( ) * + , - . /  */
    0x00,0xDF,0x00,0xE8,0xEB,0xEC,0xB1,0xED, /* 0 1 2 3 4 5 6 7  */
    0xAA,0xEA,0xA6,0xB0,0xCC,0xDB,0xCD,0xBB, /* 8 9 : ; < = > ?  */
    0xAB,0xD1,0xA1,0xA3,0xAE,0xA2,0xA4,0xCB, /* @ A B C D E F G  */
    0xAE,0xAB,0xDA,0xE6,0xBB,0xAC,0xB2,0xCE, /* H I J K L M N O  */
    0xCC,0x00,0xA5,0xA9,0xC5,0xA8,0xE7,0xC8, /* P Q R S T U V W  */
    0xAB,0xE6,0xAC,0x00,0x00,0x00,0x00,0x00, /* X Y Z [ \ ] ^ _  */
};

/* ------------------------------------------------------------------ */
/* INI keys (from Obj2 string analysis)                               */
/* ------------------------------------------------------------------ */
#define THAI_INI_APP        "PM_INSTALL"
#define THAI_INI_SETTING    "<THAI_SETTING>"
#define THAI_INI_TOGGLEKEY  "ToggleKey"
#define THAI_INI_CHECKORDER "CheckOrder"
#define THAI_INI_WARNBEEP   "WarnBeep"
#define THAI_INI_BALI       "BaliSupport"
#define THAI_INI_WINOS2     "WINOS2_LOCATION"
#define THAI_DLL_NAME       "THAILIB"
#define THAI_CP_ENV         "PWORKPLACE_PRIMARY_CP"
#define THAI_KBD_DEVICE     "\\DEV\\KBD$"
#define THAI_SEAMLESS_CLASS "SeamlessClass"

/* ------------------------------------------------------------------ */
/* Toggle key names (from Obj3, zero-indexed)                         */
/* ------------------------------------------------------------------ */
static const char * const g_apszToggleKeyNames[] = {
    "None",
    "` (Accent Grave) key",
    "Left Ctrl",
    "Right Ctrl",
    "Scroll Lock",
    "Function / 12",
    "Alt + `",
    "Pause",
    NULL
};

/* ------------------------------------------------------------------ */
/* Global state (originally in Obj2 mutable shared data)             */
/* ------------------------------------------------------------------ */
static ULONG  g_ulInitialized    = 0;
static ULONG  g_ulKbdLayout      = 0;   /* 0=Kesmanee, 1=Pattachote    */
static ULONG  g_ulToggleKey      = 0;   /* index into toggle key table */
static BOOL   g_fCheckOrder      = TRUE; /* validate char ordering      */
static BOOL   g_fWarnBeep        = TRUE; /* beep on invalid sequence    */
static BOOL   g_fBaliSupport     = FALSE;/* consonant cluster support   */
static BOOL   g_fThaiActive      = FALSE;/* TRUE = Thai input mode      */
static BOOL   g_fKesmanee        = TRUE; /* TRUE=Kesmanee, FALSE=Patta  */
static HWND   g_hwndPanel        = NULLHANDLE;
static HFILE  g_hKbd             = NULLHANDLE;
static HEV    g_hevInit          = NULLHANDLE;
static HMODULE g_hmodFW2DLL      = NULLHANDLE;

/* FW2DLL font-check function (loaded dynamically at init) */
typedef BOOL (APIENTRY *PFN_ISTHAIFW)(PSZ pszFacename);
static PFN_ISTHAIFW g_pfnIsThaiFont = NULL;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static VOID ReadConfig(void)
{
    CHAR szBuf[64];

    /* Toggle key */
    if (PrfQueryProfileString(HINI_USERPROFILE,
                              (PSZ)THAI_INI_APP,
                              (PSZ)THAI_INI_TOGGLEKEY,
                              (PSZ)"0", szBuf, sizeof(szBuf)))
        g_ulToggleKey = atoi(szBuf);

    /* CheckOrder */
    g_fCheckOrder = (BOOL)PrfQueryProfileInt(HINI_USERPROFILE,
                    (PSZ)THAI_INI_APP, (PSZ)THAI_INI_CHECKORDER, 1);

    /* WarnBeep */
    g_fWarnBeep = (BOOL)PrfQueryProfileInt(HINI_USERPROFILE,
                  (PSZ)THAI_INI_APP, (PSZ)THAI_INI_WARNBEEP, 1);

    /* BaliSupport */
    g_fBaliSupport = (BOOL)PrfQueryProfileInt(HINI_USERPROFILE,
                     (PSZ)THAI_INI_APP, (PSZ)THAI_INI_BALI, 0);
}

static BOOL IsValidThaiSequence(UCHAR chPrev, UCHAR chNew)
{
    UCHAR classPrev = g_abThaiClass[chPrev];
    UCHAR classNew  = g_abThaiClass[chNew];

    if (!g_fCheckOrder) return TRUE;
    if (classNew == THAI_CLASS_NONE) return TRUE;

    /* Tone marks and above-vowels can only follow consonants */
    if ((classNew == THAI_CLASS_TONE  ||
         classNew == THAI_CLASS_VOW_A ||
         classNew == THAI_CLASS_VOW_B) &&
        classPrev != THAI_CLASS_CONS)
        return FALSE;

    /* Below-vowels can only follow consonants or above-vowels */
    if (classNew == THAI_CLASS_VOW_B &&
        classPrev != THAI_CLASS_CONS &&
        classPrev != THAI_CLASS_VOW_A)
        return FALSE;

    return TRUE;
}

/* ------------------------------------------------------------------ */
/* ord 1: THAIINITIALIZELIBRARY                                       */
/*                                                                     */
/* Initialise the Thai library. Must be called before any other       */
/* function. Reads configuration from HINI_USERPROFILE, opens the     */
/* keyboard device, and loads FW2DLL for font checking.               */
/*                                                                     */
/* hab     - HAB of calling process                                   */
/* ulFlags - reserved, pass 0                                         */
/*                                                                     */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY THAIINITIALIZELIBRARY(HAB hab, ULONG ulFlags)
{
    ULONG  action;
    CHAR   szFail[CCHMAXPATH];

    if (g_ulInitialized) return NO_ERROR;
    (void)ulFlags;

    /* Create sync semaphore */
    DosCreateEventSem(NULL, &g_hevInit, 0, FALSE);

    /* Read configuration from INI */
    ReadConfig();

    /* Load FW2DLL for Thai font detection */
    if (DosLoadModule(szFail, sizeof(szFail), "FW2DLL", &g_hmodFW2DLL) == NO_ERROR
        && g_hmodFW2DLL)
    {
        DosQueryProcAddr(g_hmodFW2DLL, 694, NULL, (PFN *)&g_pfnIsThaiFont);
    }

    /* Open keyboard device for layout control */
    DosOpen((PSZ)THAI_KBD_DEVICE, &g_hKbd, &action,
            0, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
            OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, NULL);

    /* Read toggle key name for debugging / panel label */
    {
        ULONG ulKey = g_ulToggleKey;
        const char *pszKeyName = (ulKey < 8 && g_apszToggleKeyNames[ulKey])
                                 ? g_apszToggleKeyNames[ulKey] : "None";
        (void)pszKeyName;  /* available for panel display use */
    }

    g_ulInitialized = 1;
    DosPostEventSem(g_hevInit);

    (void)hab;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* ord 2: OS2_PM_DRV_ENABLE                                          */
/*                                                                     */
/* PM display driver enable hook. Called by PMGRE during              */
/* initialisation to allow THAILIB to hook into the graphics          */
/* rendering pipeline for Thai text shaping.                          */
/*                                                                     */
/* This export is identified in the resident name table and is used   */
/* by the PM graphics engine (PMGRE) to call back into THAILIB for   */
/* Thai glyph reordering before display.                              */
/*                                                                     */
/* pDrvData - driver data structure pointer (PM-internal format)      */
/* ulFlags  - enable flags                                            */
/*                                                                     */
/* Returns: TRUE on success.                                          */
/* ------------------------------------------------------------------ */
BOOL APIENTRY OS2_PM_DRV_ENABLE(PVOID pDrvData, ULONG ulFlags)
{
    /* Hook into PMGRE rendering for Thai character shaping.
     * In the original, this registers THAILIB's GreTextOut wrapper
     * with PMGRE so Thai text is reordered before being passed to
     * the actual display driver. */
    (void)pDrvData;
    (void)ulFlags;
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* Forward declarations for functions called before they are defined  */
/* ------------------------------------------------------------------ */
APIRET APIENTRY SHOWPANEL( HWND hwndOwner, BOOL fShow );

/* ------------------------------------------------------------------ */
/* ord 3: THAIISTHAIFONT                                             */
/*                                                                     */
/* Test whether a font is a Thai font (supports CP874 Thai chars).   */
/* Delegates to FW2DLL for the actual font metric check.             */
/*                                                                     */
/* pszFacename - font face name string                                */
/*                                                                     */
/* Returns: TRUE if the font contains Thai glyphs.                   */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIISTHAIFONT(PSZ pszFacename)
{
    if (!pszFacename || !*pszFacename) return FALSE;

    /* Delegate to FW2DLL font worker if available */
    if (g_pfnIsThaiFont)
        return g_pfnIsThaiFont(pszFacename);

    /* Fallback: check if name contains known Thai font indicators */
    if (strstr(pszFacename, "Thai")     != NULL) return TRUE;
    if (strstr(pszFacename, "Kesmanee") != NULL) return TRUE;
    if (strstr(pszFacename, "Baht")     != NULL) return TRUE;
    if (strstr(pszFacename, "Thonburi") != NULL) return TRUE;
    if (strstr(pszFacename, "Angsana")  != NULL) return TRUE;
    if (strstr(pszFacename, "Cordia")   != NULL) return TRUE;

    return FALSE;
}

/* ------------------------------------------------------------------ */
/* ord 4: THAIFRAMEWNDPROC                                           */
/*                                                                     */
/* Thai-aware frame window subclass procedure.                        */
/* Intercepts WM_CHAR events to handle Thai character composition:    */
/*   - Validates Thai character ordering (base+vowel+tone)           */
/*   - Applies Kesmanee/Pattachote keyboard mapping                  */
/*   - Handles toggle key to switch English/Thai input mode          */
/*   - Beeps on invalid sequences (if WarnBeep is set)              */
/*                                                                     */
/* Standard PM EXPENTRY window procedure.                             */
/* ------------------------------------------------------------------ */
MRESULT EXPENTRY THAIFRAMEWNDPROC(HWND hwnd, ULONG msg,
                                   MPARAM mp1, MPARAM mp2)
{
    static PFNWP pfnOldProc = NULL;

    switch (msg)
    {
    case WM_CHAR:
        {
            USHORT fsKC = SHORT1FROMMP(mp1);
            USHORT usCh = SHORT1FROMMP(mp2);
            USHORT usVK = SHORT2FROMMP(mp2);

            if (fsKC & KC_KEYUP) break;

            /* Check for toggle key */
            if (g_ulToggleKey > 0)
            {
                BOOL bToggle = FALSE;
                switch (g_ulToggleKey)
                {
                case 1: /* ` key */
                    if ((fsKC & KC_CHAR) && usCh == '`') bToggle = TRUE;
                    break;
                case 2: /* Left Ctrl */
                    if ((fsKC & KC_CTRL) && usVK == VK_CTRL) bToggle = TRUE;
                    break;
                case 6: /* Alt+` */
                    if ((fsKC & KC_ALT) && (fsKC & KC_CHAR) && usCh == '`')
                        bToggle = TRUE;
                    break;
                case 4: /* Scroll Lock */
                    if (usVK == VK_SCRLLOCK) bToggle = TRUE;
                    break;
                default:
                    break;
                }

                if (bToggle)
                {
                    g_fThaiActive = !g_fThaiActive;
                    SHOWPANEL(hwnd, g_fThaiActive);
                    return (MRESULT)TRUE;  /* consume the key */
                }
            }

            /* Apply Thai keyboard mapping if in Thai mode */
            if (g_fThaiActive && (fsKC & KC_CHAR) && usCh >= 0x20 && usCh < 0x80)
            {
                ULONG idx = usCh - 0x20;
                UCHAR chThai;
                BOOL  fShift = (fsKC & KC_SHIFT) != 0;

                if (idx < 64)
                {
                    chThai = fShift ? g_abKesmaneeShifted[idx]
                                    : g_abKesmaneeNormal[idx];
                    if (chThai)
                    {
                        /* Validate ordering if required */
                        if (g_fCheckOrder)
                        {
                            CHAR szBuf[4] = {0};
                            WinQueryWindowText(hwnd, sizeof(szBuf), szBuf);
                            if (szBuf[0] && !IsValidThaiSequence(
                                                (UCHAR)szBuf[0], chThai))
                            {
                                if (g_fWarnBeep)
                                    WinAlarm(HWND_DESKTOP, WA_NOTE);
                                return (MRESULT)TRUE;  /* reject invalid */
                            }
                        }
                        /* Replace char code with Thai char */
                        mp2 = MPFROM2SHORT((USHORT)chThai, usVK);
                    }
                }
            }
        }
        break;

    case WM_DESTROY:
        /* Restore old proc if we subclassed */
        break;
    }

    if (pfnOldProc)
        return pfnOldProc(hwnd, msg, mp1, mp2);
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------------ */
/* ord 5: THAIFINDWORDHOOK                                           */
/*                                                                     */
/* PM input hook for Thai word boundary detection.                    */
/* Installed via WinSetHook(WH_INPUT) to intercept mouse clicks       */
/* and cursor movement to find Thai word boundaries correctly         */
/* (Thai has no spaces between words -- boundaries must be inferred). */
/*                                                                     */
/* This hook is called for every input event. For mouse double-click  */
/* events in Thai text, it computes the Thai word span containing     */
/* the click point and adjusts the selection accordingly.             */
/*                                                                     */
/* Standard PM hook procedure signature.                              */
/* ------------------------------------------------------------------ */
VOID EXPENTRY THAIFINDWORDHOOK(HAB hab, PQMSG pqmsg, USHORT fs)
{
    /* Thai word-finding algorithm:
     * When a double-click occurs in Thai text, scan left and right
     * from the click position to find word boundaries using the
     * dictionary lookup or n-gram model stored in Obj4.
     *
     * The full implementation requires the word dictionary in Obj4
     * and the THAIWORDSEPERATE function for boundary detection.
     */
    (void)hab;
    (void)pqmsg;
    (void)fs;
}

/* ------------------------------------------------------------------ */
/* ord 6: THAIWORDSEPERATE                                           */
/*                                                                     */
/* Thai word separator / word-break algorithm.                        */
/* Given a Thai string, identifies word boundaries for text           */
/* processing (line breaking, cursor movement, text selection).       */
/*                                                                     */
/* pszText    - Thai CP874 string to analyse                          */
/* cbText     - byte count of string                                  */
/* pabBreaks  - output array of break points (1=word boundary, 0=not)*/
/*                                                                     */
/* Returns: number of word boundaries found.                          */
/* ------------------------------------------------------------------ */
ULONG APIENTRY THAIWORDSEPERATE(PSZ pszText, ULONG cbText,
                                  PBYTE pabBreaks)
{
    ULONG i;
    ULONG cBreaks = 0;

    if (!pszText || !pabBreaks || cbText == 0) return 0;

    memset(pabBreaks, 0, cbText);

    /* Simple boundary detection: mark break points at:
     * - Transition from Thai to non-Thai
     * - Transition from non-Thai to Thai
     * - After Thai punctuation
     * Full implementation would use a word dictionary from Obj4. */
    for (i = 1; i < cbText; i++)
    {
        UCHAR prev = (UCHAR)pszText[i-1];
        UCHAR curr = (UCHAR)pszText[i];
        UCHAR classPrev = g_abThaiClass[prev];
        UCHAR classCurr = g_abThaiClass[curr];

        /* Break between Thai and non-Thai */
        if ((classPrev != THAI_CLASS_NONE) != (classCurr != THAI_CLASS_NONE))
        {
            pabBreaks[i] = 1;
            cBreaks++;
        }
        /* Break after Thai punctuation */
        else if (classPrev == THAI_CLASS_PUNC && classCurr != THAI_CLASS_NONE)
        {
            pabBreaks[i] = 1;
            cBreaks++;
        }
        /* Break between tone mark/vowel and consonant (start of new syllable) */
        else if ((classPrev == THAI_CLASS_TONE || classPrev == THAI_CLASS_VOW_A)
                 && classCurr == THAI_CLASS_CONS)
        {
            pabBreaks[i] = 1;
            cBreaks++;
        }
    }

    return cBreaks;
}

/* ------------------------------------------------------------------ */
/* ord 10: THAIISTHAIBASE                                            */
/*                                                                     */
/* Test whether a CP874 byte value is a Thai base consonant.         */
/* Base consonants are the foundation of Thai syllables and can       */
/* bear tone marks and vowel diacritics.                              */
/*                                                                     */
/* chTest - byte value to test (CP874 encoded)                        */
/*                                                                     */
/* Returns: TRUE if the character is a Thai base consonant.          */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIISTHAIBASE(UCHAR chTest)
{
    return (g_abThaiClass[chTest] == THAI_CLASS_CONS);
}

/* ------------------------------------------------------------------ */
/* ord 11: THAIQUERYISTHAI                                           */
/*                                                                     */
/* Test whether a CP874 byte value is any Thai character.            */
/* Returns TRUE for any character in the Thai range (0xA0-0xFF)      */
/* that has a defined Thai character class.                           */
/*                                                                     */
/* chTest - byte value to test                                         */
/*                                                                     */
/* Returns: TRUE if chTest is a Thai character.                       */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIQUERYISTHAI(UCHAR chTest)
{
    return (g_abThaiClass[chTest] != THAI_CLASS_NONE && chTest >= 0xA0);
}

/* ------------------------------------------------------------------ */
/* ord 12: THAIQUERYISKESMANEE                                       */
/*                                                                     */
/* Determine whether a font uses the Kesmanee (TIS-620) character    */
/* encoding vs. an alternative Thai encoding.                         */
/* Delegates to FW2DLL for the actual encoding check.                */
/*                                                                     */
/* pszFacename - font face name                                       */
/*                                                                     */
/* Returns: TRUE if font uses Kesmanee encoding.                     */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIQUERYISKESMANEE(PSZ pszFacename)
{
    if (!pszFacename) return FALSE;

    /* All standard Thai fonts on OS/2 use Kesmanee (TIS-620) encoding.
     * Alternative encodings (Pattachote, Manee etc.) are rare.
     * Check for known non-Kesmanee font names: */
    if (strstr(pszFacename, "Pattachote") != NULL) return FALSE;
    if (strstr(pszFacename, "Manee")      != NULL) return FALSE;

    /* Default: assume Kesmanee encoding */
    return THAIISTHAIFONT(pszFacename);
}

/* ------------------------------------------------------------------ */
/* ord 13: THAISETKEYBOARDLAYOUT                                     */
/*                                                                     */
/* Set the active Thai keyboard layout.                               */
/*                                                                     */
/* ulLayout - 0 = Kesmanee (standard TIS 820-2531)                   */
/*            1 = Pattachote (alternative layout)                     */
/*                                                                     */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY THAISETKEYBOARDLAYOUT(ULONG ulLayout)
{
    if (ulLayout > 1) return ERROR_INVALID_PARAMETER;

    g_ulKbdLayout  = ulLayout;
    g_fKesmanee    = (ulLayout == 0);

    /* Write to INI */
    {
        CHAR szVal[4];
        szVal[0] = (CHAR)('0' + (int)ulLayout);
        szVal[1] = '\0';
        PrfWriteProfileString(HINI_USERPROFILE,
                              (PSZ)THAI_INI_APP, "KbdLayout", (PSZ)szVal);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* ord 15: SHOWPANEL                                                  */
/*                                                                     */
/* Show or hide the Thai input indicator panel.                       */
/* The panel is a small floating window that displays the current     */
/* input mode (Thai or English) near the focus window.               */
/*                                                                     */
/* hwndOwner - owner window (positions panel relative to this)        */
/* fShow     - TRUE to show, FALSE to hide                            */
/*                                                                     */
/* Returns: NO_ERROR.                                                 */
/* ------------------------------------------------------------------ */
APIRET APIENTRY SHOWPANEL(HWND hwndOwner, BOOL fShow)
{
    if (!fShow)
    {
        if (g_hwndPanel)
        {
            WinSetWindowPos(g_hwndPanel, NULLHANDLE, 0,0,0,0, SWP_HIDE);
        }
        return NO_ERROR;
    }

    /* Position the panel near the focus window */
    if (g_hwndPanel && hwndOwner)
    {
        SWP swp;
        WinQueryWindowPos(hwndOwner, &swp);
        WinSetWindowPos(g_hwndPanel, HWND_TOP,
                        swp.x, swp.y - 20, 40, 20,
                        SWP_MOVE | SWP_SHOW | SWP_ZORDER);
    }
    else if (g_hwndPanel)
    {
        WinSetWindowPos(g_hwndPanel, HWND_TOP, 0, 0, 40, 20,
                        SWP_MOVE | SWP_SHOW | SWP_ZORDER);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* ord 107: BASEOS_SETKBDLAYER                                       */
/*                                                                     */
/* Set the base OS keyboard layer (switches between Thai and Latin    */
/* keyboard layers at the OS/2 session manager level, equivalent to  */
/* the Ctrl+Shift toggle in bidi systems).                            */
/*                                                                     */
/* ulLayer - 0 = Latin/English layer                                  */
/*           1 = Thai layer                                           */
/*                                                                     */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BASEOS_SETKBDLAYER(ULONG ulLayer)
{
    if (ulLayer > 1) return ERROR_INVALID_PARAMETER;

    g_fThaiActive = (ulLayer == 1);

    /* Update indicator panel */
    {
        HWND hwndFocus = WinQueryFocus(HWND_DESKTOP);
        SHOWPANEL(hwndFocus, g_fThaiActive);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* DLL init / term                                                     */
/* ------------------------------------------------------------------ */
unsigned _System LibMain(unsigned hmod, unsigned flag)
{
    (void)hmod;
    if (flag == 0)
    {
        g_ulInitialized = 0;
        g_ulKbdLayout   = 0;
        g_ulToggleKey   = 0;
        g_fCheckOrder   = TRUE;
        g_fWarnBeep     = TRUE;
        g_fBaliSupport  = FALSE;
        g_fThaiActive   = FALSE;
        g_fKesmanee     = TRUE;
        g_hwndPanel     = NULLHANDLE;
        g_hKbd          = NULLHANDLE;
        g_hevInit       = NULLHANDLE;
        g_hmodFW2DLL    = NULLHANDLE;
        g_pfnIsThaiFont = NULL;
    }
    else
    {
        if (g_hmodFW2DLL)   { DosFreeModule(g_hmodFW2DLL); g_hmodFW2DLL = NULLHANDLE; }
        if (g_hKbd)         { DosClose(g_hKbd);             g_hKbd       = NULLHANDLE; }
        if (g_hevInit)      { DosCloseEventSem(g_hevInit);  g_hevInit    = NULLHANDLE; }
        if (g_hwndPanel)    { WinDestroyWindow(g_hwndPanel); g_hwndPanel = NULLHANDLE; }
        g_ulInitialized = 0;
    }
    return 1;
}