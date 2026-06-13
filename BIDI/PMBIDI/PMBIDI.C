/*
 * PMBIDI.C  --  OS/2 PM Bidirectional Language APIs DLL
 *
 * Replacement for PMBIDI.DLL  (IBM, v14.081, LX format, 13611 bytes)
 * Description: "OS/2 PM Bidirectional Language APIs."
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -6s -fpi87 -sg -otexanr -wx -fo=PMBIDI.OBJ PMBIDI.C
 *   wlink @PMBIDI.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Object layout of the original DLL:
 *   Obj1  32-bit CODE  0x20cc bytes  -- Main dispatch & function bodies
 *   Obj2  16-bit CODE  0x0088 bytes  -- 17 x 16->32 far-jump thunks
 *   Obj3  16-bit CODE  0x0008 bytes  -- Single thunk for PMBIDI_Initialize
 *   Obj4  16-bit CODE  0x0019 bytes  -- Generic 16->32 parameter dispatcher
 *   Obj5  32-bit DATA  0x0a10 bytes  -- EBCDIC<->ASCII translation tables
 *   Obj6  32-bit SHARED DATA  0x0430 -- Per-process LANGINFO/BidiAttr state
 *   Obj7  32-bit SHARED DATA  0x0004 -- Global init flag (DWORD)
 *   Obj8  16-bit SHARED DATA  0x0008 -- 16-bit accessible shared state
 *   Obj9  32-bit SHARED DATA  0x0340 -- layout_object keyword/attribute table
 *
 * Entry table (actual entries found):
 *   ord   1: PMBIDI_Initialize  (32-bit, Obj3->Obj1+0x1378)
 *   ords 18-19: WIN16SET/QUERYPROCESSLANGINFO (16-near, Obj2 thunks)
 *   ords 20-23, 108-109: more 16-near stubs
 *   All other ordinals: exported by name only, NO entry table entry
 *
 * Imports used by Obj1:
 *   BDCALL32  -- Main bidi engine (all real work delegated here)
 *   DOSCALLS  ord#425 (DosAllocMem), ord#426 (DosFreeMem)
 *   PMGPI     ord#588 (GpiQueryCharBox), ord#440
 *   PMWIN, PMMERGE
 *
 * Design: PMBIDI.DLL is a DISPATCH LAYER over BDCALL32.DLL.
 * The 32-bit functions (Win32*, Nls32*, Gpi32*, Bidi_*, layout_object_*)
 * validate parameters, set up per-process state, then forward to BDCALL32.
 * The 16-bit functions (WIN16*, NLS16*, GPI16*, BIDI_16*) are 16->32 thunks
 * that call the corresponding 32-bit function via a far-jump stub.
 *
 * This replacement implements the complete export surface.  The actual
 * bidi algorithm work is still delegated to BDCALL32 through DosLoadModule /
 * DosQueryProcAddr at init time, exactly as the original does.
 * ============================================================
 */

/*
 * Pull in every OS/2 subsystem we actually use.
 *
 * INCL_DOSMODULEMGR  -- DosLoadModule, DosFreeModule, DosQueryProcAddr
 * INCL_DOSNLS        -- DosQueryCp
 * INCL_DOSMEMMGR     -- DosAllocMem, DosFreeMem
 * INCL_DOSPROCESS    -- DosBeep, DosExit
 * INCL_DOSSEMAPHORES -- (kept for completeness)
 * INCL_WIN           -- all Win* functions + HINI_USERPROFILE + PrfQuery*
 * INCL_WINPRF        -- PrfQueryProfileData (separate from INCL_WIN on some headers)
 * INCL_GPI           -- HPS, GpiSetBidiAttr / GpiQueryBidiAttr
 * INCL_GPIBIDI       -- GpiSetBidiAttr / GpiQueryBidiAttr prototypes
 * INCL_ERRORS        -- ERROR_INVALID_PARAMETER, ERROR_BUFFER_OVERFLOW, etc.
 */
#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSNLS
#define INCL_DOSSEMAPHORES
#define INCL_WIN
#define INCL_WINPRF
#define INCL_GPI
#define INCL_GPIBIDI
#define INCL_ERRORS
#include <os2.h>
#include <string.h>
#include <stdlib.h>

/*
 * ERROR_NOT_SUPPORTED (50) is in bseerr.h pulled by INCL_ERRORS.
 * Guard against headers that may omit it.
 */
#ifndef ERROR_NOT_SUPPORTED
#define ERROR_NOT_SUPPORTED  50
#endif

/* ------------------------------------------------------------------ */
/* Public type definitions                                             */
/* ------------------------------------------------------------------ */

/* Language/keyboard/viewer information structure (per-process) */
typedef struct _LANGINFO {
    ULONG   cbSize;             /* sizeof(LANGINFO)                 */
    ULONG   flReserved;         /* must be 0                        */
    ULONG   idLang;             /* language identifier              */
    ULONG   idKbdLayer;         /* keyboard layer ID                */
    ULONG   idLangViewer;       /* language viewer ID               */
    ULONG   flBidiAttr;         /* bidi attribute flags             */
} LANGINFO;
typedef LANGINFO *PLANGINFO;

/* Per-process bidi attribute flags stored in flBidiAttr / ulProcess_BidiAttr */
#define BIDI_ATTR_TEXTTYPE_VISUAL   0x00000001
#define BIDI_ATTR_ORIENT_RTL        0x00000002
#define BIDI_ATTR_NUMERALS_PASSTHRU 0x00000004
#define BIDI_ATTR_SYM_SWAP_OFF      0x00000008
#define BIDI_ATTR_WORD_BREAK_ON     0x00000010
#define BIDI_ATTR_DISPLAY_SHAPED    0x00000020
#define BIDI_ATTR_SCREEN_FIELD_REV  0x00000040
#define BIDI_ATTR_STATUS_INDICATOR  0x00000080
#define BIDI_ATTR_HKFLAG_ENG_LAYER  0x00000100
#define BIDI_ATTR_INPUT_PROCESSING  0x00000200

/* NLS bidi string conversion flags */
#define NLS_BIDI_VISUAL             0x0001
#define NLS_BIDI_IMPLICIT           0x0002
#define NLS_BIDI_LTR                0x0004
#define NLS_BIDI_RTL                0x0008
#define NLS_BIDI_ORIENT_CONTEXT     0x0010

/*
 * BIDIATTR -- GPI bidi presentation attribute structure.
 * The OS/2 Toolkit 4.5 headers may or may not define this and the
 * GpiSetBidiAttr / GpiQueryBidiAttr prototypes under INCL_GPIBIDI.
 * We guard against double-definition with the _BIDIATTR_DEFINED macro.
 */
#ifndef _BIDIATTR_DEFINED
#define _BIDIATTR_DEFINED
typedef struct _BIDIATTR {
    ULONG   cbSize;
    ULONG   flBidiAttr;
} BIDIATTR;
typedef BIDIATTR *PBIDIATTR;
#endif

