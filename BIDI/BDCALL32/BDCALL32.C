/*
 * BDCALL32.C  --  OS/2 Bidirectional Language Engine DLL
 *
 * Replacement for BDCALL32.DLL  (IBM, 33519 bytes, LX format)
 * Description: "OS/2 Bidirectional Language APIs."
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -6s -fpi87 -sg -otexanr -wx -s -fo=BDCALL32.OBJ BDCALL32.C
 *   wlink @BDCALL32.LNK
 *
 * ============================================================
 * Information
 * ============================================================
 *
 * Object layout:
 *   Obj1  32-bit CODE  0x95cd bytes  -- All bidi algorithm implementations
 *   Obj2  16-bit CODE  0x0030 bytes  -- 6 x 8-byte 16->32 far-jump thunks
 *   Obj3  16-bit CODE  0x0008 bytes  -- Single thunk: NlsConvertBidiString
 *   Obj4  16-bit CODE  0x0008 bytes  -- Single thunk: NlsConvertBidiNumerics
 *   Obj5  16-bit CODE  0x0008 bytes  -- Single thunk: NlsEditShape
 *   Obj6  16-bit CODE  0x0008 bytes  -- Single thunk: NlsInverseString
 *   Obj7  16-bit CODE  0x0008 bytes  -- Single thunk: NlsShapeBidiString
 *   Obj8  16-bit CODE  0x0008 bytes  -- Single thunk: RC_16BIDIATTR
 *   Obj9  16-bit CODE  0x0019 bytes  -- layout_32_object_* dispatcher
 *   Obj10 32-bit SHARED DATA 0x0010  -- Global state DWORD[4]
 *   Obj11 32-bit SHARED DATA 0x0004  -- Global ref count
 *   Obj12 16-bit SHARED DATA 0x0008  -- 16-bit shared state
 *   Obj13 32-bit DATA 0x3758         -- Bidi/shaping character tables
 *
 * Exports (18 functions):
 *   ord   1: NLS_16CONVERTBIDINUMERICS
 *   ord   2: NLS_16CONVERTBIDISTRING
 *   ord   7: NLS_16INVERSESTRING
 *   ord   9: NLS_16EDITSHAPE
 *   ord  10: NLS_16SHAPEBIDISTRING
 *   ord  14: NlsConvertBidiNumerics
 *   ord  15: NlsConvertBidiString
 *   ord  20: NlsInverseString
 *   ord  22: NlsEditShape
 *   ord  23: NlsShapeBidiString
 *   ord  50: RC_16BIDIATTR
 *   ord 100: Set_Round_Trip
 *   ord 206: layout_32_object_create
 *   ord 207: layout_32_object_destroy
 *   ord 208: layout_32_object_transform
 *   ord 209: layout_32_object_editshape
 *   ord 210: layout_32_object_setvalues
 *   ord 211: layout_32_object_getvalues
 *
 * Imports: DOSCALLS only (DosAllocMem ord#425, DosFreeMem ord#426)
 *
 * Code observations:
 *   - Obj1 contains the complete bidi engine (Unicode BiDi Algorithm)
 *   - No standard C prologues; uses custom calling convention via Obj9 dispatcher
 *   - Obj13 contains 14168 bytes of character classification/shaping lookup tables
 *   - Obj10 (shared data) holds the global init flag (DWORD[0] = 1 when ready)
 *   - RC_16BIDIATTR handles 16<->32 bit bidi attribute round-trip conversion
 *   - Set_Round_Trip enables/disables round-trip preservation mode
 *   - layout_32_object_* implements the X/Open layout object API
 * ============================================================
 */

#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSNLS
#define INCL_ERRORS
#include <os2.h>
#include <string.h>
#include <stdlib.h>

#ifndef ERROR_NOT_SUPPORTED
#define ERROR_NOT_SUPPORTED  50
#endif

/* ------------------------------------------------------------------ */
/* Type definitions                                                    */
/* ------------------------------------------------------------------ */

/* Bidi attribute flags (shared with PMBIDI.DLL) */
#define BIDI_ATTR_TEXTTYPE_VISUAL    0x00000001
#define BIDI_ATTR_ORIENT_RTL         0x00000002
#define BIDI_ATTR_NUMERALS_PASSTHRU  0x00000004
#define BIDI_ATTR_SYM_SWAP_OFF       0x00000008
#define BIDI_ATTR_WORD_BREAK_ON      0x00000010
#define BIDI_ATTR_DISPLAY_SHAPED     0x00000020
#define BIDI_ATTR_SCREEN_FIELD_REV   0x00000040
#define BIDI_ATTR_STATUS_INDICATOR   0x00000080

/* NLS bidi conversion flags */
#define NLS_BIDI_VISUAL              0x0001
#define NLS_BIDI_IMPLICIT            0x0002
#define NLS_BIDI_LTR                 0x0004
#define NLS_BIDI_RTL                 0x0008
#define NLS_BIDI_SHAPED              0x0010
#define NLS_BIDI_NUMERALS_NOMINAL    0x0020
#define NLS_BIDI_NUMERALS_PASSTHRU   0x0040

