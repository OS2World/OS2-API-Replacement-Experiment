/*
 * WPPRTMRI.C -- OS/2 Workplace Print Object Resource DLL stub
 *
 * Replacement for WPPRTMRI.DLL (LX, 76433 bytes, IBM v14.083)
 * Module description: 'Workplace Print Object Resource dll'
 * Module name: 'WPPRTMRI'
 *
 * Build:
 *   wcc386 -bt=OS2 -bm -wx -s -fo=WPPRTMRI.OBJ WPPRTMRI.C
 *   wlink @WPPRTMRI.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format:  LX, 6 objects
 *   Obj1: 32-bit CODE  7253b  -- DLL init routine (at offset 0x10)
 *   Obj2: 32-bit RW SHR   4b -- minimal shared header
 *   Obj3: 32-bit RW SHR  48b -- OS version + module name (filled by init)
 *   Obj4: 32-bit RO SHR 64KB -- WPS printer MRI resources (part 1)
 *   Obj5: 32-bit RO SHR 64KB -- WPS printer MRI resources (part 2)
 *   Obj6: 32-bit RO SHR 18KB -- WPS printer MRI resources (part 3)
 *
 * Imports: DOSCALLS.299 (DosQuerySysInfo)
 *          DOSCALLS.509 (DosQueryModuleName)
 * Exports: none
 * Entry:   Obj1:0x0010
 *
 * Init calls DosQuerySysInfo for OS version and DosQueryModuleName
 * to store the module path in Obj3. MRI resources (Objs 4-6) absent
 * in stub -- WPS printer object strings will be blank, acceptable
 * for boot purposes.
 * ============================================================
 */

#define INCL_DOSMISC
#define INCL_DOSMODULEMGR
#define INCL_DOS
#include <os2.h>

static ULONG _wpprtmri_verMajor  = 0;
static ULONG _wpprtmri_verMinor  = 0;
static ULONG _wpprtmri_revision  = 0;
static CHAR  _wpprtmri_modpath[260] = {0};

ULONG APIENTRY LibMain(ULONG hmod, ULONG flag)
{
    if (flag == 0) {
        ULONG aulBuf[3] = {0,0,0};
        DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_REVISION,
                        aulBuf, sizeof(aulBuf));
        _wpprtmri_verMajor = aulBuf[0];
        _wpprtmri_verMinor = aulBuf[1];
        _wpprtmri_revision = aulBuf[2];
        DosQueryModuleName((HMODULE)hmod, sizeof(_wpprtmri_modpath),
                           _wpprtmri_modpath);
    }
    return TRUE;
}
#pragma aux LibMain "LibMain"
