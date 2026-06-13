/*
 * PMBIDI.H  --  Public interface for PMBIDI.DLL
 *               OS/2 PM Bidirectional Language APIs
 *
 * Include after os2.h:
 *   #define INCL_BASE
 *   #define INCL_WIN
 *   #define INCL_GPI
 *   #include <os2.h>
 *   #include "PMBIDI.H"
 *
 * Link against PMBIDI.LIB (import library generated from PMBIDI.DLL).
 */

#ifndef PMBIDI_H_INCLUDED
#define PMBIDI_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* LANGINFO  --  language/keyboard/viewer information (per-process)    */
/* ------------------------------------------------------------------ */

#pragma pack(1)

typedef struct _LANGINFO {
    ULONG   cbSize;         /* must be sizeof(LANGINFO) == 24         */
    ULONG   flReserved;     /* must be 0                              */
    ULONG   idLang;         /* language identifier                    */
    ULONG   idKbdLayer;     /* keyboard layer ID                      */
    ULONG   idLangViewer;   /* language viewer ID                     */
    ULONG   flBidiAttr;     /* bidi attribute flags (BIDI_ATTR_*)     */
} LANGINFO;
typedef LANGINFO *PLANGINFO;

/* BIDIATTR  --  GPI bidi presentation parameter */
typedef struct _BIDIATTR {
    ULONG   cbSize;         /* must be sizeof(BIDIATTR) == 8          */
    ULONG   flBidiAttr;     /* bidi attribute flags                   */
} BIDIATTR;
typedef BIDIATTR *PBIDIATTR;

/* layout_object attribute value pair (name + pointer to value)       */
typedef struct _LayoutValueRec {
    int     name;           /* LAYOUT_* attribute name constant       */
    PVOID   value;          /* pointer to the attribute value (ULONG) */
} LayoutValueRec;
typedef LayoutValueRec *LayoutValues;

#pragma pack()

/* ------------------------------------------------------------------ */
/* flBidiAttr flag bits                                                */
/* ------------------------------------------------------------------ */
#define BIDI_ATTR_TEXTTYPE_VISUAL       0x00000001
#define BIDI_ATTR_ORIENT_RTL            0x00000002
#define BIDI_ATTR_NUMERALS_PASSTHRU     0x00000004
#define BIDI_ATTR_SYM_SWAP_OFF          0x00000008
#define BIDI_ATTR_WORD_BREAK_ON         0x00000010
#define BIDI_ATTR_DISPLAY_SHAPED        0x00000020
#define BIDI_ATTR_SCREEN_FIELD_REV      0x00000040
#define BIDI_ATTR_STATUS_INDICATOR      0x00000080
#define BIDI_ATTR_HKFLAG_ENG_LAYER      0x00000100
#define BIDI_ATTR_INPUT_PROCESSING      0x00000200

/* NLS string conversion flags */
#define NLS_BIDI_VISUAL                 0x0001
#define NLS_BIDI_IMPLICIT               0x0002
#define NLS_BIDI_LTR                    0x0004
#define NLS_BIDI_RTL                    0x0008
#define NLS_BIDI_ORIENT_CONTEXT         0x0010

/* layout_object attribute name constants */
#define LAYOUT_TEXTTYPE_VISUAL          0
#define LAYOUT_TEXTTYPE_IMPLICIT        1
#define LAYOUT_ORIENTATION_LTR          2
#define LAYOUT_ORIENTATION_RTL          3
#define LAYOUT_ORIENTATION_CONTEXT      4
#define LAYOUT_NUMERALS_NOMINAL         5
#define LAYOUT_NUMERALS_PASSTHRU        6
#define LAYOUT_SYM_SWAP_OFF             7
#define LAYOUT_WORD_BREAK_ON            8
#define LAYOUT_DISPLAY_SHAPED           9

/* layout_object handle */
typedef PVOID LAYOUT_OBJECT;
typedef LAYOUT_OBJECT *PLAYOUT_OBJECT;