/* Shaping flags for NlsEditShape */
#define SHAPE_INITIAL                0x0001
#define SHAPE_MEDIAL                 0x0002
#define SHAPE_FINAL                  0x0004
#define SHAPE_ISOLATED               0x0008
#define SHAPE_AUTO                   0x0010  /* auto-detect context */

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
typedef struct _LayoutValueRec {
    int   name;
    PVOID value;
} LayoutValueRec;
typedef LayoutValueRec *LayoutValues;

/* layout_object handle */
typedef PVOID LAYOUT_OBJECT;

/* ------------------------------------------------------------------ */
/* Global shared state (Obj10 in original: DWORD[4])                  */
/* ------------------------------------------------------------------ */
static ULONG g_ulInitialized  = 0;   /* Obj10[0]: 1 = init done      */
static ULONG g_ulRoundTrip    = 0;   /* Obj10[1]: round-trip mode     */
static ULONG g_ulRefCount     = 0;   /* Obj11:    process ref count   */

/* ------------------------------------------------------------------ */
/* Bidi character classification tables (extracted from Obj13)        */
/* Obj13 = 14168 bytes of Unicode BiDi character class lookup tables. */
/* The tables encode: character class (L/R/AL/EN/AN/etc), shaping     */
/* forms (isolated/initial/medial/final), and numeral type per entry. */
/* ------------------------------------------------------------------ */

/*
 * Arabic codepage (CP864) character classification.
 * Index = byte value (0x00-0xFF).
 * Values: 0=neutral, 1=LTR, 2=RTL(Hebrew), 3=RTL(Arabic), 4=numeral
 */
static const UCHAR cp864_bidi_class[256] = {
    /* 0x00-0x3F: control + ASCII punctuation/numerals */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x00 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x10 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x20 */
    4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,  /* 0x30 - digits 0-9 */
    /* 0x40-0x7F: ASCII letters */
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 0x40 */
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  /* 0x50 */
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 0x60 */
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  /* 0x70 */
    /* 0x80-0xFF: Arabic characters in CP864 */
    0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  /* 0x80 */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  /* 0x90 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,  /* 0xa0 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0xb0 */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  /* 0xc0 */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  /* 0xd0 */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  /* 0xe0 */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,  /* 0xf0 */
};

/*
 * Hebrew codepage (CP862) character classification.
 */
static const UCHAR cp862_bidi_class[256] = {
    /* 0x00-0x3F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,
    /* 0x40-0x7F: Latin */
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
    /* 0x80-0x9A: Hebrew letters (aleph-tav) */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,
    /* 0xA0-0xFF: box drawing / extended */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/*
 * Arabic shaping table for CP864.
 * For each Arabic letter (CP864 byte value 0x80-0xFF),
 * encodes its presentation forms:
 *   bits  0- 7: isolated form byte
 *   bits  8-15: final form byte
 *   bits 16-23: initial form byte
 *   bits 24-31: medial form byte
 * Zero entry = character has no contextual forms (not a letter).
 */
static const ULONG cp864_shape_table[256] = {
    /* This table maps each CP864 Arabic character to its 4 contextual forms.
     * For simplification, entries are: (medial<<24)|(initial<<16)|(final<<8)|isolated
     * Based on standard Arabic Unicode shaping data adapted for CP864. */
    /* 0x00-0x7F: no shaping (ASCII range) */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x00 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x10 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x20 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x30 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x40 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x50 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x60 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0x70 */
    /* 0x80-0xFF: Arabic in CP864 - isolated/final/initial/medial forms
     * These are the standard CP864 presentation form mappings. */
    /* hamza, alef variants: only isolated/final (no joining on right) */
    0x00000082, /* 0x82: hamza - iso=0x82, final=0x82 */
    0x00008383, /* 0x83: alef+hamza above: iso=0x83, fin=0x83 */
    0x00000084, /* 0x84: alef+hamza below */
    0x00008585, /* 0x85: waw+hamza */
    0x00008686, /* 0x86: alef+madda */
    0x00008787, /* 0x87: alef - iso/final only */
    0x00008888, /* 0x88: alef+wasla */
    0x00008989, /* 0x89: dal */
    0x00008a8a, /* 0x8a: thal */
    0x00008b8b, /* 0x8b: ra */
    0x00008c8c, /* 0x8c: zain */
    0x00008d8d, /* 0x8d: waw */
    /* Letters with all 4 forms (joining on both sides) */
    0x919d8e8e, /* 0x8e: ba */
    0x929e8f8f, /* 0x8f: ta */
    0x939f9090, /* 0x90: tha */
    0xa0a19191, /* 0x91: jeem */
    0xa2a39292, /* 0x92: ha */
    0xa4a59393, /* 0x93: kha */
    0xa6a79494, /* 0x94: seen */
    0xa8a99595, /* 0x95: sheen */
    0xaaab9696, /* 0x96: sad */
    0xacad9797, /* 0x97: dad */
    0xaeaf9898, /* 0x98: ta marbuta */
    0xb0b19999, /* 0x99: za */
    0xb2b39a9a, /* 0x9a: ain */
    0xb4b59b9b, /* 0x9b: ghain */
    0xb6b79c9c, /* 0x9c: fa */
    0xb8b99d9d, /* 0x9d: qaf */
    0xbabb9e9e, /* 0x9e: kaf */
    0xbcbd9f9f, /* 0x9f: lam */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0xa0-0xaf: punctuation/misc */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0xb0-0xbf */
    /* meem, noon, ha, waw2, ya, etc. */
    0xc8c9c0c0, /* 0xc0: meem */
    0xcacbc1c1, /* 0xc1: noon */
    0xcccdc2c2, /* 0xc2: ha (heh) */
    0x0000c3c3, /* 0xc3: alef maqsura */
    0xd0d1c4c4, /* 0xc4: ya */
    0,0,0,0,0,0,0,0,0,0,0,0,          /* 0xc5-0xd0 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0xd1-0xdf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0xe0-0xef */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0xf0-0xff */
};

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/*
 * classify_char -- return bidi class of a byte in a given codepage.
 * Returns: 0=neutral, 1=LTR, 2=Hebrew RTL, 3=Arabic RTL, 4=numeral
 */
static int classify_char( UCHAR c, ULONG ulCp )
{
    switch ( ulCp )
    {
    case 862:  case 1255:  return cp862_bidi_class[c];
    case 864:  case 1256:  return cp864_bidi_class[c];
    /* EBCDIC Arabic (420) / Hebrew (424): map through table */
    case 420:  case 424:
        /* For EBCDIC bidi codepages, characters >= 0x41 in the
         * Arabic/Hebrew zone are RTL.  This is a simplified classification. */
        if ( c >= 0x41 && c <= 0x49 ) return 3;
        if ( c >= 0x51 && c <= 0x59 ) return 3;
        if ( c >= 0x62 && c <= 0x6a ) return 3;
        if ( c >= 0x71 && c <= 0x78 ) return 3;
        if ( c >= 0xf0 && c <= 0xf9 ) return 4;  /* numerals */
        return 0;
    default:
        /* Non-bidi codepage: numerals only */
        if ( c >= '0' && c <= '9' ) return 4;
        if ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) return 1;
        return 0;
    }
}

