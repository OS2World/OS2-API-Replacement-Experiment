/*
 * BDWPCLS.H -- Public interface for BDWPCLS.DLL
 *              Bidirectional WPS Settings Page for OS/2
 *
 * Include after os2.h:
 *   #define INCL_WIN
 *   #include <os2.h>
 *   #include "BDWPCLS.H"
 */

#ifndef BDWPCLS_H_INCLUDED
#define BDWPCLS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Bidi attribute flags (used in flBidiAttr fields)                   */
/* ------------------------------------------------------------------ */
#define BIDI_TEXTTYPE_VISUAL         0x00000001
#define BIDI_ORIENT_RTL              0x00000002
#define BIDI_NUMERALS_PASSTHRU       0x00000004
#define BIDI_SYM_SWAP_OFF            0x00000008
#define BIDI_WORD_BREAK_ON           0x00000010
#define BIDI_DISPLAY_SHAPED          0x00000020
#define BIDI_SCREEN_FIELD_REV        0x00000040
#define BIDI_STATUS_INDICATOR        0x00000080
#define BIDI_HKFLAG_ENG_LAYER        0x00000100
#define BIDI_INPUT_PROCESSING        0x00000200
#define BIDI_DEFAULT_ATTR            0x00000000

/* ------------------------------------------------------------------ */
/* Settings structure passed between WPS and BDWPCLS functions        */
/* ------------------------------------------------------------------ */
#pragma pack(1)
typedef struct _BIDI_SETTINGS {
    ULONG  cbSize;            /* sizeof(BIDI_SETTINGS)                */
    ULONG  flBidiAttr;        /* current bidi attribute flags         */
    ULONG  flBidiAttrSaved;   /* saved (original) bidi attribute      */
    ULONG  idLang;            /* current language ID                  */
    ULONG  idLangSaved;       /* saved language ID                    */
    HWND   hwndDlg;           /* settings dialog window handle        */
    HWND   hwndHelp;          /* help instance handle                 */
    BOOL   fModified;         /* TRUE if settings changed             */
    BOOL   fHebrewSystem;     /* TRUE if Hebrew OS/2 system           */
    BOOL   fDOSProgram;       /* TRUE if object is a DOS program      */
    CHAR   szObjTitle[256];   /* object title for help context        */
} BIDI_SETTINGS;
typedef BIDI_SETTINGS *PBIDI_SETTINGS;
#pragma pack()

/* ------------------------------------------------------------------ */
/* ord  1: BDWPSDLG_is_DOS_prgm                                      */
/*                                                                     */
/* Test whether a WPS object is a DOS/WIN program.                    */
/* pObject: WPS object pointer (from wpAddSettingsPages context).     */
/* Returns TRUE if the object is a DOS program; FALSE otherwise.      */
/* ------------------------------------------------------------------ */
BOOL APIENTRY BDWPSDLG_is_DOS_prgm( PVOID pObject );

/* ------------------------------------------------------------------ */
/* ord  2: BDWPSDLG_PageSettings                                     */
/*                                                                     */
/* WPS Settings notebook page dialog procedure.                       */
/* Used as the pfnwp in PAGEINFO when inserting the bidi settings     */
/* page via wpInsertSettingsPage.                                     */
/* Standard PM dialog procedure signature.                            */
/* ------------------------------------------------------------------ */
MRESULT EXPENTRY BDWPSDLG_PageSettings( HWND hwnd, ULONG msg,
                                          MPARAM mp1, MPARAM mp2 );

/* ------------------------------------------------------------------ */
/* ord  3: BDWPSDLG_BidiValError                                     */
/*                                                                     */
/* Display an error message for inconsistent bidi settings.           */
/* hwnd:    parent window for the error dialog.                       */
/* ulErrId: error identifier (selects the message text).              */
/* Returns: WinMessageBox result (MBID_OK).                          */
/* ------------------------------------------------------------------ */
ULONG APIENTRY BDWPSDLG_BidiValError( HWND hwnd, ULONG ulErrId );

/* ------------------------------------------------------------------ */
/* ord  4: IsHebrewSystem                                            */
/*                                                                     */
/* Test whether the running system is configured for Hebrew.          */
/* Queries the active language from PMBIDI.DLL.                       */
/* Returns TRUE for Hebrew, FALSE for Arabic or neutral systems.      */
/* ------------------------------------------------------------------ */
BOOL APIENTRY IsHebrewSystem( void );

