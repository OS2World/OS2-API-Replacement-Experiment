/*
 * BDCALL32.H  --  Public interface for BDCALL32.DLL
 *                 OS/2 Bidirectional Language Engine APIs
 *
 * Include after os2.h:
 *   #define INCL_BASE
 *   #include <os2.h>
 *   #include "BDCALL32.H"
 *
 * Note: PMBIDI.DLL is the normal caller of these functions.
 * Direct use is possible for applications that need the bidi engine
 * without the PM layer.
 */

#ifndef BDCALL32_H_INCLUDED
#define BDCALL32_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* NLS bidi conversion flags                                           */
/* ------------------------------------------------------------------ */
#define NLS_BIDI_VISUAL              0x0001  /* string is in visual order  */
#define NLS_BIDI_IMPLICIT            0x0002  /* string is in logical order */
#define NLS_BIDI_LTR                 0x0004  /* force LTR paragraph dir    */
#define NLS_BIDI_RTL                 0x0008  /* force RTL paragraph dir    */
#define NLS_BIDI_SHAPED              0x0010  /* string is already shaped   */
#define NLS_BIDI_NUMERALS_NOMINAL    0x0020  /* leave numerals as-is       */
#define NLS_BIDI_NUMERALS_PASSTHRU   0x0040  /* convert to native numeral  */

/* Bidi attribute flags (32-bit, shared with PMBIDI.DLL) */
#define BIDI_ATTR_TEXTTYPE_VISUAL    0x00000001
#define BIDI_ATTR_ORIENT_RTL         0x00000002
#define BIDI_ATTR_NUMERALS_PASSTHRU  0x00000004
#define BIDI_ATTR_SYM_SWAP_OFF       0x00000008
#define BIDI_ATTR_WORD_BREAK_ON      0x00000010
#define BIDI_ATTR_DISPLAY_SHAPED     0x00000020
#define BIDI_ATTR_SCREEN_FIELD_REV   0x00000040
#define BIDI_ATTR_STATUS_INDICATOR   0x00000080

/* layout_object attribute names (X/Open layout API) */
#define LAYOUT_TEXTTYPE_VISUAL       0
#define LAYOUT_TEXTTYPE_IMPLICIT     1
#define LAYOUT_ORIENTATION_LTR       2
#define LAYOUT_ORIENTATION_RTL       3
#define LAYOUT_ORIENTATION_CONTEXT   4
#define LAYOUT_NUMERALS_NOMINAL      5
#define LAYOUT_NUMERALS_PASSTHRU     6
#define LAYOUT_SYM_SWAP_OFF          7
#define LAYOUT_WORD_BREAK_ON         8
#define LAYOUT_DISPLAY_SHAPED        9

/* layout_object attribute value pair */
#pragma pack(1)
typedef struct _LayoutValueRec {
    int   name;     /* LAYOUT_* constant              */
    PVOID value;    /* pointer to ULONG holding value */
} LayoutValueRec;
typedef LayoutValueRec *LayoutValues;
#pragma pack()

/* layout_object opaque handle */
typedef PVOID LAYOUT_OBJECT;
typedef LAYOUT_OBJECT *PLAYOUT_OBJECT;

/* ------------------------------------------------------------------ */
/* 32-bit NLS bidi functions (primary API)                            */
/* ------------------------------------------------------------------ */

/*
 * NlsConvertBidiString  (ordinal 15)
 *
 * Convert a string between visual and implicit bidi ordering.
 * flFlags: NLS_BIDI_VISUAL (visual->implicit) or 0 (implicit->visual).
 *          NLS_BIDI_LTR / NLS_BIDI_RTL to override paragraph direction.
 * ulCp:    codepage (0 = use process CP).
 */
APIRET APIENTRY NlsConvertBidiString( PSZ    pszSrc,
                                       ULONG  cbSrc,
                                       ULONG  flFlags,
                                       PSZ    pszDst,
                                       PULONG pcbDst,
                                       ULONG  ulCp );              /* 15 */

/*
 * NlsConvertBidiNumerics  (ordinal 14)
 *
 * Convert numeral digits between Arabic-Indic and Western forms.
 */
APIRET APIENTRY NlsConvertBidiNumerics( PSZ    pszSrc,
                                         ULONG  cbSrc,
                                         ULONG  flFlags,
                                         PSZ    pszDst,
                                         PULONG pcbDst,
                                         ULONG  ulCp );            /* 14 */

/*
 * NlsEditShape  (ordinal 22)
 *
 * Apply Arabic character contextual shaping (presentation forms).
 * Replaces base Arabic letters with isolated/initial/medial/final forms.
 */
APIRET APIENTRY NlsEditShape( PSZ    pszSrc,
                               ULONG  cbSrc,
                               ULONG  flFlags,
                               PSZ    pszDst,
                               PULONG pcbDst,
                               ULONG  ulCp );                      /* 22 */

/*
 * NlsInverseString  (ordinal 20)
 *
 * Reverse the byte order of a string (low-level primitive).
 */
APIRET APIENTRY NlsInverseString( PSZ    pszSrc,
                                   ULONG  cbSrc,
                                   PSZ    pszDst,
                                   PULONG pcbDst );                /* 20 */