/*
 * GpiSetBidiAttr / GpiQueryBidiAttr prototypes.
 * These are in PMGPI at ordinals 440 and 441.
 *
 * The OS/2 Toolkit 4.5 headers declare them when INCL_GPIBIDI is defined.
 * The Open Watcom headers (as shipped with ArcaOS) may or may not include
 * them.  We use a two-level guard:
 *   1. If INCL_GPIBIDI caused pmgpi.h to define GPIBIDI_INCLUDED, we skip.
 *   2. Otherwise we declare them ourselves.
 * This avoids both "no prototype" (W131) and "redeclared" errors.
 */
#if !defined( GPIBIDI_INCLUDED ) && !defined( GPI_BIDI_PROTOTYPES_DEFINED )
#define GPI_BIDI_PROTOTYPES_DEFINED
BOOL APIENTRY GpiSetBidiAttr( HPS hps, PBIDIATTR pBidiAttr );
BOOL APIENTRY GpiQueryBidiAttr( HPS hps, PBIDIATTR pBidiAttr );
#endif

/* layout_object handle (opaque pointer) */
typedef PVOID LAYOUT_OBJECT;
typedef LAYOUT_OBJECT *PLAYOUT_OBJECT;

/* layout_object attribute value pair */
typedef struct _LayoutValueRec {
    int    name;
    PVOID  value;
} LayoutValueRec;
typedef LayoutValueRec *LayoutValues;

/* Layout attribute name constants (from Obj9 keyword table) */
#define LAYOUT_TEXTTYPE_VISUAL      0
#define LAYOUT_TEXTTYPE_IMPLICIT    1
#define LAYOUT_ORIENTATION_LTR      2
#define LAYOUT_ORIENTATION_RTL      3
#define LAYOUT_ORIENTATION_CONTEXT  4
#define LAYOUT_NUMERALS_NOMINAL     5
#define LAYOUT_NUMERALS_PASSTHRU    6
#define LAYOUT_SYM_SWAP_OFF         7
#define LAYOUT_WORD_BREAK_ON        8
#define LAYOUT_DISPLAY_SHAPED       9

/* ------------------------------------------------------------------ */
/* EBCDIC <-> ASCII translation tables (extracted from Obj5, offsets   */
/* 0x000 and 0x100 respectively, 256 bytes each)                       */
/* ------------------------------------------------------------------ */
static const UCHAR ebcdic2ascii[256] = {
    0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, 0x16, 0x05, 0x25, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f, 0x40,
    0x4f, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, 0xf0,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, 0xfc,
    0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0x4a, 0xe0, 0x5a, 0x5f, 0x6d,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x6a, 0xd0, 0xa1, 0x04,
    0x07, 0x00, 0x06, 0x40, 0xfc, 0x71, 0x40, 0xdd, 0xef, 0xdf, 0xce, 0xde, 0x72, 0xcf, 0x73, 0x75,
    0x68, 0x76, 0x40, 0x40, 0x77, 0x69, 0xfb, 0x78, 0xea, 0xfa, 0xb1, 0xb0, 0xb2, 0xb3, 0xb5, 0xb4,
    0xcc, 0xb6, 0xb7, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0xdb, 0x48, 0x49, 0xee, 0xfe, 0x2b,
    0x2c, 0x09, 0x21, 0x28, 0x51, 0x52, 0x53, 0x54, 0x38, 0x31, 0x34, 0x33, 0x55, 0x56, 0x24, 0x22,
    0x17, 0x29, 0x06, 0x20, 0xfc, 0x2a, 0x57, 0x58, 0x1a, 0x35, 0x08, 0x39, 0x36, 0x30, 0x3a, 0x59,
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x8a, 0x8b, 0x8c, 0x23, 0x15, 0x14, 0x04, 0x8d, 0x8e, 0x3b,
    0x8f, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xba, 0xbb, 0xa0,
    0xca, 0xda, 0xbc, 0xbd, 0xbe, 0xeb, 0xbf, 0x80, 0x90, 0x70, 0xcb, 0xb8, 0xcd, 0xb9, 0x3e, 0x74,
};

static const UCHAR ascii2ebcdic[256] = {
    0x00, 0x01, 0x02, 0x03, 0x2c, 0xdc, 0x09, 0xc3, 0x7f, 0xca, 0xb2, 0x20, 0x0b, 0x0c, 0x0d, 0x0e,
    0x21, 0x80, 0xc0, 0xdb, 0xda, 0x08, 0xc1, 0x18, 0x19, 0xc8, 0x20, 0x1c, 0x1d, 0x1e, 0x1f, 0xc4,
    0xb3, 0xc0, 0xd9, 0xbf, 0x0a, 0x17, 0x1b, 0xb4, 0xc2, 0xc5, 0xb0, 0xb1, 0x05, 0x06, 0x07, 0xcd,
    0xba, 0x16, 0xbc, 0xbb, 0xc9, 0xcc, 0x04, 0xb9, 0xcb, 0xce, 0xdf, 0x14, 0x15, 0xfe, 0x1a, 0x20,
    0xa4, 0xa5, 0xa6, 0x01, 0x66, 0xfc, 0xaa, 0xac, 0xad, 0x5b, 0x2e, 0x3c, 0x28, 0x2b, 0x21, 0x26,
    0xb5, 0xb6, 0xb7, 0xb8, 0xbd, 0xbe, 0xc6, 0xc7, 0xcf, 0x5d, 0x24, 0x2a, 0x29, 0x3b, 0x5e, 0x2d,
    0x2f, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0x91, 0x96, 0x7c, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, 0xf9,
    0x86, 0x8d, 0x8f, 0xff, 0x90, 0x92, 0x95, 0x98, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, 0xf7,
    0x61, 0x62, 0x63, 0x64, 0x65, 0xfc, 0x66, 0x67, 0x68, 0x69, 0xd6, 0xd7, 0xd8, 0xdd, 0xde, 0xe0,
    0xf8, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xef, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec,
    0x9c, 0x9b, 0x9d, 0x9e, 0xa0, 0x9f, 0xa2, 0xa3, 0xfb, 0xfd, 0xed, 0xee, 0xf2, 0xf3, 0xf4, 0xf6,
    0x7b, 0x41, 0x42, 0x43, 0x44, 0xf8, 0x45, 0x46, 0x47, 0x48, 0x49, 0xf0, 0xfa, 0xa1, 0xfc, 0x8b,
    0x8e, 0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0xf1, 0xab, 0x20, 0x88, 0x8c,
    0x8a, 0x5c, 0x20, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x99, 0xf5, 0x20, 0x20, 0xae,
    0x89, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x9a, 0x97, 0x20, 0x20, 0xaf,
};

/* ------------------------------------------------------------------ */
/* Shared / per-process state (originally in Obj6, Obj7)               */
/* ------------------------------------------------------------------ */

/* Per-process bidi attribute -- exported as ordinal 999 */
ULONG ulProcess_BidiAttr = 0;

/* Per-process LANGINFO (shared data, zeroed at load time) */
static LANGINFO g_LangInfo = { sizeof(LANGINFO), 0, 0, 0, 0, 0 };

