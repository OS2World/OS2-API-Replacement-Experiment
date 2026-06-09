/*
 * IMP.C  -  OS/2 Input Method Profiler DLL replacement
 *
 * Original:  IMP.DLL  (LX format, OS/2, i386, 6681 bytes)
 * Module:    IMP  ("Input Method Profiler")
 * Ordinals:  1..6  (see IMP.DEF)
 *
 * Analysis of the original binary
 * --------------------------------
 * The DLL manages one OS/2 "Alternate Input Method" (AIM) profile stored
 * in the OS2.INI application "PM_AlternateInputMethods".  The profile
 * holds five ULONG values keyed by name:
 *
 *   Key name         Default   Meaning
 *   ──────────────── ───────   ──────────────────────────────────────────
 *   AIM_Active           0     Boolean – AIM enabled (non-zero) or off
 *   AIM_TimeOut        120     Idle timeout in ms (0 = no timeout)
 *   AIM_FKAccept         0     Filter-key accept count
 *   AIM_FKRate         500     Filter-key repeat rate  (ms)
 *   AIM_FKDelay       1000     Filter-key initial delay (ms)
 *
 * The DLL exports two 16-bit (APIENTRY16) and four 32-bit (APIENTRY) entry
 * points:
 *
 *   Ord  16/32  Name                    Action
 *   ───  ─────  ──────────────────────  ──────────────────────────────────────
 *    1   16     IMPSETAIMPROFILE        16-bit wrapper → calls IMP32SetAIMProfile
 *    2   32     IMP32SetAIMProfile      Write AIMProfile to OS2.INI
 *    3   16     IMPRESETAIMPROFILE      16-bit wrapper → calls IMP32ResetAIMProfile
 *    4   16     IMPQUERYAIMPROFILE      16-bit wrapper → calls IMP32QueryAIMProfile
 *    5   32     IMP32ResetAIMProfile    Reset profile to compiled-in defaults
 *    6   32     IMP32QueryAIMProfile    Read profile from OS2.INI
 *
 * All three "reset / query / set" functions work on a caller-supplied
 * AIMPROFILE structure:
 *
 *   typedef struct _AIMPROFILE {
 *       ULONG  cbSize;       // must be sizeof(AIMPROFILE) == 0x1c (28)
 *       ULONG  flReserved;   // must be 0
 *       ULONG  fAIMActive;   // AIM_Active
 *       ULONG  ulTimeOut;    // AIM_TimeOut
 *       ULONG  ulFKAccept;   // AIM_FKAccept
 *       ULONG  ulFKRate;     // AIM_FKRate
 *       ULONG  ulFKDelay;    // AIM_FKDelay
 *   } AIMPROFILE, *PAIMPROFILE;
 *
 * The original object 1 (16-bit code, 0x76c bytes) contains:
 *   • Three 16-bit APIENTRY16 thunks that validate the structure header
 *     and forward to the corresponding 32-bit function via a far call.
 *   • Shared helper routines for PrfQueryProfileData / PrfWriteProfileData.
 *
 * The original object 2 (32-bit code, 0x1d0 bytes) contains three 32-bit
 * functions (Set / Query / Reset) that call into PMSHAPI:
 *   PrfWriteProfileData  (PMSHAPI ord 0x86)
 *   PrfQueryProfileData  (PMSHAPI ord 0xee)
 *   (+ internal helpers)
 *
 * The original object 3 (16-bit stubs, 0x27 bytes) holds three call-gate
 * thunks used by objects 1 and 2 to switch between 16-bit and 32-bit
 * segments.
 *
 * Rebuild instructions (OpenWatcom for OS/2)
 * -------------------------------------------
 *   wcl386 -bt=os2 -bd -fo=IMP.OBJ IMP.C
 *   wlink @IMP.LNK
 *
 * See IMP.DEF and IMP.LNK for linker directives.
 *
 * Compiler: Open Watcom C/C++ 1.9 (or 2.0) targeting 32-bit OS/2 (flat)
 */

#define INCL_DOSPROCESS
#define INCL_WIN
#define INCL_BASE
#include <os2.h>

