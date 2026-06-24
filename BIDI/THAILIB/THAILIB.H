/*
 * THAILIB.H -- Public interface for THAILIB.DLL
 *              Thai Language Support Library for OS/2 PM
 *
 * Include after os2.h:
 *   #define INCL_WIN
 *   #include <os2.h>
 *   #include "THAILIB.H"
 */

#ifndef THAILIB_H_INCLUDED
#define THAILIB_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Thai character class constants (returned by internal classifier)   */
/* ------------------------------------------------------------------ */
#define THAI_CLASS_NONE    0x00   /* not a Thai character              */
#define THAI_CLASS_CONS    0x01   /* base consonant                    */
#define THAI_CLASS_VOW_A   0x02   /* above-base vowel diacritic        */
#define THAI_CLASS_VOW_B   0x03   /* below-base vowel diacritic        */
#define THAI_CLASS_TONE    0x04   /* tone mark                         */
#define THAI_CLASS_LEAD    0x05   /* leading (pre-consonant) vowel     */
#define THAI_CLASS_DIGIT   0x06   /* Thai digit (๐-๙)                  */
#define THAI_CLASS_PUNC    0x07   /* Thai punctuation                  */
#define THAI_CLASS_FVOW    0x08   /* following vowel                   */
#define THAI_CLASS_OTH     0x09   /* other composing character         */

/* ------------------------------------------------------------------ */
/* Keyboard layout constants for THAISETKEYBOARDLAYOUT                */
/* ------------------------------------------------------------------ */
#define THAI_KBD_KESMANEE    0    /* TIS 820-2531 standard layout      */
#define THAI_KBD_PATTACHOTE  1    /* Pattachote alternative layout     */

/* ------------------------------------------------------------------ */
/* Toggle key index constants for THAIINITIALIZELIBRARY               */
/* (stored in INI under PM_INSTALL/ToggleKey)                        */
/* ------------------------------------------------------------------ */
#define THAI_TOGGLE_NONE        0
#define THAI_TOGGLE_ACCENT      1    /* ` (Accent Grave) key           */
#define THAI_TOGGLE_LCTRL       2    /* Left Ctrl                      */
#define THAI_TOGGLE_RCTRL       3    /* Right Ctrl                     */
#define THAI_TOGGLE_SCROLLLOCK  4    /* Scroll Lock                    */
#define THAI_TOGGLE_FUNC12      5    /* Function / 12                  */
#define THAI_TOGGLE_ALT_ACCENT  6    /* Alt + `                        */
#define THAI_TOGGLE_PAUSE       7    /* Pause key                      */

/* ------------------------------------------------------------------ */
/* ord 1: THAIINITIALIZELIBRARY                                       */
/*                                                                     */
/* Initialise the Thai library for the calling process.               */
/* Must be called before any other THAILIB function.                  */
/* Reads configuration from HINI_USERPROFILE (PM_INSTALL section),   */
/* opens the keyboard device, and loads FW2DLL for font detection.    */
/*                                                                     */
/* hab     - HAB of the calling application                           */
/* ulFlags - reserved; pass 0                                         */
/*                                                                     */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY THAIINITIALIZELIBRARY( HAB hab, ULONG ulFlags );

/* ------------------------------------------------------------------ */
/* ord 2: OS2_PM_DRV_ENABLE                                          */
/*                                                                     */
/* PM graphics engine (PMGRE) hook for Thai text shaping.            */
/* Called automatically by PMGRE during initialisation.              */
/* Applications should NOT call this directly.                        */
/* ------------------------------------------------------------------ */
BOOL APIENTRY OS2_PM_DRV_ENABLE( PVOID pDrvData, ULONG ulFlags );

/* ------------------------------------------------------------------ */
/* ord 3: THAIISTHAIFONT                                             */
/*                                                                     */
/* Test whether a named font supports Thai characters (CP874).       */
/*                                                                     */
/* pszFacename - font face name (e.g. "Cordia New", "Angsana New")   */
/*                                                                     */
/* Returns: TRUE if the font contains Thai glyphs, FALSE otherwise.  */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIISTHAIFONT( PSZ pszFacename );

/* ------------------------------------------------------------------ */
/* ord 4: THAIFRAMEWNDPROC                                           */
/*                                                                     */
/* Thai-aware PM frame window procedure.                              */
/* Used as a window procedure (or subclass target) for PM windows    */
/* that need to accept Thai keyboard input.                           */
/*                                                                     */
/* Handles:                                                           */
/*   - Toggle key detection (switches English/Thai mode)             */
/*   - Kesmanee keyboard mapping (VK -> Thai CP874 char)             */
/*   - Thai character ordering validation                             */
/*   - WarnBeep on invalid character sequences                        */
/*                                                                     */
/* Standard EXPENTRY PM window procedure.                             */
/* ------------------------------------------------------------------ */
MRESULT EXPENTRY THAIFRAMEWNDPROC( HWND hwnd, ULONG msg,
                                    MPARAM mp1, MPARAM mp2 );