/* Global initialised flag (originally Obj7 -- single DWORD) */
static ULONG g_ulInitialized = 0;

/* BDCALL32 module handle loaded at init time */
static HMODULE  g_hmodBDCALL32 = NULLHANDLE;

/*
 * INI application and key names (strings found in Obj6 of original binary).
 * Placed here -- before any function body -- so all functions can reference them.
 */
static const char szBidiApp[]       = "PM_BIDI";   /* INI application name  */
static const char szKeyStdDlgLang[] = "STDDLGLANG";/* standard dialog lang   */
static const char szKeyHelpBidi[]   = "HELPBIDI";  /* help bidi setting      */
/* BIDIATTR_INIKEY is used as a string literal to avoid a separate variable  */
#define BIDIATTR_INIKEY "BIDIATTR"

/*
 * Generic BDCALL32 function pointer type.
 * All delegation to BDCALL32 uses DosQueryProcAddr per call.
 */
typedef APIRET (APIENTRY *PFN_BDCALL)(ULONG, ...);

/*
 * DosQueryModuleHandle prototype guard.
 * Under INCL_DOSMODULEMGR the Open Watcom bsedos.h already declares this.
 * We only need our own declaration if the toolkit header didn't provide it.
 * The macro DOSEXECPGM_DECLARED is a sentinel that bsedos.h sets; if it is
 * defined we know the module-manager section was included and we skip.
 * If neither guard is present we emit our own declaration.
 */
#if !defined( INCL_DOSMODULEMGR ) && !defined( DOSQUERYMODULEHANDLE_DECLARED )
#define DOSQUERYMODULEHANDLE_DECLARED
APIRET APIENTRY DosQueryModuleHandle( PCSZ pszModname, PHMODULE phmod );
#endif

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/*
 * LoadBDCALL32 -- load BDCALL32.DLL and cache its generic entry point.
 * Called once from PMBIDI_Initialize.  On failure the functions below
 * return error codes rather than crashing.
 */