/* ──────────────────────────────────────────────────────────────────────────
 * Public types
 * ────────────────────────────────────────────────────────────────────────── */

#pragma pack(1)

typedef struct _AIMPROFILE {
    ULONG   cbSize;        /* sizeof(AIMPROFILE) must equal AIM_PROFILE_SIZE */
    ULONG   flReserved;    /* must be 0                                      */
    ULONG   fAIMActive;    /* 0 = inactive, non-zero = active                */
    ULONG   ulTimeOut;     /* idle timeout, milliseconds (0 = none)          */
    ULONG   ulFKAccept;    /* filter-key accept count                        */
    ULONG   ulFKRate;      /* filter-key repeat rate, ms                     */
    ULONG   ulFKDelay;     /* filter-key initial delay, ms                   */
} AIMPROFILE;
typedef AIMPROFILE *PAIMPROFILE;

#pragma pack()

#define AIM_PROFILE_SIZE   sizeof(AIMPROFILE)   /* 28 / 0x1c */

/* ──────────────────────────────────────────────────────────────────────────
 * INI application / key names  (from object 4 data section)
 * ────────────────────────────────────────────────────────────────────────── */

static const char szApp[]         = "PM_AlternateInputMethods";

/* Keys used for Set / Reset (write side, object 4 offset 0x19..0x8d) */
static const char szKeyActive[]   = "AIM_Active";
static const char szKeyTimeOut[]  = "AIM_TimeOut";
static const char szKeyFKAccept[] = "AIM_FKAccept";
static const char szKeyFKRate[]   = "AIM_FKRate";
static const char szKeyFKDelay[]  = "AIM_FKDelay";

/* ──────────────────────────────────────────────────────────────────────────
 * Default profile  (from object 4 binary tail at offset 0x90)
 *   AIM_Active  = 0x00000000  (off)
 *   AIM_TimeOut = 0x00000078  (120 ms)
 *   AIM_FKAccept= 0x00000000
 *   AIM_FKRate  = 0x000001f4  (500 ms)
 *   AIM_FKDelay = 0x000003e8  (1000 ms)
 * ────────────────────────────────────────────────────────────────────────── */

static const AIMPROFILE g_aimDefault = {
    AIM_PROFILE_SIZE,   /* cbSize     */
    0,                  /* flReserved */
    0,                  /* fAIMActive */
    120,                /* ulTimeOut  */
    0,                  /* ulFKAccept */
    500,                /* ulFKRate   */
    1000                /* ulFKDelay  */
};

/* ──────────────────────────────────────────────────────────────────────────
 * Forward declarations
 * ────────────────────────────────────────────────────────────────────────── */

APIRET APIENTRY IMP32SetAIMProfile  ( PAIMPROFILE pAIM );
APIRET APIENTRY IMP32ResetAIMProfile( PAIMPROFILE pAIM );
APIRET APIENTRY IMP32QueryAIMProfile( PAIMPROFILE pAIM );

/* ──────────────────────────────────────────────────────────────────────────
 * Internal helpers
 * ────────────────────────────────────────────────────────────────────────── */

/* Validate the caller-supplied AIMPROFILE header.
 * The original 16-bit code checks:
 *   es:[bx+0] == 0x1c  (cbSize)
 *   es:[bx+2] == 0x00  (flReserved high word)
 * and returns error 2 (ERROR_FILE_NOT_FOUND repurposed as bad-param) if wrong.
 * We use ERROR_INVALID_PARAMETER (0x57) which is more appropriate on 32-bit.
 */
static APIRET ValidateProfile( PAIMPROFILE pAIM )
{
    if ( !pAIM )
        return ERROR_INVALID_PARAMETER;
    if ( pAIM->cbSize     != AIM_PROFILE_SIZE )
        return ERROR_INVALID_PARAMETER;
    if ( pAIM->flReserved != 0 )
        return ERROR_INVALID_PARAMETER;
    return NO_ERROR;
}