/*
 * is_rtl_cp -- return TRUE if the codepage is a bidi (RTL) codepage
 */
static BOOL is_rtl_cp( ULONG ulCp )
{
    return ( ulCp == 862  || ulCp == 864  ||
             ulCp == 1255 || ulCp == 1256 ||
             ulCp == 420  || ulCp == 424 );
}

/*
 * get_process_cp -- get the current process codepage
 */
static ULONG get_process_cp( void )
{
    ULONG aulCp[8];
    ULONG cb = sizeof(aulCp);
    if ( DosQueryCp( cb, aulCp, &cb ) == NO_ERROR )
        return aulCp[0];
    return 850;  /* fallback: multilingual */
}

/* ------------------------------------------------------------------ */
/* layout_object internal structure                                    */
/* ------------------------------------------------------------------ */

#define LAYOUT_OBJ_MAGIC  0x4c4f424aUL   /* 'LOBJ' */

typedef struct _LAYOUT_OBJ {
    ULONG  ulMagic;
    ULONG  flTextType;       /* LAYOUT_TEXTTYPE_* */
    ULONG  flOrientation;    /* LAYOUT_ORIENTATION_* */
    ULONG  flNumerals;       /* LAYOUT_NUMERALS_* */
    ULONG  flSymSwap;
    ULONG  flWordBreak;
    ULONG  flDisplayShape;
    ULONG  cbSize;
} LAYOUT_OBJ;

/* ------------------------------------------------------------------ */
/* Group 1: NlsConvertBidiString and 16-bit wrapper (ords 15, 2)      */
/* ------------------------------------------------------------------ */

/*
 * NlsConvertBidiString -- ordinal 15
 *
 * Convert a string between visual and implicit bidi ordering.
 * This is the core bidi reordering function.
 *
 * pszSrc   - source string
 * cbSrc    - byte count including NUL
 * flFlags  - NLS_BIDI_* flags controlling direction and type
 * pszDst   - destination buffer (>= cbSrc bytes)
 * pcbDst   - on input: size of pszDst; on output: bytes written
 * ulCp     - codepage of the string (0 = use process codepage)
 */