static APIRET LoadBDCALL32( void )
{
    UCHAR szFailName[CCHMAXPATH];
    APIRET arc;

    if ( g_hmodBDCALL32 != NULLHANDLE )
        return NO_ERROR;

    arc = DosLoadModule( szFailName, sizeof(szFailName),
                         "BDCALL32", &g_hmodBDCALL32 );
    if ( arc != NO_ERROR )
    {
        g_hmodBDCALL32 = NULLHANDLE;
        return arc;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Group 1 -- Initialization (ord 1)                                   */
/* ------------------------------------------------------------------ */

/*
 * PMBIDI_Initialize -- ordinal 1
 *
 * Must be called once by every application that uses PMBIDI services
 * before calling any other function.  Loads BDCALL32, initialises
 * per-process state and sets the process bidi attribute from the
 * session defaults stored in HINI_USERPROFILE.
 *
 * Returns: NO_ERROR on success, DOS error code on failure.
 */
APIRET APIENTRY PMBIDI_Initialize( void )
{
    APIRET arc;
    ULONG  ulAttr = 0;
    ULONG  cbAttr = sizeof(ULONG);

    if ( g_ulInitialized )
        return NO_ERROR;

    arc = LoadBDCALL32();
    if ( arc != NO_ERROR )
        return arc;

    /* Initialise per-process LANGINFO to neutral defaults */
    memset( &g_LangInfo, 0, sizeof(g_LangInfo) );
    g_LangInfo.cbSize = sizeof(LANGINFO);

    /* Restore the process bidi attribute from HINI_USERPROFILE if saved.
     * Key: application="PM_BIDI", key="BIDIATTR".
     * If not found, ulProcess_BidiAttr stays 0 (neutral). */
    if ( PrfQueryProfileData( HINI_USERPROFILE,
                              (PSZ)szBidiApp, (PSZ)BIDIATTR_INIKEY,
                              &ulAttr, &cbAttr ) )
    {
        ulProcess_BidiAttr = ulAttr;
    }
    else
    {
        ulProcess_BidiAttr = 0;
    }

    g_ulInitialized = 1;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Group 2 -- LangInfo / KbdLayer / LangViewer (ords 10-15, 18-31)    */
/* ------------------------------------------------------------------ */

/*
 * Win32SetLangInfo -- ordinal 20
 *
 * Set the language information for the current process.
 * pLangInfo->cbSize must equal sizeof(LANGINFO); flReserved must be 0.
 */
APIRET APIENTRY Win32SetLangInfo( PLANGINFO pLangInfo )
{
    if ( !pLangInfo )                          return ERROR_INVALID_PARAMETER;
    if ( pLangInfo->cbSize != sizeof(LANGINFO) ) return ERROR_INVALID_PARAMETER;
    if ( pLangInfo->flReserved != 0 )          return ERROR_INVALID_PARAMETER;

    g_LangInfo = *pLangInfo;
    return NO_ERROR;
}

/*
 * Win32QueryLangInfo -- ordinal 21
 *
 * Query the language information for the current process.
 */
APIRET APIENTRY Win32QueryLangInfo( PLANGINFO pLangInfo )
{
    if ( !pLangInfo )                          return ERROR_INVALID_PARAMETER;
    if ( pLangInfo->cbSize != sizeof(LANGINFO) ) return ERROR_INVALID_PARAMETER;
    if ( pLangInfo->flReserved != 0 )          return ERROR_INVALID_PARAMETER;

    *pLangInfo = g_LangInfo;
    return NO_ERROR;
}

/*
 * Win32SetKbdLayer -- ordinal 22
 *
 * Set the keyboard layer ID for the current process.
 */
APIRET APIENTRY Win32SetKbdLayer( ULONG idKbdLayer )
{
    g_LangInfo.idKbdLayer = idKbdLayer;
    return NO_ERROR;
}

/*
 * Win32QueryKbdLayer -- ordinal 23
 *
 * Query the keyboard layer ID for the current process.
 */
APIRET APIENTRY Win32QueryKbdLayer( PULONG pidKbdLayer )
{
    if ( !pidKbdLayer ) return ERROR_INVALID_PARAMETER;
    *pidKbdLayer = g_LangInfo.idKbdLayer;
    return NO_ERROR;
}

/*
 * Win32SetLangViewer -- ordinal 24
 *
 * Set the language viewer ID for the current process.
 */
APIRET APIENTRY Win32SetLangViewer( ULONG idLangViewer )
{
    g_LangInfo.idLangViewer = idLangViewer;
    return NO_ERROR;
}

/*
 * Win32QueryLangViewer -- ordinal 25
 *
 * Query the language viewer ID for the current process.
 */
APIRET APIENTRY Win32QueryLangViewer( PULONG pidLangViewer )
{
    if ( !pidLangViewer ) return ERROR_INVALID_PARAMETER;
    *pidLangViewer = g_LangInfo.idLangViewer;
    return NO_ERROR;
}

/*
 * Win32SetProcessLangInfo -- ordinal 30
 * Win32QueryProcessLangInfo -- ordinal 31
 *
 * Like Win32Set/QueryLangInfo but the "process" variants operate on the
 * global per-process structure visible to all DLLs in this process.
 * In the original DLL these use the shared-data Obj6 block;
 * here we map them to the same g_LangInfo.
 */
APIRET APIENTRY Win32SetProcessLangInfo( PLANGINFO pLangInfo )
{
    return Win32SetLangInfo( pLangInfo );
}

APIRET APIENTRY Win32QueryProcessLangInfo( PLANGINFO pLangInfo )
{
    return Win32QueryLangInfo( pLangInfo );
}

/* 16-bit wrappers -- ords 10-15, 18, 19 -- forward to 32-bit */
APIRET APIENTRY WIN16SETLANGINFO( PLANGINFO p )         { return Win32SetLangInfo(p); }
APIRET APIENTRY WIN16QUERYLANGINFO( PLANGINFO p )        { return Win32QueryLangInfo(p); }
APIRET APIENTRY WIN16SETKBDLAYER( ULONG id )             { return Win32SetKbdLayer(id); }
APIRET APIENTRY WIN16QUERYKBDLAYER( PULONG p )           { return Win32QueryKbdLayer(p); }
APIRET APIENTRY WIN16SETLANGVIEWER( ULONG id )           { return Win32SetLangViewer(id); }
APIRET APIENTRY WIN16QUERYLANGVIEWER( PULONG p )         { return Win32QueryLangViewer(p); }
APIRET APIENTRY WIN16SETPROCESSLANGINFO( PLANGINFO p )   { return Win32SetProcessLangInfo(p); }
APIRET APIENTRY WIN16QUERYPROCESSLANGINFO( PLANGINFO p ) { return Win32QueryProcessLangInfo(p); }

/* ------------------------------------------------------------------ */
/* Group 3 -- GPI Bidi Attributes (ords 40-41, 50-51)                  */
/* ------------------------------------------------------------------ */

/*
 * Gpi32SetBidiAttr -- ordinal 50
 *
 * Set the bidi presentation attributes on a presentation space.
 * GpiSetBidiAttr is in PMGPI at ordinal 440.  We call it dynamically
 * to avoid a link-time dependency that may not be in older stub libs.
 */
APIRET APIENTRY Gpi32SetBidiAttr( HPS hps, PBIDIATTR pBidiAttr )
{
    static BOOL   s_fTried = FALSE;
    static BOOL   (APIENTRY *s_pfnSet)(HPS, PBIDIATTR) = NULL;

    if ( !pBidiAttr )                              return ERROR_INVALID_PARAMETER;
    if ( pBidiAttr->cbSize != sizeof(BIDIATTR) )   return ERROR_INVALID_PARAMETER;

    /* Lazily resolve GpiSetBidiAttr from PMGPI */
    if ( !s_fTried )
    {
        HMODULE hmod = NULLHANDLE;
        s_fTried = TRUE;
        if ( DosQueryModuleHandle( "PMGPI", &hmod ) == NO_ERROR && hmod )
            DosQueryProcAddr( hmod, 440, NULL, (PFN *)&s_pfnSet );
    }

    if ( s_pfnSet )
    {
        if ( !s_pfnSet( hps, pBidiAttr ) )
            return ERROR_INVALID_FUNCTION;  /* GPI rejected the call */
        return NO_ERROR;
    }

    /* PMGPI not available -- silently succeed (no-op) */
    return NO_ERROR;
}

/*
 * Gpi32QueryBidiAttr -- ordinal 51
 *
 * Query the bidi presentation attributes from a presentation space.
 * GpiQueryBidiAttr is in PMGPI at ordinal 441.
 */
APIRET APIENTRY Gpi32QueryBidiAttr( HPS hps, PBIDIATTR pBidiAttr )
{
    static BOOL   s_fTried = FALSE;
    static BOOL   (APIENTRY *s_pfnQuery)(HPS, PBIDIATTR) = NULL;

    if ( !pBidiAttr )                              return ERROR_INVALID_PARAMETER;
    if ( pBidiAttr->cbSize != sizeof(BIDIATTR) )   return ERROR_INVALID_PARAMETER;

    /* Lazily resolve GpiQueryBidiAttr from PMGPI */
    if ( !s_fTried )
    {
        HMODULE hmod = NULLHANDLE;
        s_fTried = TRUE;
        if ( DosQueryModuleHandle( "PMGPI", &hmod ) == NO_ERROR && hmod )
            DosQueryProcAddr( hmod, 441, NULL, (PFN *)&s_pfnQuery );
    }

    if ( s_pfnQuery )
    {
        if ( !s_pfnQuery( hps, pBidiAttr ) )
            return ERROR_INVALID_FUNCTION;  /* GPI rejected the call */
        return NO_ERROR;
    }

    /* PMGPI not available -- return neutral defaults */
    pBidiAttr->flBidiAttr = 0;
    return NO_ERROR;
}

/* 16-bit wrappers */
APIRET APIENTRY GPI16SETBIDIATTR( HPS hps, PBIDIATTR p )   { return Gpi32SetBidiAttr(hps, p); }
APIRET APIENTRY GPI16QUERYBIDIATTR( HPS hps, PBIDIATTR p ) { return Gpi32QueryBidiAttr(hps, p); }

/* ------------------------------------------------------------------ */
/* Group 4 -- Codepage Query (ords 60-61)                              */
/* ------------------------------------------------------------------ */

/*
 * Bidi_QueryCp -- ordinal 60
 *
 * Returns the active bidi codepage for the current process.
 * BDCALL32 manages the actual codepage table; we query the process CP
 * from OS/2 and return it.
 */
APIRET APIENTRY Bidi_QueryCp( PULONG pulCp )
{
    ULONG  aulCpList[8];
    ULONG  cbCpList = sizeof(aulCpList);
    APIRET arc;

    if ( !pulCp ) return ERROR_INVALID_PARAMETER;

    arc = DosQueryCp( cbCpList, aulCpList, &cbCpList );
    if ( arc != NO_ERROR ) return arc;

    *pulCp = aulCpList[0];
    return NO_ERROR;
}

APIRET APIENTRY BIDI_16QUERYCP( PULONG p ) { return Bidi_QueryCp(p); }

/* ------------------------------------------------------------------ */
/* Group 5 -- Settings (ords 101-102, 110-113)                         */
/* ------------------------------------------------------------------ */

/*
 * PMBIDI_Get_StdDlgLang_Setting -- ordinal 101
 *
 * Retrieve the standard dialog language setting from HINI_USERPROFILE.
 * pulSetting receives the stored ULONG value, or 0 if not set.
 */
APIRET APIENTRY PMBIDI_Get_StdDlgLang_Setting( PULONG pulSetting )
{
    ULONG  ulVal  = 0;
    ULONG  cbData = sizeof(ULONG);

    if ( !pulSetting ) return ERROR_INVALID_PARAMETER;

    PrfQueryProfileData( HINI_USERPROFILE,
                         (PSZ)szBidiApp, (PSZ)szKeyStdDlgLang,
                         &ulVal, &cbData );
    *pulSetting = ulVal;
    return NO_ERROR;
}

APIRET APIENTRY PMBIDI_16GET_STDDLGLANG_SETTING( PULONG p )
{
    return PMBIDI_Get_StdDlgLang_Setting(p);
}

/*
 * PMBIDI_String_To_BinVal -- ordinal 110
 *
 * Convert a NUL-terminated ASCII string representation of a bidi setting
 * value into the corresponding binary ULONG.  pszString is the keyword
 * (e.g. "TEXTTYPE_VISUAL"), pulValue receives the bidi attribute bit.
 */
APIRET APIENTRY PMBIDI_String_To_BinVal( PSZ pszString, PULONG pulValue )
{
    /* Keyword table -- matches Obj9 strings */
    static const struct { const char *pszKey; ULONG ulVal; } aTable[] = {
        { "TEXTTYPE_VISUAL",    BIDI_ATTR_TEXTTYPE_VISUAL   },
        { "IMPLICIT",           0                           },
        { "_ORIENT_LTR",        0                           },
        { "_ORIENT_RTL",        BIDI_ATTR_ORIENT_RTL        },
        { "NUMERALS_NOMINAL",   0                           },
        { "PASSTHRU",           BIDI_ATTR_NUMERALS_PASSTHRU },
        { "SYM_SWAP_OFF",       BIDI_ATTR_SYM_SWAP_OFF      },
        { "WORD_BREAK_ON",      BIDI_ATTR_WORD_BREAK_ON     },
        { "_DISPLAY_SHAPED",    BIDI_ATTR_DISPLAY_SHAPED    },
        { "SCREEN_FIELD_REV",   BIDI_ATTR_SCREEN_FIELD_REV  },
        { "STATUS_INDICATOR",   BIDI_ATTR_STATUS_INDICATOR  },
        { "HKFLAG_ENG_LAYER",   BIDI_ATTR_HKFLAG_ENG_LAYER  },
        { "INPUT_PROCESSING",   BIDI_ATTR_INPUT_PROCESSING  },
        { NULL, 0 }
    };
    int i;

    if ( !pszString || !pulValue ) return ERROR_INVALID_PARAMETER;

    for ( i = 0; aTable[i].pszKey; i++ )
    {
        if ( strcmp( pszString, aTable[i].pszKey ) == 0 )
        {
            *pulValue = aTable[i].ulVal;
            return NO_ERROR;
        }
    }
    return ERROR_INVALID_PARAMETER;
}

/*
 * PMBIDI_BinVal_To_String -- ordinal 111
 *
 * Convert a bidi attribute ULONG back to its keyword string.
 * pszBuffer must be at least 32 bytes.
 */
APIRET APIENTRY PMBIDI_BinVal_To_String( ULONG ulValue, PSZ pszBuffer, ULONG cbBuffer )
{
    static const struct { ULONG ulVal; const char *pszKey; } aTable[] = {
        { BIDI_ATTR_TEXTTYPE_VISUAL,    "TEXTTYPE_VISUAL"   },
        { BIDI_ATTR_ORIENT_RTL,         "_ORIENT_RTL"       },
        { BIDI_ATTR_NUMERALS_PASSTHRU,  "PASSTHRU"          },
        { BIDI_ATTR_SYM_SWAP_OFF,       "SYM_SWAP_OFF"      },
        { BIDI_ATTR_WORD_BREAK_ON,      "WORD_BREAK_ON"     },
        { BIDI_ATTR_DISPLAY_SHAPED,     "_DISPLAY_SHAPED"   },
        { BIDI_ATTR_SCREEN_FIELD_REV,   "SCREEN_FIELD_REV"  },
        { BIDI_ATTR_STATUS_INDICATOR,   "STATUS_INDICATOR"  },
        { BIDI_ATTR_HKFLAG_ENG_LAYER,   "HKFLAG_ENG_LAYER"  },
        { BIDI_ATTR_INPUT_PROCESSING,   "INPUT_PROCESSING"  },
        { 0, NULL }
    };
    int i;

    if ( !pszBuffer ) return ERROR_INVALID_PARAMETER;

    for ( i = 0; aTable[i].pszKey; i++ )
    {
        if ( aTable[i].ulVal == ulValue )
        {
            if ( strlen(aTable[i].pszKey) + 1 > cbBuffer )
                return ERROR_BUFFER_OVERFLOW;
            strcpy( pszBuffer, aTable[i].pszKey );
            return NO_ERROR;
        }
    }
    return ERROR_INVALID_PARAMETER;
}

/*
 * PMBIDI_Keyword_To_BinVal -- ordinal 112
 *
 * Synonym for PMBIDI_String_To_BinVal (same call signature).
 */
APIRET APIENTRY PMBIDI_Keyword_To_BinVal( PSZ pszKeyword, PULONG pulValue )
{
    return PMBIDI_String_To_BinVal( pszKeyword, pulValue );
}

/*
 * PMBIDI_Get_HelpBidi_Setting -- ordinal 113
 */
APIRET APIENTRY PMBIDI_Get_HelpBidi_Setting( PULONG pulSetting )
{
    ULONG  ulVal  = 0;
    ULONG  cbData = sizeof(ULONG);

    if ( !pulSetting ) return ERROR_INVALID_PARAMETER;

    PrfQueryProfileData( HINI_USERPROFILE,
                         (PSZ)szBidiApp, (PSZ)szKeyHelpBidi,
                         &ulVal, &cbData );
    *pulSetting = ulVal;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Group 6 & 7 -- NLS Bidi String Operations (ords 200-209)            */
/* ------------------------------------------------------------------ */

/*
 * Nls32ConvertBidiString -- ordinal 205
 *
 * Convert a bidi string between visual and implicit ordering.
 * pszSrc:   source string (NUL-terminated)
 * pszDst:   destination buffer (at least cbSrc bytes)
 * cbSrc:    byte count of source string (including NUL)
 * flFlags:  NLS_BIDI_* flags controlling the conversion direction
 */
APIRET APIENTRY Nls32ConvertBidiString( PSZ    pszSrc,
                                         PSZ    pszDst,
                                         ULONG  cbSrc,
                                         ULONG  flFlags )
{
    if ( !pszSrc || !pszDst || cbSrc == 0 ) return ERROR_INVALID_PARAMETER;

    /* If BDCALL32 is loaded, delegate; otherwise provide identity transform */
    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 205, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, (ULONG)pszDst, cbSrc, flFlags );
    }

    /* Fallback: identity copy */
    memcpy( pszDst, pszSrc, cbSrc );
    return NO_ERROR;
}

/*
 * Nls32EditShape -- ordinal 206
 *
 * Apply Arabic character shaping to a string (initial/medial/final/isolated).
 */
APIRET APIENTRY Nls32EditShape( PSZ pszSrc, PSZ pszDst, ULONG cbSrc, ULONG flFlags )
{
    if ( !pszSrc || !pszDst || cbSrc == 0 ) return ERROR_INVALID_PARAMETER;

    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 206, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, (ULONG)pszDst, cbSrc, flFlags );
    }

    memcpy( pszDst, pszSrc, cbSrc );
    return NO_ERROR;
}