/* Write a single ULONG key to HINI_USERPROFILE (OS2.INI).
 * Returns NO_ERROR on success, or ERROR_WRITE_FAULT on failure.
 */
static APIRET WriteULONG( const char *pszKey, ULONG ulValue )
{
    BOOL rc = PrfWriteProfileData( HINI_USERPROFILE,
                                   (PSZ)szApp,
                                   (PSZ)pszKey,
                                   &ulValue,
                                   sizeof(ULONG) );
    return rc ? NO_ERROR : ERROR_WRITE_FAULT;
}

/* Read a single ULONG key from HINI_USERPROFILE.
 * If the key is missing, *pulOut is left unchanged (caller pre-fills default).
 * Returns NO_ERROR (key found or missing), or ERROR_READ_FAULT on I/O error.
 */
static APIRET ReadULONG( const char *pszKey, ULONG *pulOut )
{
    ULONG  cbData = sizeof(ULONG);
    ULONG  ulTmp  = 0;
    BOOL   rc;

    rc = PrfQueryProfileData( HINI_USERPROFILE,
                              (PSZ)szApp,
                              (PSZ)pszKey,
                              &ulTmp,
                              &cbData );
    if ( rc && cbData == sizeof(ULONG) )
        *pulOut = ulTmp;
    /* if key absent, caller keeps its default – that is intentional */
    return NO_ERROR;
}

/* ──────────────────────────────────────────────────────────────────────────
 * IMP32SetAIMProfile  (ordinal 2)
 *
 * Write the five AIM profile values from *pAIM to OS2.INI.
 *
 * Parameters:
 *   pAIM   pointer to caller-filled AIMPROFILE
 *
 * Returns:
 *   NO_ERROR                 success
 *   ERROR_INVALID_PARAMETER  bad structure or null pointer
 *   ERROR_WRITE_FAULT        PrfWriteProfileData failed
 * ────────────────────────────────────────────────────────────────────────── */

APIRET APIENTRY IMP32SetAIMProfile( PAIMPROFILE pAIM )
{
    APIRET arc;

    arc = ValidateProfile( pAIM );
    if ( arc != NO_ERROR )
        return arc;

    arc = WriteULONG( szKeyActive,   pAIM->fAIMActive );  if (arc) return arc;
    arc = WriteULONG( szKeyTimeOut,  pAIM->ulTimeOut  );  if (arc) return arc;
    arc = WriteULONG( szKeyFKAccept, pAIM->ulFKAccept );  if (arc) return arc;
    arc = WriteULONG( szKeyFKRate,   pAIM->ulFKRate   );  if (arc) return arc;
    arc = WriteULONG( szKeyFKDelay,  pAIM->ulFKDelay  );

    return arc;
}

/* ──────────────────────────────────────────────────────────────────────────
 * IMP32ResetAIMProfile  (ordinal 5)
 *
 * Fill *pAIM with the compiled-in defaults AND write them to OS2.INI.
 * (The original binary resets both the in-memory structure and the INI.)
 *
 * Parameters:
 *   pAIM   pointer to AIMPROFILE that receives defaults
 *
 * Returns:
 *   NO_ERROR                 success
 *   ERROR_INVALID_PARAMETER  bad structure or null pointer
 *   ERROR_WRITE_FAULT        PrfWriteProfileData failed
 * ────────────────────────────────────────────────────────────────────────── */

APIRET APIENTRY IMP32ResetAIMProfile( PAIMPROFILE pAIM )
{
    APIRET arc;

    arc = ValidateProfile( pAIM );
    if ( arc != NO_ERROR )
        return arc;

    /* Copy defaults into caller's buffer */
    pAIM->fAIMActive = g_aimDefault.fAIMActive;
    pAIM->ulTimeOut  = g_aimDefault.ulTimeOut;
    pAIM->ulFKAccept = g_aimDefault.ulFKAccept;
    pAIM->ulFKRate   = g_aimDefault.ulFKRate;
    pAIM->ulFKDelay  = g_aimDefault.ulFKDelay;

    /* Persist to INI */
    arc = IMP32SetAIMProfile( pAIM );

    return arc;
}

