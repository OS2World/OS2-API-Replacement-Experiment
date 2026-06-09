/*
 * IMP.H  –  Public interface for IMP.DLL (Input Method Profiler)
 *
 * Include this header in any application that calls IMP.DLL.
 * Requires the standard OS/2 headers (os2.h) to be included first.
 *
 * Example:
 *   #define INCL_BASE
 *   #include <os2.h>
 *   #include "IMP.H"
 */

#ifndef IMP_H_INCLUDED
#define IMP_H_INCLUDED

/* ──────────────────────────────────────────────────────────────────────────
 * AIMPROFILE structure
 *
 * Pass a pointer to this structure to all three IMP32 functions.
 * cbSize must always be sizeof(AIMPROFILE) and flReserved must be zero.
 * ────────────────────────────────────────────────────────────────────────── */

#pragma pack(1)

typedef struct _AIMPROFILE {
    ULONG   cbSize;        /* must be sizeof(AIMPROFILE) == 28 (0x1C)       */
    ULONG   flReserved;    /* reserved, must be 0                           */
    ULONG   fAIMActive;    /* 0 = AIM disabled,  non-zero = enabled         */
    ULONG   ulTimeOut;     /* idle timeout in ms  (0 = no timeout)          */
    ULONG   ulFKAccept;    /* filter-key accept count                       */
    ULONG   ulFKRate;      /* filter-key repeat rate, ms                    */
    ULONG   ulFKDelay;     /* filter-key initial delay, ms                  */
} AIMPROFILE;
typedef AIMPROFILE *PAIMPROFILE;

#pragma pack()

/* Convenience macro so callers can initialise cbSize portably */
#define AIM_PROFILE_SIZE   ((ULONG)sizeof(AIMPROFILE))

/* ──────────────────────────────────────────────────────────────────────────
 * Function prototypes
 * ────────────────────────────────────────────────────────────────────────── */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IMP32SetAIMProfile  (ordinal 2)
 *
 * Write the AIM profile in *pAIM to OS2.INI
 * (application "PM_AlternateInputMethods").
 *
 * Returns: NO_ERROR on success
 *          ERROR_INVALID_PARAMETER if pAIM is NULL, cbSize wrong, or
 *                                  flReserved non-zero
 *          ERROR_WRITE_FAULT if PrfWriteProfileData fails
 */
APIRET APIENTRY IMP32SetAIMProfile( PAIMPROFILE pAIM );

/*
 * IMP32ResetAIMProfile  (ordinal 5)
 *
 * Fill *pAIM with the compiled-in defaults and write them to OS2.INI.
 *
 * Default values:
 *   fAIMActive = 0
 *   ulTimeOut  = 120  ms
 *   ulFKAccept = 0
 *   ulFKRate   = 500  ms
 *   ulFKDelay  = 1000 ms
 *
 * Returns: same as IMP32SetAIMProfile
 */
APIRET APIENTRY IMP32ResetAIMProfile( PAIMPROFILE pAIM );

/*
 * IMP32QueryAIMProfile  (ordinal 6)
 *
 * Read the AIM profile from OS2.INI into *pAIM.
 * Keys not present in the INI file silently receive the default value.
 *
 * Returns: NO_ERROR on success (including "key absent" – default is used)
 *          ERROR_INVALID_PARAMETER if pAIM is NULL, cbSize wrong, or
 *                                  flReserved non-zero
 */
APIRET APIENTRY IMP32QueryAIMProfile( PAIMPROFILE pAIM );

/*
 * 16-bit compatibility wrappers  (ordinals 1, 3, 4)
 * These are thin forwarding stubs that call the 32-bit versions above.
 * 32-bit applications should call the IMP32* functions directly.
 */
APIRET APIENTRY IMPSETAIMPROFILE  ( PAIMPROFILE pAIM );
APIRET APIENTRY IMPRESETAIMPROFILE( PAIMPROFILE pAIM );
APIRET APIENTRY IMPQUERYAIMPROFILE( PAIMPROFILE pAIM );

#ifdef __cplusplus
}
#endif

#endif /* IMP_H_INCLUDED */