/* ------------------------------------------------------------------ */
/* ord 5: THAIFINDWORDHOOK                                           */
/*                                                                     */
/* PM input hook for Thai word boundary detection.                    */
/* Register with WinSetHook(WH_INPUT) to enable correct double-click */
/* word selection in Thai text (Thai has no inter-word spaces).       */
/*                                                                     */
/* Standard PM hook procedure (EXPENTRY).                            */
/* ------------------------------------------------------------------ */
VOID EXPENTRY THAIFINDWORDHOOK( HAB hab, PQMSG pqmsg, USHORT fs );

/* ------------------------------------------------------------------ */
/* ord 6: THAIWORDSEPERATE                                           */
/*                                                                     */
/* Thai word boundary / word-break algorithm.                         */
/* Analyses a CP874 Thai string and marks word boundaries.            */
/*                                                                     */
/* pszText   - Thai CP874 encoded input string                        */
/* cbText    - byte count of pszText                                  */
/* pabBreaks - caller-allocated BYTE array of size cbText;           */
/*             receives 1 at each word boundary position, 0 elsewhere */
/*                                                                     */
/* Returns: number of word boundaries found.                          */
/* ------------------------------------------------------------------ */
ULONG APIENTRY THAIWORDSEPERATE( PSZ pszText, ULONG cbText,
                                  PBYTE pabBreaks );

/* ------------------------------------------------------------------ */
/* ord 10: THAIISTHAIBASE                                            */
/*                                                                     */
/* Test whether a byte is a Thai base consonant in CP874 encoding.   */
/* Base consonants (0xA1-0xCB) can bear tone marks and vowels.       */
/*                                                                     */
/* chTest - byte value to test                                         */
/*                                                                     */
/* Returns: TRUE if chTest is a Thai base consonant.                 */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIISTHAIBASE( UCHAR chTest );

/* ------------------------------------------------------------------ */
/* ord 11: THAIQUERYISTHAI                                           */
/*                                                                     */
/* Test whether a byte value is any Thai character in CP874.         */
/*                                                                     */
/* chTest - byte value to test                                         */
/*                                                                     */
/* Returns: TRUE if chTest is a Thai character (any class).          */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIQUERYISTHAI( UCHAR chTest );

/* ------------------------------------------------------------------ */
/* ord 12: THAIQUERYISKESMANEE                                       */
/*                                                                     */
/* Determine whether a Thai font uses the Kesmanee (TIS-620)         */
/* character encoding (the OS/2 and Windows standard for Thai).      */
/* Fonts using the Kesmanee encoding map Thai characters to           */
/* CP874 byte values 0xA1-0xFB.                                      */
/*                                                                     */
/* pszFacename - font face name                                       */
/*                                                                     */
/* Returns: TRUE if font uses standard Kesmanee (TIS-620) encoding.  */
/* ------------------------------------------------------------------ */
BOOL APIENTRY THAIQUERYISKESMANEE( PSZ pszFacename );

/* ------------------------------------------------------------------ */
/* ord 13: THAISETKEYBOARDLAYOUT                                     */
/*                                                                     */
/* Set the active Thai keyboard layout.                               */
/*                                                                     */
/* ulLayout - THAI_KBD_KESMANEE (0) or THAI_KBD_PATTACHOTE (1)      */
/*                                                                     */
/* Returns: NO_ERROR on success, ERROR_INVALID_PARAMETER if unknown. */
/* ------------------------------------------------------------------ */
APIRET APIENTRY THAISETKEYBOARDLAYOUT( ULONG ulLayout );

/* ------------------------------------------------------------------ */
/* ord 15: SHOWPANEL                                                  */
/*                                                                     */
/* Show or hide the Thai input mode indicator panel.                  */
/* The panel is a small floating window showing current input mode.  */
/*                                                                     */
/* hwndOwner - owner/reference window for panel positioning          */
/* fShow     - TRUE to show the panel, FALSE to hide it              */
/*                                                                     */
/* Returns: NO_ERROR.                                                 */
/* ------------------------------------------------------------------ */
APIRET APIENTRY SHOWPANEL( HWND hwndOwner, BOOL fShow );

/* ------------------------------------------------------------------ */
/* ord 107: BASEOS_SETKBDLAYER                                       */
/*                                                                     */
/* Switch the keyboard input layer at the OS/2 session manager level.*/
/*                                                                     */
/* ulLayer - 0 = Latin/English keyboard layer                         */
/*           1 = Thai keyboard layer                                  */
/*                                                                     */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BASEOS_SETKBDLAYER( ULONG ulLayer );

#ifdef __cplusplus
}
#endif

#endif /* THAILIB_H_INCLUDED */