/*
 * Nls32InverseString -- ordinal 207
 *
 * Reverse the character order of a string for bidi display.
 */
APIRET APIENTRY Nls32InverseString( PSZ pszSrc, PSZ pszDst, ULONG cbSrc )
{
    ULONG i;
    if ( !pszSrc || !pszDst || cbSrc == 0 ) return ERROR_INVALID_PARAMETER;

    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 207, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, (ULONG)pszDst, cbSrc );
    }

    /* Fallback: naive byte reversal (excludes trailing NUL) */
    {
        ULONG len = ( cbSrc > 0 && pszSrc[cbSrc-1] == '\0' ) ? cbSrc-1 : cbSrc;
        for ( i = 0; i < len; i++ )
            pszDst[i] = pszSrc[len - 1 - i];
        if ( cbSrc > 0 && pszSrc[cbSrc-1] == '\0' )
            pszDst[len] = '\0';
    }
    return NO_ERROR;
}

/*
 * Nls32ShapeBidiString -- ordinal 208
 *
 * Shape a bidirectional string for display (combines shaping + reordering).
 */
APIRET APIENTRY Nls32ShapeBidiString( PSZ pszSrc, PSZ pszDst, ULONG cbSrc, ULONG flFlags )
{
    if ( !pszSrc || !pszDst || cbSrc == 0 ) return ERROR_INVALID_PARAMETER;

    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 208, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, (ULONG)pszDst, cbSrc, flFlags );
    }

    memcpy( pszDst, pszSrc, cbSrc );
    return NO_ERROR;
}