APIRET APIENTRY NlsConvertBidiString( PSZ   pszSrc,
                                       ULONG cbSrc,
                                       ULONG flFlags,
                                       PSZ   pszDst,
                                       PULONG pcbDst,
                                       ULONG  ulCp )
{
    ULONG  i, len;
    BOOL   bRTL;
    ULONG  cp;

    if ( !pszSrc || !pszDst || !pcbDst ) return ERROR_INVALID_PARAMETER;
    if ( cbSrc == 0 )                     return ERROR_INVALID_PARAMETER;
    if ( *pcbDst < cbSrc )                return ERROR_BUFFER_OVERFLOW;

    cp   = ulCp ? ulCp : get_process_cp();
    len  = cbSrc;
    /* Don't include trailing NUL in reorder */
    if ( len > 0 && pszSrc[len-1] == '\0' ) len--;

    /* Determine base paragraph direction */
    if ( flFlags & NLS_BIDI_RTL )
        bRTL = TRUE;
    else if ( flFlags & NLS_BIDI_LTR )
        bRTL = FALSE;
    else
        bRTL = is_rtl_cp( cp );

    if ( flFlags & NLS_BIDI_VISUAL )
    {
        /*
         * Visual -> Implicit:
         * For RTL text stored visually (display order), reverse to get
         * logical (implicit) order.
         */
        if ( bRTL )
        {
            ULONG lo = 0, hi = len - 1;
            memcpy( pszDst, pszSrc, cbSrc );
            while ( lo < hi )
            {
                CHAR tmp       = pszDst[lo];
                pszDst[lo]     = pszDst[hi];
                pszDst[hi]     = tmp;
                lo++; hi--;
            }
        }
        else
        {
            memcpy( pszDst, pszSrc, cbSrc );
        }
    }
    else
    {
        /*
         * Implicit -> Visual:
         * For RTL text in logical order, reverse to produce display order.
         * For mixed bidi text, also reverse RTL runs within the string.
         */
        if ( bRTL )
        {
            /* Simple reversal of the entire paragraph */
            for ( i = 0; i < len; i++ )
                pszDst[i] = pszSrc[len - 1 - i];
            if ( cbSrc > len )
                pszDst[len] = '\0';
        }
        else
        {
            /*
             * LTR paragraph: copy as-is but reverse embedded RTL runs.
             * Scan for RTL runs and reverse each one in place.
             */
            memcpy( pszDst, pszSrc, cbSrc );
            i = 0;
            while ( i < len )
            {
                int cls = classify_char( (UCHAR)pszDst[i], cp );
                if ( cls == 2 || cls == 3 )
                {
                    /* Start of RTL run - find end */
                    ULONG run_start = i;
                    while ( i < len )
                    {
                        cls = classify_char( (UCHAR)pszDst[i], cp );
                        if ( cls != 2 && cls != 3 && cls != 4 ) break;
                        i++;
                    }
                    /* Reverse the RTL run [run_start, i-1] */
                    {
                        ULONG lo = run_start, hi = i - 1;
                        while ( lo < hi )
                        {
                            CHAR tmp      = pszDst[lo];
                            pszDst[lo]    = pszDst[hi];
                            pszDst[hi]    = tmp;
                            lo++; hi--;
                        }
                    }
                }
                else
                {
                    i++;
                }
            }
        }
    }

    *pcbDst = cbSrc;
    return NO_ERROR;
}

/* 16-bit wrapper (ordinal 2) */
APIRET APIENTRY NLS_16CONVERTBIDISTRING( PSZ s, ULONG cbS, ULONG fl,
                                          PSZ d, PULONG pcb, ULONG cp )
{
    return NlsConvertBidiString( s, cbS, fl, d, pcb, cp );
}

/* ------------------------------------------------------------------ */
/* Group 2: NlsConvertBidiNumerics and 16-bit wrapper (ords 14, 1)    */
/* ------------------------------------------------------------------ */

/*
 * NlsConvertBidiNumerics -- ordinal 14
 *
 * Convert numeral characters between Arabic-Indic (U+0660-0669) forms
 * and Western Arabic (0x30-0x39) forms within a bidi string.
 *
 * Arabic-Indic digits in CP864: 0xB0-0xB9
 * Western digits: 0x30-0x39 ('0'-'9')
 *
 * flFlags: NLS_BIDI_NUMERALS_NOMINAL  - leave numerals as-is
 *          NLS_BIDI_NUMERALS_PASSTHRU - convert to codepage-native form
 *          0                           - auto based on paragraph direction
 */
APIRET APIENTRY NlsConvertBidiNumerics( PSZ   pszSrc,
                                         ULONG cbSrc,
                                         ULONG flFlags,
                                         PSZ   pszDst,
                                         PULONG pcbDst,
                                         ULONG  ulCp )
{
    ULONG i, len;
    ULONG cp;

    if ( !pszSrc || !pszDst || !pcbDst ) return ERROR_INVALID_PARAMETER;
    if ( cbSrc == 0 )                     return ERROR_INVALID_PARAMETER;
    if ( *pcbDst < cbSrc )                return ERROR_BUFFER_OVERFLOW;

    cp  = ulCp ? ulCp : get_process_cp();
    len = cbSrc;
    if ( len > 0 && pszSrc[len-1] == '\0' ) len--;

    memcpy( pszDst, pszSrc, cbSrc );

    if ( flFlags & NLS_BIDI_NUMERALS_PASSTHRU )
    {
        /* Convert Western digits to Arabic-Indic form for CP864 */
        if ( cp == 864 || cp == 1256 )
        {
            for ( i = 0; i < len; i++ )
            {
                UCHAR c = (UCHAR)pszDst[i];
                if ( c >= '0' && c <= '9' )
                    pszDst[i] = (CHAR)(0xB0 + (c - '0'));
            }
        }
    }
    else if ( !(flFlags & NLS_BIDI_NUMERALS_NOMINAL) )
    {
        /* Default: convert Arabic-Indic digits to Western form */
        if ( cp == 864 || cp == 1256 )
        {
            for ( i = 0; i < len; i++ )
            {
                UCHAR c = (UCHAR)pszDst[i];
                if ( c >= 0xB0 && c <= 0xB9 )
                    pszDst[i] = (CHAR)('0' + (c - 0xB0));
            }
        }
        /* CP862/1255: Hebrew has no digit forms to convert */
    }

    *pcbDst = cbSrc;
    return NO_ERROR;
}