/* ------------------------------------------------------------------ */
/* ord  5: BDWPSDLG_setup                                            */
/*                                                                     */
/* Initialise a BIDI_SETTINGS structure for a WPS object.            */
/* Call once when the settings page is first created.                 */
/* pObject:   WPS object pointer.                                     */
/* pSettings: caller-allocated BIDI_SETTINGS to initialise.          */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BDWPSDLG_setup( PVOID pObject, PBIDI_SETTINGS pSettings );

/* ------------------------------------------------------------------ */
/* ord  6: FindRunningBidiSettings                                   */
/*                                                                     */
/* Get the current system-wide bidi attribute and language settings.  */
/* pulAttr: receives flBidiAttr of the running system.               */
/* pulLang: receives the active language ID.                          */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY FindRunningBidiSettings( PULONG pulAttr, PULONG pulLang );

/* ------------------------------------------------------------------ */
/* ord  7: BDWPSDLG_LoadDlg                                          */
/*                                                                     */
/* Load and run the bidi settings dialog modally.                     */
/* hwndParent: parent window (Settings notebook page).               */
/* hmod:       module handle for resources (NULLHANDLE = this DLL).  */
/* idDlg:      dialog resource ID.                                    */
/* pSettings:  BIDI_SETTINGS structure to populate.                   */
/* Returns: DID_OK if user confirmed, DID_CANCEL if dismissed.       */
/* ------------------------------------------------------------------ */
ULONG APIENTRY BDWPSDLG_LoadDlg( HWND hwndParent, HMODULE hmod,
                                   ULONG idDlg, PBIDI_SETTINGS pSettings );

/* ------------------------------------------------------------------ */
/* ord  8: BDWPSDLG_ResetEnv                                         */
/*                                                                     */
/* Reset pSettings to the default (neutral) bidi configuration.      */
/* Also refreshes the dialog controls if the dialog is open.         */
/* pSettings: BIDI_SETTINGS to reset.                                 */
/* Returns: NO_ERROR.                                                 */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BDWPSDLG_ResetEnv( PBIDI_SETTINGS pSettings );

/* ------------------------------------------------------------------ */
/* ord  9: BDWPSDLG_ModifyEnv                                        */
/*                                                                     */
/* Apply changed bidi settings to a WPS program object.              */
/* Updates the object's ENVIRONMENT string with new bidi attributes. */
/* pObject:   WPS object to modify.                                   */
/* pSettings: BIDI_SETTINGS containing the new values.               */
/* Returns: NO_ERROR on success.                                      */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BDWPSDLG_ModifyEnv( PVOID pObject,
                                      PBIDI_SETTINGS pSettings );

/* ------------------------------------------------------------------ */
/* ord 10: BDWPSDLG_Verify_BidiSettings                             */
/*                                                                     */
/* Validate a set of bidi flags for internal consistency.             */
/* Some combinations are mutually exclusive.                          */
/* flBidiAttr: combination of BIDI_* flags to validate.              */
/* pulError:   receives an error code if validation fails; may be NULL*/
/* Returns: TRUE if valid, FALSE if inconsistent.                    */
/* ------------------------------------------------------------------ */
BOOL APIENTRY BDWPSDLG_Verify_BidiSettings( ULONG flBidiAttr,
                                              PULONG pulError );

/* ------------------------------------------------------------------ */
/* ord 11: BDWPSDLG_getMRI_Lang                                      */
/*                                                                     */
/* Get the current MRI (Message Resource Information) language.       */
/* pulLang: receives the language ID for dialog UI.                   */
/* Returns: NO_ERROR.                                                 */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BDWPSDLG_getMRI_Lang( PULONG pulLang );

/* ------------------------------------------------------------------ */
/* ord 12: BDWPSDLG_setMRI_Lang                                      */
/*                                                                     */
/* Set the MRI language for the settings dialog UI.                  */
/* ulLang: new language ID to activate.                               */
/* Returns: NO_ERROR.                                                 */
/* ------------------------------------------------------------------ */
APIRET APIENTRY BDWPSDLG_setMRI_Lang( ULONG ulLang );

#ifdef __cplusplus
}
#endif

#endif /* BDWPCLS_H_INCLUDED */