/*
 * Nls32ConvertBidiNumerics -- ordinal 209
 *
 * Convert numeral characters between Arabic-Indic and Western Arabic.
 */
APIRET APIENTRY Nls32ConvertBidiNumerics( PSZ pszSrc, PSZ pszDst,
                                           ULONG cbSrc, ULONG flFlags )
{
    if ( !pszSrc || !pszDst || cbSrc == 0 ) return ERROR_INVALID_PARAMETER;

    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 209, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, (ULONG)pszDst, cbSrc, flFlags );
    }

    memcpy( pszDst, pszSrc, cbSrc );
    return NO_ERROR;
}

/* 16-bit wrappers -- ords 200-204 */
APIRET APIENTRY NLS16CONVERTBIDISTRING( PSZ s, PSZ d, ULONG cb, ULONG fl )
    { return Nls32ConvertBidiString(s,d,cb,fl); }
APIRET APIENTRY NLS16EDITSHAPE( PSZ s, PSZ d, ULONG cb, ULONG fl )
    { return Nls32EditShape(s,d,cb,fl); }
APIRET APIENTRY NLS16INVERSESTRING( PSZ s, PSZ d, ULONG cb )
    { return Nls32InverseString(s,d,cb); }
APIRET APIENTRY NLS16SHAPEBIDISTRING( PSZ s, PSZ d, ULONG cb, ULONG fl )
    { return Nls32ShapeBidiString(s,d,cb,fl); }
APIRET APIENTRY NLS16CONVERTBIDINUMERICS( PSZ s, PSZ d, ULONG cb, ULONG fl )
    { return Nls32ConvertBidiNumerics(s,d,cb,fl); }

/* ------------------------------------------------------------------ */
/* Group 8 -- layout_object (ords 306-311)                             */
/* ------------------------------------------------------------------ */

/*
 * Internal layout object structure.
 * The keyword attribute table lives in the original Obj9 shared data.
 * Here we keep it simple: each object is a heap-allocated block.
 */
typedef struct _LAYOUT_OBJ_INTERNAL {
    ULONG   ulMagic;        /* 'LOBJ' = 0x4c4f424a */
    ULONG   flTextType;     /* LAYOUT_TEXTTYPE_* */
    ULONG   flOrientation;  /* LAYOUT_ORIENTATION_* */
    ULONG   flNumerals;     /* LAYOUT_NUMERALS_* */
    ULONG   flSymSwap;
    ULONG   flWordBreak;
    ULONG   flDisplayShape;
    ULONG   cbAlloc;
} LAYOUT_OBJ_INTERNAL;

#define LAYOUT_OBJ_MAGIC 0x4c4f424aUL  /* 'LOBJ' */

/*
 * layout_object_create -- ordinal 306
 *
 * Allocate and initialise a new layout object.
 * *pLayoutObj receives the handle.  Returns 0 on success, -1 on failure.
 */
int APIENTRY layout_object_create( ULONG ulExtra, PLAYOUT_OBJECT pLayoutObj )
{
    LAYOUT_OBJ_INTERNAL *pObj;
    APIRET arc;

    if ( !pLayoutObj ) return -1;

    arc = DosAllocMem( (PPVOID)&pObj, sizeof(LAYOUT_OBJ_INTERNAL),
                       PAG_COMMIT | PAG_READ | PAG_WRITE );
    if ( arc != NO_ERROR ) return -1;

    memset( pObj, 0, sizeof(LAYOUT_OBJ_INTERNAL) );
    pObj->ulMagic  = LAYOUT_OBJ_MAGIC;
    pObj->cbAlloc  = sizeof(LAYOUT_OBJ_INTERNAL);

    *pLayoutObj = (LAYOUT_OBJECT)pObj;
    return 0;
}

/*
 * layout_object_destroy -- ordinal 307
 *
 * Free a layout object previously created with layout_object_create.
 * Returns 0 on success, -1 on failure.
 */
int APIENTRY layout_object_destroy( LAYOUT_OBJECT layoutObj )
{
    LAYOUT_OBJ_INTERNAL *pObj = (LAYOUT_OBJ_INTERNAL *)layoutObj;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;

    pObj->ulMagic = 0;   /* Poison */
    DosFreeMem( pObj );
    return 0;
}

/*
 * layout_object_transform -- ordinal 308
 *
 * Apply the bidi transformation described by layoutObj to the input
 * string pszSrc, writing results to pszDst.
 */
int APIENTRY layout_object_transform( LAYOUT_OBJECT layoutObj,
                                       LayoutValues   pIn,
                                       LayoutValues   pOut,
                                       int            *pFlag )
{
    LAYOUT_OBJ_INTERNAL *pObj = (LAYOUT_OBJ_INTERNAL *)layoutObj;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pIn || !pOut )                               return -1;

    /* Delegate to BDCALL32 if available */
    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 308, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return (int)pfn( (ULONG)layoutObj, (ULONG)pIn, (ULONG)pOut, (ULONG)pFlag );
    }

    return -1;
}

/*
 * layout_object_editshape -- ordinal 309
 *
 * Apply shape editing to strings within the layout object context.
 */