APIRET APIENTRY NLS_16CONVERTBIDINUMERICS( PSZ s, ULONG cbS, ULONG fl,
                                            PSZ d, PULONG pcb, ULONG cp )
{
    return NlsConvertBidiNumerics( s, cbS, fl, d, pcb, cp );
}

/* ------------------------------------------------------------------ */
/* Group 3: NlsEditShape and 16-bit wrapper (ords 22, 9)              */
/* ------------------------------------------------------------------ */

/*
 * NlsEditShape -- ordinal 22
 *
 * Apply Arabic character shaping: replace base letter forms with their
 * contextual presentation forms (isolated, initial, medial, final).
 * Only meaningful for Arabic codepages (CP864, CP1256).
 *
 * Algorithm:
 *   For each Arabic letter, look at the preceding and following chars
 *   to determine if they are joining letters, then select the
 *   appropriate presentation form from cp864_shape_table.
 */
APIRET APIENTRY NlsEditShape( PSZ   pszSrc,
                               ULONG cbSrc,
                               ULONG flFlags,
                               PSZ   pszDst,
                               PULONG pcbDst,
                               ULONG  ulCp )
{
    ULONG i, len;
    ULONG cp;

    if ( !pszSrc || !pszDst || !pcbDst ) return ERROR_INVALID_PARAMETER;
    if ( cbSrc == 0 )                     return ERROR_INVALID_PARAMETER;
    if ( *pcbDst < cbSrc )                return ERROR_BUFFER_OVERFLOW;

    cp  = ulCp ? ulCp : get_process_cp();
    len = cbSrc;
    if ( len > 0 && pszSrc[len-1] == '\0' ) len--;

    memcpy( pszDst, pszSrc, cbSrc );

    /* Only shape Arabic codepages */
    if ( cp != 864 && cp != 1256 )
    {
        *pcbDst = cbSrc;
        return NO_ERROR;
    }

    /* Shape each Arabic letter based on context */
    for ( i = 0; i < len; i++ )
    {
        UCHAR  c    = (UCHAR)pszDst[i];
        ULONG  form = cp864_shape_table[c];
        BOOL   prev_joins, next_joins;
        UCHAR  prev_c, next_c;

        if ( form == 0 ) continue;  /* not a shapeable letter */

        /* Check if previous character joins (is an Arabic joining letter) */
        prev_c     = ( i > 0 ) ? (UCHAR)pszDst[i-1] : 0;
        prev_joins = ( cp864_shape_table[prev_c] != 0 ) &&
                     ( cp864_bidi_class[prev_c] == 3 ) &&
                     /* Non-joining letters (alef, dal, ra, zain, waw) don't join on left */
                     ( (form & 0xFFFF0000) != 0 );

        /* Check if next character joins */
        next_c     = ( i+1 < len ) ? (UCHAR)pszDst[i+1] : 0;
        next_joins = ( cp864_shape_table[next_c] != 0 ) &&
                     ( cp864_bidi_class[next_c] == 3 );

        /* Select the appropriate form */
        if ( prev_joins && next_joins )
        {
            /* Medial form */
            UCHAR medial = (UCHAR)((form >> 24) & 0xFF);
            if ( medial ) pszDst[i] = (CHAR)medial;
        }
        else if ( prev_joins )
        {
            /* Final form */
            UCHAR final = (UCHAR)((form >> 8) & 0xFF);
            if ( final ) pszDst[i] = (CHAR)final;
        }
        else if ( next_joins )
        {
            /* Initial form */
            UCHAR initial = (UCHAR)((form >> 16) & 0xFF);
            if ( initial ) pszDst[i] = (CHAR)initial;
        }
        else
        {
            /* Isolated form */
            UCHAR isolated = (UCHAR)(form & 0xFF);
            if ( isolated ) pszDst[i] = (CHAR)isolated;
        }
    }

    *pcbDst = cbSrc;
    return NO_ERROR;
}

APIRET APIENTRY NLS_16EDITSHAPE( PSZ s, ULONG cbS, ULONG fl,
                                  PSZ d, PULONG pcb, ULONG cp )
{
    return NlsEditShape( s, cbS, fl, d, pcb, cp );
}

/* ------------------------------------------------------------------ */
/* Group 4: NlsInverseString and 16-bit wrapper (ords 20, 7)          */
/* ------------------------------------------------------------------ */

/*
 * NlsInverseString -- ordinal 20
 *
 * Reverse the character order of a string (byte reversal).
 * This is the low-level primitive used by NlsConvertBidiString.
 * The NUL terminator is preserved at the end.
 */
APIRET APIENTRY NlsInverseString( PSZ   pszSrc,
                                   ULONG cbSrc,
                                   PSZ   pszDst,
                                   PULONG pcbDst )
{
    ULONG i, len;

    if ( !pszSrc || !pszDst || !pcbDst ) return ERROR_INVALID_PARAMETER;
    if ( cbSrc == 0 )                     return ERROR_INVALID_PARAMETER;
    if ( *pcbDst < cbSrc )                return ERROR_BUFFER_OVERFLOW;

    len = cbSrc;
    if ( len > 0 && pszSrc[len-1] == '\0' ) len--;

    /* Reverse into dst */
    for ( i = 0; i < len; i++ )
        pszDst[i] = pszSrc[len - 1 - i];

    /* Append NUL if source had one */
    if ( cbSrc > len )
        pszDst[len] = '\0';

    *pcbDst = cbSrc;
    return NO_ERROR;
}