/*
 * NlsShapeBidiString  (ordinal 23)
 *
 * Combined shape + reorder operation for display.
 * Equivalent to NlsEditShape followed by NlsConvertBidiString.
 */
APIRET APIENTRY NlsShapeBidiString( PSZ    pszSrc,
                                     ULONG  cbSrc,
                                     ULONG  flFlags,
                                     PSZ    pszDst,
                                     PULONG pcbDst,
                                     ULONG  ulCp );                /* 23 */

/* ------------------------------------------------------------------ */
/* 16-bit wrappers (same signatures, UPPERCASE names)                 */
/* ------------------------------------------------------------------ */
APIRET APIENTRY NLS_16CONVERTBIDISTRING( PSZ s, ULONG cbS, ULONG fl,
                                          PSZ d, PULONG pcb, ULONG cp );  /* 2  */
APIRET APIENTRY NLS_16CONVERTBIDINUMERICS( PSZ s, ULONG cbS, ULONG fl,
                                            PSZ d, PULONG pcb, ULONG cp );/* 1  */
APIRET APIENTRY NLS_16EDITSHAPE( PSZ s, ULONG cbS, ULONG fl,
                                  PSZ d, PULONG pcb, ULONG cp );          /* 9  */
APIRET APIENTRY NLS_16INVERSESTRING( PSZ s, ULONG cbS,
                                      PSZ d, PULONG pcb );                /* 7  */
APIRET APIENTRY NLS_16SHAPEBIDISTRING( PSZ s, ULONG cbS, ULONG fl,
                                        PSZ d, PULONG pcb, ULONG cp );    /* 10 */

/* ------------------------------------------------------------------ */
/* Bidi attribute conversion  (ordinal 50)                            */
/* ------------------------------------------------------------------ */

/*
 * RC_16BIDIATTR  (ordinal 50)
 *
 * Convert between 16-bit bidi attribute word and 32-bit BIDI_ATTR_* flags.
 * fTo32 = TRUE:  convert ulAttr16 -> *pulAttr32
 * fTo32 = FALSE: convert *pulAttr32 -> 16-bit result stored in *pulAttr32
 */
APIRET APIENTRY RC_16BIDIATTR( USHORT ulAttr16,
                                PULONG pulAttr32,
                                BOOL   fTo32 );                    /* 50 */

/* ------------------------------------------------------------------ */
/* Round-trip mode  (ordinal 100)                                     */
/* ------------------------------------------------------------------ */

/*
 * Set_Round_Trip  (ordinal 100)
 *
 * Enable/disable lossless round-trip preservation for bidi conversions.
 * Returns the previous setting (TRUE = was enabled).
 */
BOOL APIENTRY Set_Round_Trip( BOOL fEnable );                      /* 100 */

/* ------------------------------------------------------------------ */
/* layout_object API  (ordinals 206-211)                              */
/* ------------------------------------------------------------------ */

/*
 * layout_32_object_create  (ordinal 206)
 * Allocate a new layout object. *pLayoutObj receives the handle.
 * ulExtra is reserved (pass 0). Returns 0 on success, -1 on failure.
 */
int APIENTRY layout_32_object_create( ULONG         ulExtra,
                                       LAYOUT_OBJECT *pLayoutObj );  /* 206 */

/*
 * layout_32_object_destroy  (ordinal 207)
 * Free a layout object. Returns 0 on success, -1 on bad handle.
 */
int APIENTRY layout_32_object_destroy( LAYOUT_OBJECT layoutObj );    /* 207 */

/*
 * layout_32_object_transform  (ordinal 208)
 * Apply bidi transformation; input in pInValues, output in pOutValues.
 * *pFlag receives status flags.
 */
int APIENTRY layout_32_object_transform( LAYOUT_OBJECT  layoutObj,
                                          LayoutValues   pInValues,
                                          LayoutValues   pOutValues,
                                          int           *pFlag );    /* 208 */

/*
 * layout_32_object_editshape  (ordinal 209)
 * Apply contextual shaping within the layout object context.
 */
int APIENTRY layout_32_object_editshape( LAYOUT_OBJECT  layoutObj,
                                          ULONG          cbSrc,
                                          PSZ            pszSrc,
                                          ULONG         *pcbDst,
                                          PSZ            pszDst );   /* 209 */

/*
 * layout_32_object_setvalues  (ordinal 210)
 * Set layout attributes. Array terminated by {0,NULL}.
 * *pIndex = index of first failure, or -1 on full success.
 */
int APIENTRY layout_32_object_setvalues( LAYOUT_OBJECT layoutObj,
                                          LayoutValues  pValues,
                                          int          *pIndex );    /* 210 */

/*
 * layout_32_object_getvalues  (ordinal 211)
 * Query layout attributes. Same convention as setvalues.
 */
int APIENTRY layout_32_object_getvalues( LAYOUT_OBJECT layoutObj,
                                          LayoutValues  pValues,
                                          int          *pIndex );    /* 211 */

#ifdef __cplusplus
}
#endif

#endif /* BDCALL32_H_INCLUDED */
