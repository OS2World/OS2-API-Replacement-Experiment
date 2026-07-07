/*
 * PMSDMRI.C -- OS/2 PM Standard Dialogs MRI stub
 *
 * Replacement for PMSDMRI.DLL (LX, 12800 bytes, IBM)
 * Module description: 'PM Standard Dialogs MRI'
 * Module name: 'pmsdmri'
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -wx -s -fo=PMSDMRI.OBJ PMSDMRI.C
 *   wlink @PMSDMRI.LNK
 *
 * Pure resource DLL -- 0 imports, 0 exports. This C file only gives wlink an
 * object to link; the DLL's real content is the resources in PMSDMRI.RC (the
 * standard File Open/Save-As and Font dialogs + their string tables),
 * clean-room re-authored from the public OS/2 Toolkit <pmstddlg.h> ids and
 * bound into the DLL by rc (see PMSDMRI_MAKEFILE). Unlike the earlier hollow
 * *MRI stubs, this DLL carries functional standard-dialog resources.
 */
static const unsigned long _pmsdmri_stub = 0;