int APIENTRY layout_object_editshape( LAYOUT_OBJECT layoutObj,
                                       ULONG cbSrc, PSZ pszSrc,
                                       ULONG *pcbDst, PSZ pszDst )
{
    LAYOUT_OBJ_INTERNAL *pObj = (LAYOUT_OBJ_INTERNAL *)layoutObj;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;

    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 309, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return (int)pfn( (ULONG)layoutObj, cbSrc, (ULONG)pszSrc,
                             (ULONG)pcbDst, (ULONG)pszDst );
    }

    if ( pszSrc && pszDst && pcbDst && cbSrc )
    {
        memcpy( pszDst, pszSrc, cbSrc );
        *pcbDst = cbSrc;
    }
    return 0;
}

/*
 * layout_object_setvalues -- ordinal 310
 *
 * Set one or more layout object attributes.  pValues is an array of
 * (name, value) pairs terminated by a {0,NULL} entry.
 */
int APIENTRY layout_object_setvalues( LAYOUT_OBJECT layoutObj,
                                       LayoutValues  pValues,
                                       int           *pIndex )
{
    LAYOUT_OBJ_INTERNAL *pObj = (LAYOUT_OBJ_INTERNAL *)layoutObj;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pValues ) return -1;

    /* Store each attribute -- pIndex receives index of first failing attr or -1 */
    {
        int i = 0;
        for ( ; pValues[i].value != NULL || pValues[i].name != 0; i++ )
        {
            switch ( pValues[i].name )
            {
            case LAYOUT_TEXTTYPE_VISUAL:
                pObj->flTextType = LAYOUT_TEXTTYPE_VISUAL; break;
            case LAYOUT_TEXTTYPE_IMPLICIT:
                pObj->flTextType = LAYOUT_TEXTTYPE_IMPLICIT; break;
            case LAYOUT_ORIENTATION_LTR:
                pObj->flOrientation = LAYOUT_ORIENTATION_LTR; break;
            case LAYOUT_ORIENTATION_RTL:
                pObj->flOrientation = LAYOUT_ORIENTATION_RTL; break;
            case LAYOUT_ORIENTATION_CONTEXT:
                pObj->flOrientation = LAYOUT_ORIENTATION_CONTEXT; break;
            case LAYOUT_NUMERALS_NOMINAL:
                pObj->flNumerals = LAYOUT_NUMERALS_NOMINAL; break;
            case LAYOUT_NUMERALS_PASSTHRU:
                pObj->flNumerals = LAYOUT_NUMERALS_PASSTHRU; break;
            case LAYOUT_SYM_SWAP_OFF:
                pObj->flSymSwap = 1; break;
            case LAYOUT_WORD_BREAK_ON:
                pObj->flWordBreak = 1; break;
            case LAYOUT_DISPLAY_SHAPED:
                pObj->flDisplayShape = 1; break;
            default:
                if ( pIndex ) *pIndex = i;
                return -1;
            }
        }
    }
    if ( pIndex ) *pIndex = -1;
    return 0;
}

/*
 * layout_object_getvalues -- ordinal 311
 *
 * Retrieve one or more layout object attributes.
 */
int APIENTRY layout_object_getvalues( LAYOUT_OBJECT layoutObj,
                                       LayoutValues  pValues,
                                       int           *pIndex )
{
    LAYOUT_OBJ_INTERNAL *pObj = (LAYOUT_OBJ_INTERNAL *)layoutObj;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pValues ) return -1;

    {
        int i = 0;
        for ( ; pValues[i].value != NULL || pValues[i].name != 0; i++ )
        {
            ULONG *pulDst = (ULONG *)pValues[i].value;
            if ( !pulDst ) continue;
            switch ( pValues[i].name )
            {
            case LAYOUT_TEXTTYPE_VISUAL:
            case LAYOUT_TEXTTYPE_IMPLICIT:
                *pulDst = pObj->flTextType; break;
            case LAYOUT_ORIENTATION_LTR:
            case LAYOUT_ORIENTATION_RTL:
            case LAYOUT_ORIENTATION_CONTEXT:
                *pulDst = pObj->flOrientation; break;
            case LAYOUT_NUMERALS_NOMINAL:
            case LAYOUT_NUMERALS_PASSTHRU:
                *pulDst = pObj->flNumerals; break;
            case LAYOUT_SYM_SWAP_OFF:
                *pulDst = pObj->flSymSwap; break;
            case LAYOUT_WORD_BREAK_ON:
                *pulDst = pObj->flWordBreak; break;
            case LAYOUT_DISPLAY_SHAPED:
                *pulDst = pObj->flDisplayShape; break;
            default:
                if ( pIndex ) *pIndex = i;
                return -1;
            }
        }
    }
    if ( pIndex ) *pIndex = -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Group 9 -- Codepage Translation (ords 400-401)                      */
/* ------------------------------------------------------------------ */

/*
 * PMBIDI_CpTranslateString -- ordinal 401
 *
 * Translate a string from one codepage to another using the built-in
 * EBCDIC<->ASCII tables (Obj5) or via BDCALL32 for other pairs.
 *
 * ulCpSrc: source codepage (37=EBCDIC, 850=ASCII/Multilingual, etc.)
 * ulCpDst: destination codepage
 */
APIRET APIENTRY PMBIDI_CpTranslateString( PSZ   pszSrc,
                                           ULONG cbSrc,
                                           ULONG ulCpSrc,
                                           PSZ   pszDst,
                                           ULONG cbDst,
                                           ULONG ulCpDst )
{
    ULONG i;

    if ( !pszSrc || !pszDst || cbSrc == 0 ) return ERROR_INVALID_PARAMETER;
    if ( cbDst < cbSrc )                     return ERROR_BUFFER_OVERFLOW;

    /* EBCDIC (CP 37) -> ASCII (CP 850 / 437 / 1004) */
    if ( ulCpSrc == 37 && ulCpDst != 37 )
    {
        for ( i = 0; i < cbSrc; i++ )
            pszDst[i] = (CHAR)ebcdic2ascii[(UCHAR)pszSrc[i]];
        return NO_ERROR;
    }

    /* ASCII -> EBCDIC */
    if ( ulCpSrc != 37 && ulCpDst == 37 )
    {
        for ( i = 0; i < cbSrc; i++ )
            pszDst[i] = (CHAR)ascii2ebcdic[(UCHAR)pszSrc[i]];
        return NO_ERROR;
    }

    /* Other conversions: delegate to BDCALL32 */
    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 401, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, cbSrc, ulCpSrc,
                        (ULONG)pszDst, cbDst, ulCpDst );
    }

    /* Fallback: identity copy */
    memcpy( pszDst, pszSrc, cbSrc );
    return NO_ERROR;
}

APIRET APIENTRY PMBIDI_16CP_TRANSLATE_STRING( PSZ s, ULONG cbS, ULONG cSrc,
                                               PSZ d, ULONG cbD, ULONG cDst )
{
    return PMBIDI_CpTranslateString(s, cbS, cSrc, d, cbD, cDst);
}

/* ------------------------------------------------------------------ */
/* Group 10 -- Advanced Bidi Operations (ords 450-454, 460-461)        */
/* ------------------------------------------------------------------ */

