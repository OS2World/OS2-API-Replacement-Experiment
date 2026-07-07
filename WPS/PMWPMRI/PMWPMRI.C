/*
 * PMWPMRI.C -- OS/2 Workplace Shell MRI stub
 *
 * Replacement for PMWPMRI.DLL (LX, 54311 bytes, IBM v14.103)
 * Module description: 'Workplace Shell MRI'  (orig tag '@#IBM:14.103#@')
 * Module name: 'PMWPMRI'
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -wx -s -fo=PMWPMRI.OBJ PMWPMRI.C
 *   wlink @PMWPMRI.LNK
 *
 * Pure resource DLL -- 0 imports, 0 exports. The original holds the
 * Workplace Shell national-language (MRI) string, menu and dialog
 * resources loaded by the WPS. This stub provides the module so the
 * loader is satisfied; it carries NO resources of its own (see note).
 *
 * NOTE: like the other cWarp *MRI stubs (WPCONMRI/WPPRTMRI/WPSTKMRI),
 * this is a module placeholder, not a functional resource clone. WPS
 * text that would come from PMWPMRI's resources will be absent. A
 * faithful clone requires re-creating the resources (.RC) extracted
 * from the original 54 KB binary.
 */
static const unsigned long _pmwpmri_stub = 0;