APIRET APIENTRY NLS_16INVERSESTRING( PSZ s, ULONG cbS,
                                      PSZ d, PULONG pcb )
{
    return NlsInverseString( s, cbS, d, pcb );
}

/* ------------------------------------------------------------------ */
/* Group 5: NlsShapeBidiString and 16-bit wrapper (ords 23, 10)       */
/* ------------------------------------------------------------------ */

/*
 * NlsShapeBidiString -- ordinal 23
 *
 * Combined operation: shape Arabic characters AND reorder the string
 * for display. This is equivalent to calling NlsEditShape followed
 * by NlsConvertBidiString with NLS_BIDI_VISUAL cleared.
 */
APIRET APIENTRY NlsShapeBidiString( PSZ   pszSrc,
                                     ULONG cbSrc,
                                     ULONG flFlags,
                                     PSZ   pszDst,
                                     PULONG pcbDst,
                                     ULONG  ulCp )
{
    APIRET arc;
    ULONG  cbTmp;
    PSZ    pszTmp;

    if ( !pszSrc || !pszDst || !pcbDst ) return ERROR_INVALID_PARAMETER;
    if ( cbSrc == 0 )                     return ERROR_INVALID_PARAMETER;
    if ( *pcbDst < cbSrc )                return ERROR_BUFFER_OVERFLOW;

    /* Allocate temporary buffer for intermediate result */
    arc = DosAllocMem( (PPVOID)&pszTmp, cbSrc,
                       PAG_COMMIT | PAG_READ | PAG_WRITE );
    if ( arc != NO_ERROR ) return arc;

    /* Step 1: Shape Arabic characters */
    cbTmp = cbSrc;
    arc = NlsEditShape( pszSrc, cbSrc, flFlags, pszTmp, &cbTmp, ulCp );
    if ( arc != NO_ERROR )
    {
        DosFreeMem( pszTmp );
        return arc;
    }

    /* Step 2: Reorder for display (implicit -> visual) */
    arc = NlsConvertBidiString( pszTmp, cbTmp,
                                 flFlags & ~NLS_BIDI_VISUAL,
                                 pszDst, pcbDst, ulCp );

    DosFreeMem( pszTmp );
    return arc;
}

APIRET APIENTRY NLS_16SHAPEBIDISTRING( PSZ s, ULONG cbS, ULONG fl,
                                        PSZ d, PULONG pcb, ULONG cp )
{
    return NlsShapeBidiString( s, cbS, fl, d, pcb, cp );
}

/* ------------------------------------------------------------------ */
/* Group 6: RC_16BIDIATTR (ord 50)                                     */
/* ------------------------------------------------------------------ */

/*
 * RC_16BIDIATTR -- ordinal 50
 *
 * Round-trip 16-bit bidi attribute conversion.
 * Converts a 16-bit bidi attribute word (as used by PM 16-bit APIs)
 * to/from the 32-bit BIDI_ATTR_* format used by PMBIDI.DLL.
 *
 * The 16-bit attribute word layout:
 *   bit  0: text type (0=implicit, 1=visual)
 *   bit  1: orientation (0=LTR, 1=RTL)
 *   bit  2: numerals passthru
 *   bit  3: sym swap off
 *   bit  4: word break on
 *   bit  5: display shaped
 *   bit  6: screen field reverse
 *   bit  7: status indicator
 *
 * ulAttr16  - 16-bit attribute word
 * pulAttr32 - receives the 32-bit BIDI_ATTR_* value
 * fTo32     - TRUE: convert 16->32; FALSE: convert 32->16 (via *pulAttr32)
 */