/*
 * Bidi_MapSrcToTrg -- ordinal 450
 *
 * Build a position mapping array from source to target string after
 * a bidi reorder.  pMap is an array of (cbSrc) ULONGs.
 */
APIRET APIENTRY Bidi_MapSrcToTrg( PSZ pszSrc, ULONG cbSrc,
                                   PSZ pszTrg, ULONG cbTrg,
                                   PULONG pMap, ULONG flFlags )
{
    if ( !pszSrc || !pszTrg || !pMap ) return ERROR_INVALID_PARAMETER;

    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 450, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)pszSrc, cbSrc, (ULONG)pszTrg, cbTrg,
                        (ULONG)pMap, flFlags );
    }

    /* Identity mapping fallback */
    { ULONG i; for (i=0; i<cbSrc; i++) pMap[i] = i; }
    return NO_ERROR;
}

/*
 * Bidi_ClassifyCodepage -- ordinal 451
 *
 * Returns a classification of the codepage:
 *   0 = neutral / LTR only
 *   1 = bidi capable (Arabic/Hebrew)
 *   2 = RTL default
 */
ULONG APIENTRY Bidi_ClassifyCodepage( ULONG ulCp )
{
    /* Hebrew codepages: 862, 1255 */
    if ( ulCp == 862 || ulCp == 1255 ) return 1;
    /* Arabic codepages: 864, 1256 */
    if ( ulCp == 864 || ulCp == 1256 ) return 1;
    /* EBCDIC Hebrew: 424 */
    if ( ulCp == 424 ) return 1;
    /* EBCDIC Arabic: 420 */
    if ( ulCp == 420 ) return 1;
    return 0;
}

/*
 * Bidi_IsStringBidi -- ordinal 452
 *
 * Returns TRUE if the string contains at least one bidi character.
 */
BOOL APIENTRY Bidi_IsStringBidi( PSZ pszStr, ULONG cbStr, ULONG ulCp )
{
    ULONG i;
    if ( !pszStr ) return FALSE;

    /* For Arabic/Hebrew codepages, test if any byte is in the bidi range */
    if ( Bidi_ClassifyCodepage(ulCp) == 0 ) return FALSE;

    for ( i = 0; i < cbStr; i++ )
    {
        UCHAR b = (UCHAR)pszStr[i];
        /* Hebrew CP862: 0x80-0x9a are Hebrew letters */
        if ( ulCp == 862 && b >= 0x80 && b <= 0x9a ) return TRUE;
        /* Arabic CP864: 0xc1-0xfe are Arabic letters (approx) */
        if ( ulCp == 864 && b >= 0xc1 ) return TRUE;
    }
    return FALSE;
}

/*
 * Bidi_IsStringAllBidi -- ordinal 453
 *
 * Returns TRUE if EVERY printable character in the string is bidi.
 */
BOOL APIENTRY Bidi_IsStringAllBidi( PSZ pszStr, ULONG cbStr, ULONG ulCp )
{
    ULONG i;
    if ( !pszStr || cbStr == 0 ) return FALSE;
    if ( Bidi_ClassifyCodepage(ulCp) == 0 ) return FALSE;

    for ( i = 0; i < cbStr; i++ )
    {
        UCHAR b = (UCHAR)pszStr[i];
        if ( b == 0 ) continue;   /* NUL terminator */
        if ( b < 0x80 ) return FALSE;  /* ASCII range = not bidi */
    }
    return TRUE;
}

/*
 * Bidi_ReverseString -- ordinal 454
 *
 * Reverse the byte order of a string in-place.
 */
APIRET APIENTRY Bidi_ReverseString( PSZ pszStr, ULONG cbStr )
{
    ULONG lo, hi;
    CHAR  tmp;

    if ( !pszStr || cbStr < 2 ) return NO_ERROR;

    lo = 0;
    hi = cbStr - 1;
    /* Don't reverse trailing NUL */
    if ( pszStr[hi] == '\0' && hi > 0 ) hi--;

    while ( lo < hi )
    {
        tmp         = pszStr[lo];
        pszStr[lo]  = pszStr[hi];
        pszStr[hi]  = tmp;
        lo++; hi--;
    }
    return NO_ERROR;
}

/*
 * Bidi_LayoutConvert -- ordinal 460
 *
 * Convert a string's layout (visual<->implicit) using the full Unicode
 * Bidirectional Algorithm, delegating to BDCALL32.
 */
APIRET APIENTRY Bidi_LayoutConvert( LAYOUT_OBJECT layoutObj,
                                     LayoutValues  pInValues,
                                     LayoutValues  pOutValues,
                                     int           *pConvFlags )
{
    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 460, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)layoutObj, (ULONG)pInValues,
                        (ULONG)pOutValues, (ULONG)pConvFlags );
    }
    return ERROR_NOT_SUPPORTED;
}

/*
 * Bidi_LayoutEdit -- ordinal 461
 *
 * Perform an edit operation (insert/delete) within a bidi-reordered
 * string while maintaining correct visual/logical correspondence.
 */
APIRET APIENTRY Bidi_LayoutEdit( LAYOUT_OBJECT layoutObj,
                                  LayoutValues  pInValues,
                                  LayoutValues  pOutValues )
{
    if ( g_hmodBDCALL32 != NULLHANDLE )
    {
        PFN_BDCALL pfn = NULL;
        if ( DosQueryProcAddr( g_hmodBDCALL32, 461, NULL, (PFN *)&pfn ) == NO_ERROR && pfn )
            return pfn( (ULONG)layoutObj, (ULONG)pInValues, (ULONG)pOutValues );
    }
    return ERROR_NOT_SUPPORTED;
}

/* ------------------------------------------------------------------ */
/* DLL Initialisation / Termination                                    */
/* ------------------------------------------------------------------ */

/*
 * LibMain -- called by the C runtime DLL startup stub (__DLLstart_).
 *
 * flag == 0: initialisation (called once per process attach,
 *            because we use INITINSTANCE in the linker script).
 * flag == 1: termination.
 *
 * Returns: 1 (TRUE) = success, 0 = failure (aborts load).
 */
unsigned _System LibMain( unsigned hmod, unsigned flag )
{
    (void)hmod;

    if ( flag == 0 )
    {
        /* Initialisation: try to load BDCALL32 early.
         * Not fatal if it fails here; PMBIDI_Initialize will retry. */
        LoadBDCALL32();

        /* Initialise per-process globals */
        g_ulInitialized  = 0;
        ulProcess_BidiAttr = 0;
        memset( &g_LangInfo, 0, sizeof(g_LangInfo) );
        g_LangInfo.cbSize = sizeof(LANGINFO);
    }
    else
    {
        /* Termination: release BDCALL32 if we loaded it */
        if ( g_hmodBDCALL32 != NULLHANDLE )
        {
            DosFreeModule( g_hmodBDCALL32 );
            g_hmodBDCALL32 = NULLHANDLE;
        }
        g_ulInitialized = 0;
    }

    return 1;
}