/* ------------------------------------------------------------------ */
/* Exported data                                                       */
/* ------------------------------------------------------------------ */

/*
 * ulProcess_BidiAttr  (ordinal 999)
 *
 * Per-process bidi attribute DWORD.  Applications may read this to
 * determine the current process-wide bidi presentation state.
 * Use Win32SetProcessLangInfo / Win32QueryProcessLangInfo to modify it
 * through the proper API rather than writing directly.
 */
extern ULONG ulProcess_BidiAttr;

/* ------------------------------------------------------------------ */
/* Group 1 -- Initialisation                                           */
/* ------------------------------------------------------------------ */

/*
 * PMBIDI_Initialize  (ordinal 1)
 *
 * Must be called once per process before any other PMBIDI function.
 * Loads BDCALL32.DLL and initialises per-process bidi state.
 *
 * Returns: NO_ERROR on success, DOS error code on failure.
 */
APIRET APIENTRY PMBIDI_Initialize( void );

/* ------------------------------------------------------------------ */
/* Group 2 -- Language / Keyboard / Viewer information                 */
/* ------------------------------------------------------------------ */

/* 32-bit functions (ordinals 20-31) -- preferred for new code */
APIRET APIENTRY Win32SetLangInfo( PLANGINFO pLangInfo );            /* 20 */
APIRET APIENTRY Win32QueryLangInfo( PLANGINFO pLangInfo );          /* 21 */
APIRET APIENTRY Win32SetKbdLayer( ULONG idKbdLayer );               /* 22 */
APIRET APIENTRY Win32QueryKbdLayer( PULONG pidKbdLayer );           /* 23 */
APIRET APIENTRY Win32SetLangViewer( ULONG idLangViewer );           /* 24 */
APIRET APIENTRY Win32QueryLangViewer( PULONG pidLangViewer );       /* 25 */
APIRET APIENTRY Win32SetProcessLangInfo( PLANGINFO pLangInfo );     /* 30 */
APIRET APIENTRY Win32QueryProcessLangInfo( PLANGINFO pLangInfo );   /* 31 */

/* 16-bit compatibility wrappers (ordinals 10-19) -- for legacy code  */
APIRET APIENTRY WIN16SETLANGINFO( PLANGINFO pLangInfo );            /* 10 */
APIRET APIENTRY WIN16QUERYLANGINFO( PLANGINFO pLangInfo );          /* 11 */
APIRET APIENTRY WIN16SETKBDLAYER( ULONG idKbdLayer );               /* 12 */
APIRET APIENTRY WIN16QUERYKBDLAYER( PULONG pidKbdLayer );           /* 13 */
APIRET APIENTRY WIN16SETLANGVIEWER( ULONG idLangViewer );           /* 14 */
APIRET APIENTRY WIN16QUERYLANGVIEWER( PULONG pidLangViewer );       /* 15 */
APIRET APIENTRY WIN16SETPROCESSLANGINFO( PLANGINFO pLangInfo );     /* 18 */
APIRET APIENTRY WIN16QUERYPROCESSLANGINFO( PLANGINFO pLangInfo );   /* 19 */

/* ------------------------------------------------------------------ */
/* Group 3 -- GPI Bidi Attributes                                      */
/* ------------------------------------------------------------------ */

APIRET APIENTRY Gpi32SetBidiAttr( HPS hps, PBIDIATTR pBidiAttr );  /* 50 */
APIRET APIENTRY Gpi32QueryBidiAttr( HPS hps, PBIDIATTR pBidiAttr );/* 51 */

/* 16-bit wrappers */
APIRET APIENTRY GPI16SETBIDIATTR( HPS hps, PBIDIATTR pBidiAttr );  /* 40 */
APIRET APIENTRY GPI16QUERYBIDIATTR( HPS hps, PBIDIATTR pBidiAttr );/* 41 */

/* ------------------------------------------------------------------ */
/* Group 4 -- Codepage Query                                           */
/* ------------------------------------------------------------------ */