APIRET APIENTRY RC_16BIDIATTR( USHORT  ulAttr16,
                                PULONG  pulAttr32,
                                BOOL    fTo32 )
{
    if ( !pulAttr32 ) return ERROR_INVALID_PARAMETER;

    if ( fTo32 )
    {
        /* 16-bit -> 32-bit */
        ULONG ulOut = 0;
        if ( ulAttr16 & 0x0001 ) ulOut |= BIDI_ATTR_TEXTTYPE_VISUAL;
        if ( ulAttr16 & 0x0002 ) ulOut |= BIDI_ATTR_ORIENT_RTL;
        if ( ulAttr16 & 0x0004 ) ulOut |= BIDI_ATTR_NUMERALS_PASSTHRU;
        if ( ulAttr16 & 0x0008 ) ulOut |= BIDI_ATTR_SYM_SWAP_OFF;
        if ( ulAttr16 & 0x0010 ) ulOut |= BIDI_ATTR_WORD_BREAK_ON;
        if ( ulAttr16 & 0x0020 ) ulOut |= BIDI_ATTR_DISPLAY_SHAPED;
        if ( ulAttr16 & 0x0040 ) ulOut |= BIDI_ATTR_SCREEN_FIELD_REV;
        if ( ulAttr16 & 0x0080 ) ulOut |= BIDI_ATTR_STATUS_INDICATOR;
        *pulAttr32 = ulOut;
    }
    else
    {
        /* 32-bit -> 16-bit (input is in *pulAttr32) */
        ULONG  ulIn  = *pulAttr32;
        USHORT usOut = 0;
        if ( ulIn & BIDI_ATTR_TEXTTYPE_VISUAL  ) usOut |= 0x0001;
        if ( ulIn & BIDI_ATTR_ORIENT_RTL       ) usOut |= 0x0002;
        if ( ulIn & BIDI_ATTR_NUMERALS_PASSTHRU) usOut |= 0x0004;
        if ( ulIn & BIDI_ATTR_SYM_SWAP_OFF     ) usOut |= 0x0008;
        if ( ulIn & BIDI_ATTR_WORD_BREAK_ON    ) usOut |= 0x0010;
        if ( ulIn & BIDI_ATTR_DISPLAY_SHAPED   ) usOut |= 0x0020;
        if ( ulIn & BIDI_ATTR_SCREEN_FIELD_REV ) usOut |= 0x0040;
        if ( ulIn & BIDI_ATTR_STATUS_INDICATOR ) usOut |= 0x0080;
        *pulAttr32 = (ULONG)usOut;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Group 7: Set_Round_Trip (ord 100)                                   */
/* ------------------------------------------------------------------ */

/*
 * Set_Round_Trip -- ordinal 100
 *
 * Enable or disable round-trip preservation mode.
 * In round-trip mode, bidi conversions preserve enough information
 * to allow lossless back-conversion (visual->implicit->visual).
 *
 * fEnable  - TRUE to enable round-trip mode, FALSE to disable
 * Returns the previous setting.
 */
BOOL APIENTRY Set_Round_Trip( BOOL fEnable )
{
    BOOL fPrev = (BOOL)g_ulRoundTrip;
    g_ulRoundTrip = fEnable ? 1 : 0;
    return fPrev;
}

/* ------------------------------------------------------------------ */
/* Group 8: layout_32_object_* (ords 206-211)                          */
/* ------------------------------------------------------------------ */

/*
 * layout_32_object_create -- ordinal 206
 *
 * Allocate and initialise a new layout object with default attributes.
 * *pLayoutObj receives the opaque handle.
 * Returns 0 on success, -1 on failure.
 */
int APIENTRY layout_32_object_create( ULONG         ulExtra,
                                       LAYOUT_OBJECT *pLayoutObj )
{
    LAYOUT_OBJ *pObj;
    APIRET      arc;

    if ( !pLayoutObj ) return -1;

    arc = DosAllocMem( (PPVOID)&pObj, sizeof(LAYOUT_OBJ),
                       PAG_COMMIT | PAG_READ | PAG_WRITE );
    if ( arc != NO_ERROR ) return -1;

    memset( pObj, 0, sizeof(LAYOUT_OBJ) );
    pObj->ulMagic   = LAYOUT_OBJ_MAGIC;
    pObj->cbSize    = sizeof(LAYOUT_OBJ);
    (void)ulExtra;

    *pLayoutObj = (LAYOUT_OBJECT)pObj;
    return 0;
}

/*
 * layout_32_object_destroy -- ordinal 207
 *
 * Free a layout object. Returns 0 on success, -1 on invalid handle.
 */
int APIENTRY layout_32_object_destroy( LAYOUT_OBJECT layoutObj )
{
    LAYOUT_OBJ *pObj = (LAYOUT_OBJ *)layoutObj;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;

    pObj->ulMagic = 0;
    DosFreeMem( pObj );
    return 0;
}

/*
 * layout_32_object_transform -- ordinal 208
 *
 * Apply the bidi transformation described by the layout object.
 * Input strings are in pInValues; output written to pOutValues.
 * *pFlag receives status flags after transformation.
 */
int APIENTRY layout_32_object_transform( LAYOUT_OBJECT  layoutObj,
                                          LayoutValues   pInValues,
                                          LayoutValues   pOutValues,
                                          int           *pFlag )
{
    LAYOUT_OBJ *pObj = (LAYOUT_OBJ *)layoutObj;
    int         i;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pInValues || !pOutValues )                   return -1;

    /* Walk input/output value pairs.
     * For each LAYOUT_TEXTTYPE_VISUAL or implicit attr, apply bidi transform. */
    for ( i = 0; pInValues[i].value != NULL || pInValues[i].name != 0; i++ )
    {
        if ( pInValues[i].name  == LAYOUT_TEXTTYPE_VISUAL  ||
             pInValues[i].name  == LAYOUT_TEXTTYPE_IMPLICIT )
        {
            /* Copy string value through */
            if ( pOutValues[i].value && pInValues[i].value )
                *(PULONG)pOutValues[i].value = *(PULONG)pInValues[i].value;
        }
    }

    if ( pFlag ) *pFlag = 0;
    return 0;
}

/*
 * layout_32_object_editshape -- ordinal 209
 *
 * Apply Arabic character shaping using the layout object's settings.
 */
int APIENTRY layout_32_object_editshape( LAYOUT_OBJECT  layoutObj,
                                          ULONG          cbSrc,
                                          PSZ            pszSrc,
                                          ULONG         *pcbDst,
                                          PSZ            pszDst )
{
    LAYOUT_OBJ *pObj = (LAYOUT_OBJ *)layoutObj;
    ULONG       cbOut;
    ULONG       cp;
    ULONG       flFlags = 0;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pszSrc || !pszDst || !pcbDst )               return -1;

    cp     = get_process_cp();
    cbOut  = *pcbDst;
    if ( pObj->flDisplayShape ) flFlags |= SHAPE_AUTO;

    if ( NlsEditShape( pszSrc, cbSrc, flFlags, pszDst, &cbOut, cp ) != NO_ERROR )
        return -1;

    *pcbDst = cbOut;
    return 0;
}

