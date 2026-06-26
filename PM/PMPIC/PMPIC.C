/*
 * PMPIC.C -- OS/2 PMPIC Dynalink stub
 *
 * Replacement for PMPIC.DLL (LX, 47206 bytes, IBM)
 * Module description: 'PMPIC Dynalink'
 * Module name: 'pmpic'
 *
 * Build (32-bit):
 *   wcc386 -bt=OS2 -bm -wx -s -fo=PMPIC.OBJ PMPIC.C
 *   wlink @PMPIC.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format:  LX, 4 objects
 *   Obj1: 32-bit CODE  52072 bytes  -- PIF/metafile conversion engine
 *   Obj2: 16-bit CODE     77 bytes  -- 16-bit thunk stubs
 *   Obj3: 32-bit CODE    268 bytes  -- 32-bit entry bridge
 *   Obj4: 32-bit RW     7056 bytes  -- data (strings, profiles, heap)
 *
 * Imports: DOSCALLS(20), PMGPI(19), PMWIN(6), NLS(6),
 *          PMSHAPI(unknown), PMSPL(2), PMMERGE(3), MSG(1)
 *
 * Key strings (Obj4): 'PPM_PICPRINT', 'PROFILE', 'Q_STD',
 *   'PPN12345.MET', 'PicIchg', 'DISPLAY', 'Metafile created from PIF by',
 *   'OS/2 FONT', 'HEAPMIN'
 *
 * Exports (7 functions):
 *   ord  1: PicPrint     -- Print a PIF/picture to spooler (16-bit entry)
 *   ord  2: PicIchg      -- Picture interchange (16-bit entry)
 *   ord  3: Pif2Met      -- Convert PIF to metafile (16-bit entry)
 *   ord  4: PrfPif2Met   -- Profile-driven PIF to metafile (16-bit entry)
 *   ord 11: Pic32Print   -- 32-bit version of PicPrint
 *   ord 12: Pic32Ichg    -- 32-bit version of PicIchg
 *   ord 13: Prf32Pif2Met -- 32-bit version of PrfPif2Met
 *
 * Ordinal gaps (5-10) are intentional -- not assigned in original.
 *
 * Architecture:
 *   PMPIC.DLL converts OS/2 PIF (Picture Interchange Format) files
 *   to PM metafiles and handles printing of picture objects. It is
 *   used by the PM print subsystem (PMSPL) and by applications that
 *   embed pictures. Ords 1-4 are 16-bit thunked entries; ords 11-13
 *   are the native 32-bit equivalents added later.
 * ============================================================
 */

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DEV
#include <os2.h>

/* Error return for all stubs */
#define PIC_ERROR  0UL

/* ================================================================== */
/* 16-bit thunk entry points (ords 1-4)                               */
/* ================================================================== */

/*
 * PicPrint -- Print a PIF/picture object to the spooler.
 * Parameters are opaque structures; stub returns error.
 */
ULONG APIENTRY PicPrint(PVOID pPicData, ULONG cbPicData,
                          PVOID pPrintParams)
{
    (void)pPicData; (void)cbPicData; (void)pPrintParams;
    return PIC_ERROR;
}
#pragma aux PicPrint "PicPrint"

/*
 * PicIchg -- Picture interchange: convert/render picture data.
 */
ULONG APIENTRY PicIchg(PVOID pSrcData, ULONG cbSrcData,
                         PVOID pDstData, PULONG pcbDstData,
                         ULONG ulFormat)
{
    (void)pSrcData; (void)cbSrcData; (void)pDstData;
    (void)pcbDstData; (void)ulFormat;
    return PIC_ERROR;
}
#pragma aux PicIchg "PicIchg"

/*
 * Pif2Met -- Convert a PIF file to a PM metafile.
 * Returns handle to metafile or NULLHANDLE on error.
 */
HMF APIENTRY Pif2Met(PVOID pPifData, ULONG cbPifData)
{
    (void)pPifData; (void)cbPifData;
    return NULLHANDLE;
}
#pragma aux Pif2Met "Pif2Met"

/*
 * PrfPif2Met -- Profile-driven PIF to metafile conversion.
 * Uses PROFILE data to control conversion parameters.
 */
HMF APIENTRY PrfPif2Met(PVOID pPifData, ULONG cbPifData,
                           PVOID pProfile, ULONG cbProfile)
{
    (void)pPifData; (void)cbPifData; (void)pProfile; (void)cbProfile;
    return NULLHANDLE;
}
#pragma aux PrfPif2Met "PrfPif2Met"

/* ================================================================== */
/* 32-bit native entry points (ords 11-13)                            */
/* ================================================================== */

/*
 * Pic32Print -- 32-bit version of PicPrint.
 */
ULONG APIENTRY Pic32Print(PVOID pPicData, ULONG cbPicData,
                            PVOID pPrintParams)
{
    (void)pPicData; (void)cbPicData; (void)pPrintParams;
    return PIC_ERROR;
}
#pragma aux Pic32Print "Pic32Print"

/*
 * Pic32Ichg -- 32-bit version of PicIchg.
 */
ULONG APIENTRY Pic32Ichg(PVOID pSrcData, ULONG cbSrcData,
                           PVOID pDstData, PULONG pcbDstData,
                           ULONG ulFormat)
{
    (void)pSrcData; (void)cbSrcData; (void)pDstData;
    (void)pcbDstData; (void)ulFormat;
    return PIC_ERROR;
}
#pragma aux Pic32Ichg "Pic32Ichg"

/*
 * Prf32Pif2Met -- 32-bit version of PrfPif2Met.
 */
HMF APIENTRY Prf32Pif2Met(PVOID pPifData, ULONG cbPifData,
                             PVOID pProfile, ULONG cbProfile)
{
    (void)pPifData; (void)cbPifData; (void)pProfile; (void)cbProfile;
    return NULLHANDLE;
}
#pragma aux Prf32Pif2Met "Prf32Pif2Met"