APIRET APIENTRY Bidi_QueryCp( PULONG pulCp );                       /* 60 */
APIRET APIENTRY BIDI_16QUERYCP( PULONG pulCp );                     /* 61 */

/* ------------------------------------------------------------------ */
/* Group 5 -- Settings and keyword conversion                          */
/* ------------------------------------------------------------------ */

/*
 * PMBIDI_Get_StdDlgLang_Setting  (ordinal 101)
 *
 * Retrieve the standard dialog language setting from HINI_USERPROFILE
 * application "PM_BIDI", key "STDDLGLANG".
 */
APIRET APIENTRY PMBIDI_Get_StdDlgLang_Setting( PULONG pulSetting );     /* 101 */
APIRET APIENTRY PMBIDI_16GET_STDDLGLANG_SETTING( PULONG pulSetting );   /* 102 */

/*
 * PMBIDI_String_To_BinVal  (ordinal 110)
 *
 * Convert a bidi attribute keyword string (e.g. "TEXTTYPE_VISUAL")
 * to the corresponding BIDI_ATTR_* flag value.
 */
APIRET APIENTRY PMBIDI_String_To_BinVal( PSZ pszString, PULONG pulValue );  /* 110 */

/*
 * PMBIDI_BinVal_To_String  (ordinal 111)
 *
 * Convert a BIDI_ATTR_* flag value to its keyword string.
 * pszBuffer must be at least 32 bytes.
 */
APIRET APIENTRY PMBIDI_BinVal_To_String( ULONG ulValue, PSZ pszBuffer, ULONG cbBuffer ); /* 111 */

/*
 * PMBIDI_Keyword_To_BinVal  (ordinal 112)
 *
 * Synonym for PMBIDI_String_To_BinVal.
 */
APIRET APIENTRY PMBIDI_Keyword_To_BinVal( PSZ pszKeyword, PULONG pulValue ); /* 112 */

/*
 * PMBIDI_Get_HelpBidi_Setting  (ordinal 113)
 *
 * Retrieve the help bidi setting from HINI_USERPROFILE.
 */
APIRET APIENTRY PMBIDI_Get_HelpBidi_Setting( PULONG pulSetting );        /* 113 */

/* ------------------------------------------------------------------ */
/* Group 6 & 7 -- NLS Bidi String Operations                           */
/* ------------------------------------------------------------------ */

/*
 * Nls32ConvertBidiString  (ordinal 205)
 *
 * Convert a string between visual and implicit bidi ordering.
 * pszSrc:  source string
 * pszDst:  destination buffer (>= cbSrc bytes)
 * cbSrc:   byte count of source (including NUL if present)
 * flFlags: NLS_BIDI_* flags
 */
APIRET APIENTRY Nls32ConvertBidiString( PSZ pszSrc, PSZ pszDst,
                                         ULONG cbSrc, ULONG flFlags );   /* 205 */

/*
 * Nls32EditShape  (ordinal 206)
 *
 * Apply Arabic/Hebrew character shaping (initial/medial/final/isolated
 * presentation forms).
 */
APIRET APIENTRY Nls32EditShape( PSZ pszSrc, PSZ pszDst,
                                 ULONG cbSrc, ULONG flFlags );           /* 206 */

/*
 * Nls32InverseString  (ordinal 207)
 *
 * Reverse the character order of a string.
 */
APIRET APIENTRY Nls32InverseString( PSZ pszSrc, PSZ pszDst, ULONG cbSrc ); /* 207 */

/*
 * Nls32ShapeBidiString  (ordinal 208)
 *
 * Shape and reorder a bidi string for display in a single operation.
 */
APIRET APIENTRY Nls32ShapeBidiString( PSZ pszSrc, PSZ pszDst,
                                       ULONG cbSrc, ULONG flFlags );     /* 208 */

/*
 * Nls32ConvertBidiNumerics  (ordinal 209)
 *
 * Convert numeral characters between Arabic-Indic (U+0660..U+0669) and
 * Western Arabic (0x30..0x39) forms.
 */