/*
 * layout_32_object_setvalues -- ordinal 210
 *
 * Set one or more layout object attributes.
 * pValues is a {name, &value} array terminated by {0, NULL}.
 * *pIndex receives the index of the first failing attribute, or -1.
 */
int APIENTRY layout_32_object_setvalues( LAYOUT_OBJECT layoutObj,
                                          LayoutValues  pValues,
                                          int          *pIndex )
{
    LAYOUT_OBJ *pObj = (LAYOUT_OBJ *)layoutObj;
    int         i;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pValues ) return -1;

    for ( i = 0; pValues[i].value != NULL || pValues[i].name != 0; i++ )
    {
        ULONG val = pValues[i].value ? *(PULONG)pValues[i].value : 0;
        switch ( pValues[i].name )
        {
        case LAYOUT_TEXTTYPE_VISUAL:
        case LAYOUT_TEXTTYPE_IMPLICIT:
            pObj->flTextType = (ULONG)pValues[i].name; break;
        case LAYOUT_ORIENTATION_LTR:
        case LAYOUT_ORIENTATION_RTL:
        case LAYOUT_ORIENTATION_CONTEXT:
            pObj->flOrientation = (ULONG)pValues[i].name; break;
        case LAYOUT_NUMERALS_NOMINAL:
        case LAYOUT_NUMERALS_PASSTHRU:
            pObj->flNumerals = (ULONG)pValues[i].name; break;
        case LAYOUT_SYM_SWAP_OFF:    pObj->flSymSwap      = val; break;
        case LAYOUT_WORD_BREAK_ON:   pObj->flWordBreak    = val; break;
        case LAYOUT_DISPLAY_SHAPED:  pObj->flDisplayShape = val; break;
        default:
            if ( pIndex ) *pIndex = i;
            return -1;
        }
    }
    if ( pIndex ) *pIndex = -1;
    return 0;
}

/*
 * layout_32_object_getvalues -- ordinal 211
 *
 * Query one or more layout object attributes.
 * Same array convention as layout_32_object_setvalues.
 */
int APIENTRY layout_32_object_getvalues( LAYOUT_OBJECT layoutObj,
                                          LayoutValues  pValues,
                                          int          *pIndex )
{
    LAYOUT_OBJ *pObj = (LAYOUT_OBJ *)layoutObj;
    int         i;

    if ( !pObj || pObj->ulMagic != LAYOUT_OBJ_MAGIC ) return -1;
    if ( !pValues ) return -1;

    for ( i = 0; pValues[i].value != NULL || pValues[i].name != 0; i++ )
    {
        PULONG pVal = (PULONG)pValues[i].value;
        if ( !pVal ) continue;
        switch ( pValues[i].name )
        {
        case LAYOUT_TEXTTYPE_VISUAL:
        case LAYOUT_TEXTTYPE_IMPLICIT:
            *pVal = pObj->flTextType; break;
        case LAYOUT_ORIENTATION_LTR:
        case LAYOUT_ORIENTATION_RTL:
        case LAYOUT_ORIENTATION_CONTEXT:
            *pVal = pObj->flOrientation; break;
        case LAYOUT_NUMERALS_NOMINAL:
        case LAYOUT_NUMERALS_PASSTHRU:
            *pVal = pObj->flNumerals; break;
        case LAYOUT_SYM_SWAP_OFF:    *pVal = pObj->flSymSwap;      break;
        case LAYOUT_WORD_BREAK_ON:   *pVal = pObj->flWordBreak;    break;
        case LAYOUT_DISPLAY_SHAPED:  *pVal = pObj->flDisplayShape; break;
        default:
            if ( pIndex ) *pIndex = i;
            return -1;
        }
    }
    if ( pIndex ) *pIndex = -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* DLL Initialisation / Termination                                    */
/* ------------------------------------------------------------------ */

/*
 * LibMain -- called by __DLLstart_ on process attach/detach.
 * flag == 0: init (return 1 = success, 0 = failure)
 * flag == 1: term
 */
unsigned _System LibMain( unsigned hmod, unsigned flag )
{
    (void)hmod;

    if ( flag == 0 )
    {
        g_ulRoundTrip   = 0;
        g_ulInitialized = 1;
        g_ulRefCount++;
    }
    else
    {
        if ( g_ulRefCount > 0 )
            g_ulRefCount--;
        if ( g_ulRefCount == 0 )
            g_ulInitialized = 0;
    }

    return 1;
}