/* ──────────────────────────────────────────────────────────────────────────
 * IMP32QueryAIMProfile  (ordinal 6)
 *
 * Read the five AIM profile values from OS2.INI into *pAIM.
 * Keys that are absent receive the compiled-in default.
 *
 * Parameters:
 *   pAIM   pointer to AIMPROFILE to receive values
 *
 * Returns:
 *   NO_ERROR                 success (missing keys get defaults)
 *   ERROR_INVALID_PARAMETER  bad structure or null pointer
 *   ERROR_READ_FAULT         PrfQueryProfileData I/O error
 * ────────────────────────────────────────────────────────────────────────── */

APIRET APIENTRY IMP32QueryAIMProfile( PAIMPROFILE pAIM )
{
    APIRET arc;

    arc = ValidateProfile( pAIM );
    if ( arc != NO_ERROR )
        return arc;

    /* Pre-fill with defaults so absent keys use them */
    pAIM->fAIMActive = g_aimDefault.fAIMActive;
    pAIM->ulTimeOut  = g_aimDefault.ulTimeOut;
    pAIM->ulFKAccept = g_aimDefault.ulFKAccept;
    pAIM->ulFKRate   = g_aimDefault.ulFKRate;
    pAIM->ulFKDelay  = g_aimDefault.ulFKDelay;

    arc = ReadULONG( szKeyActive,   &pAIM->fAIMActive );  if (arc) return arc;
    arc = ReadULONG( szKeyTimeOut,  &pAIM->ulTimeOut  );  if (arc) return arc;
    arc = ReadULONG( szKeyFKAccept, &pAIM->ulFKAccept );  if (arc) return arc;
    arc = ReadULONG( szKeyFKRate,   &pAIM->ulFKRate   );  if (arc) return arc;
    arc = ReadULONG( szKeyFKDelay,  &pAIM->ulFKDelay  );

    return arc;
}

/* ──────────────────────────────────────────────────────────────────────────
 * 16-bit thunks  (ordinals 1, 3, 4)
 *
 * The original DLL exported these as 16-bit APIENTRY16 (far 16:16) entry
 * points for compatibility with 16-bit PM callers.  Under a 32-bit rebuild
 * with OpenWatcom we map them as ordinary APIENTRY wrappers; the linker
 * script (IMP.DEF) marks them as RESIDENT NAME so the ordinals match.
 *
 * Prototype note: the original 16-bit calling convention passed the
 * AIMPROFILE pointer as a 16:16 far pointer (two WORDs on the stack).
 * 32-bit callers simply pass a flat NEAR pointer; these stubs accept that.
 * ────────────────────────────────────────────────────────────────────────── */

APIRET APIENTRY IMPSETAIMPROFILE( PAIMPROFILE pAIM )
{
    return IMP32SetAIMProfile( pAIM );
}

APIRET APIENTRY IMPRESETAIMPROFILE( PAIMPROFILE pAIM )
{
    return IMP32ResetAIMProfile( pAIM );
}

APIRET APIENTRY IMPQUERYAIMPROFILE( PAIMPROFILE pAIM )
{
    return IMP32QueryAIMProfile( pAIM );
}

/* ──────────────────────────────────────────────────────────────────────────
 * DLL initialisation / termination
 *
 * The original binary's DLL_InitTerm code (object 1 prologue at offset 0x00):
 *   • On init  (flag == 0): initialises the internal segment register copy
 *     (mov ax, seg DGROUP; mov ds, ax) – standard 16-bit DLL startup.
 *   • On term  (flag != 0): no-op beyond restoring DS.
 *
 * For the 32-bit flat rebuild there is nothing to do at init/term time
 * because there is no per-DLL segment state.  We still supply the entry
 * point to satisfy the INITINSTANCE / TERMINSTANCE directives in IMP.DEF.
 * ────────────────────────────────────────────────────────────────────────── */

unsigned _System LibMain( unsigned hmod, unsigned termflag )
{
    (void)hmod;
    (void)termflag;
    return 1;   /* 1 = success */
}