APIRET APIENTRY Nls32ConvertBidiNumerics( PSZ pszSrc, PSZ pszDst,
                                           ULONG cbSrc, ULONG flFlags ); /* 209 */

/* 16-bit wrappers (ordinals 200-204) */
APIRET APIENTRY NLS16CONVERTBIDISTRING( PSZ s, PSZ d, ULONG cb, ULONG fl );  /* 200 */
APIRET APIENTRY NLS16EDITSHAPE( PSZ s, PSZ d, ULONG cb, ULONG fl );          /* 201 */
APIRET APIENTRY NLS16INVERSESTRING( PSZ s, PSZ d, ULONG cb );                /* 202 */
APIRET APIENTRY NLS16SHAPEBIDISTRING( PSZ s, PSZ d, ULONG cb, ULONG fl );    /* 203 */
APIRET APIENTRY NLS16CONVERTBIDINUMERICS( PSZ s, PSZ d, ULONG cb, ULONG fl );/* 204 */

/* ------------------------------------------------------------------ */
/* Group 8 -- layout_object (Unicode Bidirectional Algorithm objects)  */
/* ------------------------------------------------------------------ */

/*
 * layout_object_create  (ordinal 306)
 *
 * Allocate a new layout object.  *pLayoutObj receives the handle.
 * ulExtra is reserved, pass 0.
 * Returns 0 on success, -1 on failure.
 */
int APIENTRY layout_object_create( ULONG ulExtra,
                                    PLAYOUT_OBJECT pLayoutObj );         /* 306 */

/*
 * layout_object_destroy  (ordinal 307)
 *
 * Free a layout object.  Returns 0 on success, -1 if handle is invalid.
 */
int APIENTRY layout_object_destroy( LAYOUT_OBJECT layoutObj );           /* 307 */

/*
 * layout_object_transform  (ordinal 308)
 *
 * Apply the bidi transformation described by the object to input strings
 * in pInValues and write results to pOutValues.
 * *pConvFlags receives conversion status flags.
 */
int APIENTRY layout_object_transform( LAYOUT_OBJECT layoutObj,
                                       LayoutValues  pInValues,
                                       LayoutValues  pOutValues,
                                       int           *pConvFlags );      /* 308 */

/*
 * layout_object_editshape  (ordinal 309)
 *
 * Apply shape editing within a layout object context.
 * cbSrc / *pcbDst are byte counts; pszSrc and pszDst are the buffers.
 */
int APIENTRY layout_object_editshape( LAYOUT_OBJECT layoutObj,
                                       ULONG cbSrc, PSZ pszSrc,
                                       ULONG *pcbDst, PSZ pszDst );      /* 309 */

/*
 * layout_object_setvalues  (ordinal 310)
 *
 * Set one or more layout attributes.  pValues is a {name,&value} array
 * terminated by {0,NULL}.  On error *pIndex receives the failing entry
 * index, or -1 on complete success.
 */
int APIENTRY layout_object_setvalues( LAYOUT_OBJECT layoutObj,
                                       LayoutValues  pValues,
                                       int           *pIndex );          /* 310 */

/*
 * layout_object_getvalues  (ordinal 311)
 *
 * Query one or more layout attributes.  Same array convention as
 * layout_object_setvalues; each value pointer must point to a ULONG.
 */
int APIENTRY layout_object_getvalues( LAYOUT_OBJECT layoutObj,
                                       LayoutValues  pValues,
                                       int           *pIndex );          /* 311 */

/* ------------------------------------------------------------------ */
/* Group 9 -- Codepage Translation                                     */
/* ------------------------------------------------------------------ */

/*
 * PMBIDI_CpTranslateString  (ordinal 401)
 *
 * Translate a string from codepage ulCpSrc to ulCpDst.
 * Built-in support for EBCDIC(37)<->ASCII(850/437) via internal tables.
 * Other pairs are delegated to BDCALL32.
 *
 * pszSrc:  source string
 * cbSrc:   byte count of source
 * ulCpSrc: source codepage (e.g. 37, 850, 862, 864)
 * pszDst:  destination buffer (>= cbSrc bytes)
 * cbDst:   size of destination buffer
 * ulCpDst: target codepage
 */
