/*
 * BDIME.H -- Public interface for BDIME.DLL
 *            Bidirectional Input Method Editor for OS/2 PM
 *
 * Include after os2.h:
 *   #define INCL_BASE
 *   #define INCL_WIN
 *   #include <os2.h>
 *   #include "BDIME.H"
 */

#ifndef BDIME_H_INCLUDED
#define BDIME_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Hotkey configuration flags for RegBDHookProc                       */
/* ------------------------------------------------------------------ */
#define BDIME_HOTKEY_CTRL_SHIFT  0x01  /* Ctrl+Shift toggles layer   */
#define BDIME_HOTKEY_ALT_SHIFT   0x02  /* Alt+Shift toggles layer    */
#define BDIME_HOTKEY_CUSTOM      0x04  /* Custom hotkey from INI     */

/* ------------------------------------------------------------------ */
/* ord 1: RegBDHookProc                                               */
/*                                                                     */
/* Install the bidi IME hook into the OS/2 PM input chain.            */
/*                                                                     */
/* hab     - HAB of calling thread (from WinInitialize)               */
/* hmq     - HMQ to attach hook to; pass HMQ_CURRENT for system-wide  */
/* ulFlags - BDIME_HOTKEY_* flags, or 0 to use INI configuration      */
/*                                                                     */
/* Returns: NO_ERROR on success, DOS error code on failure.           */
/*                                                                     */
/* On success, keyboard events are intercepted system-wide.           */
/* Ctrl+Shift (or Alt+Shift) switches between Latin and bidi layers.  */
/* The hook is automatically removed when the DLL is freed.           */
/* ------------------------------------------------------------------ */
APIRET APIENTRY RegBDHookProc( HAB hab, HMQ hmq, ULONG ulFlags );

/* ------------------------------------------------------------------ */
/* ord 2: GetBidiHotKeys                                              */
/*                                                                     */
/* Query the active hotkey configuration.                             */
/*                                                                     */
/* pulCtrlShift - receives 1 if Ctrl+Shift is active, 0 otherwise    */
/* pulAltShift  - receives 1 if Alt+Shift is active, 0 otherwise     */
/*                                                                     */
/* Returns: NO_ERROR always.                                          */
/* Either or both parameters may be NULL.                             */
/* ------------------------------------------------------------------ */
APIRET APIENTRY GetBidiHotKeys( PULONG pulCtrlShift, PULONG pulAltShift );

#ifdef __cplusplus
}
#endif

#endif /* BDIME_H_INCLUDED */