APIRET APIENTRY PMBIDI_CpTranslateString( PSZ pszSrc, ULONG cbSrc, ULONG ulCpSrc,
                                           PSZ pszDst, ULONG cbDst, ULONG ulCpDst ); /* 401 */

APIRET APIENTRY PMBIDI_16CP_TRANSLATE_STRING( PSZ s, ULONG cbS, ULONG cSrc,
                                               PSZ d, ULONG cbD, ULONG cDst );       /* 400 */

/* ------------------------------------------------------------------ */
/* Group 10 -- Advanced Bidi Operations                                */
/* ------------------------------------------------------------------ */

/*
 * Bidi_MapSrcToTrg  (ordinal 450)
 *
 * Build a position mapping array pMap[0..cbSrc-1] where pMap[i] is the
 * position in pszTrg that corresponds to position i in pszSrc after a
 * bidi reorder.
 */
APIRET APIENTRY Bidi_MapSrcToTrg( PSZ pszSrc, ULONG cbSrc,
                                   PSZ pszTrg, ULONG cbTrg,
                                   PULONG pMap, ULONG flFlags );         /* 450 */

/*
 * Bidi_ClassifyCodepage  (ordinal 451)
 *
 * Returns: 0 = neutral/LTR only, 1 = bidi capable, 2 = RTL default.
 */
ULONG APIENTRY Bidi_ClassifyCodepage( ULONG ulCp );                      /* 451 */

/*
 * Bidi_IsStringBidi  (ordinal 452)
 *
 * Returns TRUE if the string contains at least one bidi character.
 */
BOOL  APIENTRY Bidi_IsStringBidi( PSZ pszStr, ULONG cbStr, ULONG ulCp ); /* 452 */

/*
 * Bidi_IsStringAllBidi  (ordinal 453)
 *
 * Returns TRUE if every printable character in the string is bidi.
 */
BOOL  APIENTRY Bidi_IsStringAllBidi( PSZ pszStr, ULONG cbStr, ULONG ulCp ); /* 453 */

/*
 * Bidi_ReverseString  (ordinal 454)
 *
 * Reverse a string in-place (excluding trailing NUL).
 */
APIRET APIENTRY Bidi_ReverseString( PSZ pszStr, ULONG cbStr );           /* 454 */

/*
 * Bidi_LayoutConvert  (ordinal 460)
 *
 * Full Unicode Bidirectional Algorithm layout conversion.
 * Requires BDCALL32.DLL to be present.
 */
APIRET APIENTRY Bidi_LayoutConvert( LAYOUT_OBJECT layoutObj,
                                     LayoutValues  pInValues,
                                     LayoutValues  pOutValues,
                                     int           *pConvFlags );        /* 460 */

/*
 * Bidi_LayoutEdit  (ordinal 461)
 *
 * Edit operation within a bidi-reordered string while maintaining
 * correct visual/logical correspondence.
 */
APIRET APIENTRY Bidi_LayoutEdit( LAYOUT_OBJECT layoutObj,
                                  LayoutValues  pInValues,
                                  LayoutValues  pOutValues );            /* 461 */

/* ------------------------------------------------------------------ */
/* Convenience macro: initialise a LANGINFO on the stack              */
/* ------------------------------------------------------------------ */
#define LANGINFO_INIT(li) \
    memset(&(li), 0, sizeof(LANGINFO)); \
    (li).cbSize = sizeof(LANGINFO)

/* ------------------------------------------------------------------ */
/* Convenience macro: initialise a BIDIATTR on the stack              */
/* ------------------------------------------------------------------ */
#define BIDIATTR_INIT(ba) \
    (ba).cbSize    = sizeof(BIDIATTR); \
    (ba).flBidiAttr = 0

#ifdef __cplusplus
}
#endif

#endif /* PMBIDI_H_INCLUDED */
