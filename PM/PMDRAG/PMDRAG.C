/*
 * PMDRAG.C -- OS/2 PM Drag and Drop 16-bit Thunk Forwarder
 *
 * Replacement for PMDRAG.DLL (LX, 1731 bytes, IBM)
 * Module name: 'pmdrag'
 *
 * Build (32-bit):
 *   wcc386 -bt=OS2 -bm -wx -s -fo=PMDRAG.OBJ PMDRAG.C
 *   wlink @PMDRAG.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format:  LX, 0 code objects (pure forwarder DLL)
 * Size:    1731 bytes total
 * Imports: PMCTLS only (1 module)
 *
 * PMDRAG.DLL contains NO executable code. Every one of its 33
 * exports is an LX entry table type-0x04 (32-bit import by
 * ordinal) record that forwards directly to PMCTLS.DLL:
 *
 *   DRG16ACCESSDRAGINFO         (ord  1) -> PMCTLS.41
 *   DRG16ADDSTRHANDLE           (ord  2) -> PMCTLS.42
 *   DRG16ALLOCDRAGINFO          (ord  3) -> PMCTLS.43
 *   DRG16ALLOCDRAGTRANSFER      (ord  4) -> PMCTLS.44
 *   DRG16DELETEDRAGINFOSTRHANDL (ord  5) -> PMCTLS.45
 *   DRG16DELETESTRHANDLE        (ord  6) -> PMCTLS.46
 *   DRG16DRAG                   (ord  7) -> PMCTLS.47
 *   DRG16FREEDRAGINFO           (ord  8) -> PMCTLS.48
 *   DRG16FREEDRAGTRANSFER       (ord  9) -> PMCTLS.49
 *   DRG16GETPS                  (ord 10) -> PMCTLS.50
 *   DRG16POSTTRANSFERMSG        (ord 11) -> PMCTLS.51
 *   DRG16PUSHDRAGINFO           (ord 12) -> PMCTLS.52
 *   DRG16QUERYDRAGITEM          (ord 13) -> PMCTLS.53
 *   DRG16QUERYDRAGITEMCOUNT     (ord 14) -> PMCTLS.54
 *   DRG16QUERYDRAGITEMPTR       (ord 15) -> PMCTLS.55
 *   DRG16QUERYNATIVERMF         (ord 16) -> PMCTLS.56
 *   DRG16QUERYNATIVERMFLEN      (ord 17) -> PMCTLS.57
 *   DRG16QUERYSTRNAME           (ord 18) -> PMCTLS.58
 *   DRG16QUERYSTRNAMELEN        (ord 19) -> PMCTLS.59
 *   DRG16QUERYTRUETYPE          (ord 20) -> PMCTLS.60
 *   DRG16QUERYTRUETYPELEN       (ord 21) -> PMCTLS.61
 *   DRG16RELEASEPS              (ord 22) -> PMCTLS.62
 *   DRG16SENDTRANSFERMSG        (ord 23) -> PMCTLS.63
 *   DRG16SETDRAGPOINTER         (ord 24) -> PMCTLS.64
 *   DRG16SETDRAGIMAGE           (ord 25) -> PMCTLS.65
 *   DRG16SETDRAGITEM            (ord 26) -> PMCTLS.66
 *   DRG16VERIFYNATIVERMF        (ord 27) -> PMCTLS.67
 *   DRG16VERIFYRMF              (ord 28) -> PMCTLS.68
 *   DRG16VERIFYTRUETYPE         (ord 29) -> PMCTLS.69
 *   DRG16VERIFYTYPE             (ord 30) -> PMCTLS.70
 *   DRG16VERIFYTYPESET          (ord 31) -> PMCTLS.71
 *   DRG16DRAGFILES              (ord 63) -> PMCTLS.72
 *   DRG16ACCEPTDROPPEDFILES     (ord 64) -> PMCTLS.73
 *
 * Ordinal encoding in the original entry table:
 *   Each entry stores (pmctls_ord << 16) | 0x0001 as a packed
 *   32-bit value in the type-0x04 bundle record.
 *
 * Architecture:
 *   The 32-bit Drg* public API (DrgDrag, DrgAllocDraginfo, etc.)
 *   is implemented in PMCTLS.DLL (part of PMMERGE). PMDRAG.DLL
 *   provides named 16-bit entry points for 16-bit OS/2 applications
 *   that call drag'n'drop. The DRG16* names thunk through to the
 *   corresponding 32-bit PMCTLS ordinals 41-73.
 *
 *   Ordinal gaps 32-62 in PMDRAG are unassigned in the original.
 *
 * Replacement strategy:
 *   Since the original has no code at all, the cleanest replacement
 *   is a C source file with inline stubs that call the PMCTLS
 *   ordinals via IMPORT directives in the linker file. wlink handles
 *   the forwarding at link time, exactly replicating the original.
 *
 *   Each DRG16* stub calls the corresponding public Drg* function
 *   which resolves to PMCTLS via its import library. This gives
 *   correct behaviour for any 16-bit app that calls these APIs.
 *
 * Signatures:
 *   All Drg* functions from OS/2 Toolkit 4.5 PMSTDDLG.H.
 *   The DRG16* entry points use the same signatures -- they are
 *   straight 16-to-32 thunks with no parameter translation.
 * ============================================================
 */

#define INCL_WINSTDDRAG
#define INCL_WINWINDOWMGR
#define INCL_GPILCIDS
#include <os2.h>

/* ================================================================== */
/* Forward declarations of PMCTLS Drg* functions we call through to  */
/* These are resolved via IMPORT directives in PMDRAG.LNK            */
/* ================================================================== */

/* Declared as extern so the compiler doesn't need their bodies here. */
/* The linker resolves them from PMCTLS via the IMPORT table.         */

/* ================================================================== */
/* ord  1: DRG16ACCESSDRAGINFO -> PMCTLS.41 (DrgAccessDraginfo)      */
/* ================================================================== */
PDRAGINFO APIENTRY DRG16ACCESSDRAGINFO(PDRAGINFO pdinfo)
{
    return (PDRAGINFO)DrgAccessDraginfo(pdinfo);
}
#pragma aux DRG16ACCESSDRAGINFO "DRG16ACCESSDRAGINFO"

/* ================================================================== */
/* ord  2: DRG16ADDSTRHANDLE -> PMCTLS.42 (DrgAddStrHandle)          */
/* ================================================================== */
HSTR APIENTRY DRG16ADDSTRHANDLE(PSZ psz)
{
    return DrgAddStrHandle(psz);
}
#pragma aux DRG16ADDSTRHANDLE "DRG16ADDSTRHANDLE"

/* ================================================================== */
/* ord  3: DRG16ALLOCDRAGINFO -> PMCTLS.43 (DrgAllocDraginfo)        */
/* ================================================================== */
PDRAGINFO APIENTRY DRG16ALLOCDRAGINFO(ULONG cditem)
{
    return DrgAllocDraginfo(cditem);
}
#pragma aux DRG16ALLOCDRAGINFO "DRG16ALLOCDRAGINFO"

/* ================================================================== */
/* ord  4: DRG16ALLOCDRAGTRANSFER -> PMCTLS.44 (DrgAllocDragtransfer)*/
/* ================================================================== */
PDRAGTRANSFER APIENTRY DRG16ALLOCDRAGTRANSFER(ULONG cdxfer)
{
    return DrgAllocDragtransfer(cdxfer);
}
#pragma aux DRG16ALLOCDRAGTRANSFER "DRG16ALLOCDRAGTRANSFER"

/* ================================================================== */
/* ord  5: DRG16DELETEDRAGINFOSTRHANDLES -> PMCTLS.45                */
/* ================================================================== */
BOOL APIENTRY DRG16DELETEDRAGINFOSTRHANDLES(PDRAGINFO pdinfo)
{
    return DrgDeleteDraginfoStrHandles(pdinfo);
}
#pragma aux DRG16DELETEDRAGINFOSTRHANDLES "DRG16DELETEDRAGINFOSTRHANDLES"

/* ================================================================== */
/* ord  6: DRG16DELETESTRHANDLE -> PMCTLS.46 (DrgDeleteStrHandle)    */
/* ================================================================== */
BOOL APIENTRY DRG16DELETESTRHANDLE(HSTR hstr)
{
    return DrgDeleteStrHandle(hstr);
}
#pragma aux DRG16DELETESTRHANDLE "DRG16DELETESTRHANDLE"

/* ================================================================== */
/* ord  7: DRG16DRAG -> PMCTLS.47 (DrgDrag)                         */
/* ================================================================== */
HWND APIENTRY DRG16DRAG(HWND hwndSource, PDRAGINFO pdinfo,
                          PDRAGIMAGE pdimg, ULONG cdimg,
                          LONG vkTerminate, PVOID pRsvd)
{
    return DrgDrag(hwndSource, pdinfo, pdimg, cdimg, vkTerminate, pRsvd);
}
#pragma aux DRG16DRAG "DRG16DRAG"

/* ================================================================== */
/* ord  8: DRG16FREEDRAGINFO -> PMCTLS.48 (DrgFreeDraginfo)          */
/* ================================================================== */
BOOL APIENTRY DRG16FREEDRAGINFO(PDRAGINFO pdinfo)
{
    return DrgFreeDraginfo(pdinfo);
}
#pragma aux DRG16FREEDRAGINFO "DRG16FREEDRAGINFO"

/* ================================================================== */
/* ord  9: DRG16FREEDRAGTRANSFER -> PMCTLS.49 (DrgFreeDragtransfer)  */
/* ================================================================== */
BOOL APIENTRY DRG16FREEDRAGTRANSFER(PDRAGTRANSFER pdxfer)
{
    return DrgFreeDragtransfer(pdxfer);
}
#pragma aux DRG16FREEDRAGTRANSFER "DRG16FREEDRAGTRANSFER"

/* ================================================================== */
/* ord 10: DRG16GETPS -> PMCTLS.50 (DrgGetPS)                        */
/* ================================================================== */
HPS APIENTRY DRG16GETPS(HWND hwnd)
{
    return DrgGetPS(hwnd);
}
#pragma aux DRG16GETPS "DRG16GETPS"

/* ================================================================== */
/* ord 11: DRG16POSTTRANSFERMSG -> PMCTLS.51 (DrgPostTransferMsg)    */
/* ================================================================== */
BOOL APIENTRY DRG16POSTTRANSFERMSG(HWND hwnd, ULONG msg,
                                     PDRAGTRANSFER pdxfer,
                                     USHORT fs, ULONG ulReserved,
                                     BOOL fRetry)
{
    return DrgPostTransferMsg(hwnd, msg, pdxfer, fs, ulReserved, fRetry);
}
#pragma aux DRG16POSTTRANSFERMSG "DRG16POSTTRANSFERMSG"

/* ================================================================== */
/* ord 12: DRG16PUSHDRAGINFO -> PMCTLS.52 (DrgPushDraginfo)          */
/* ================================================================== */
BOOL APIENTRY DRG16PUSHDRAGINFO(PDRAGINFO pdinfo, HWND hwndDrop)
{
    return DrgPushDraginfo(pdinfo, hwndDrop);
}
#pragma aux DRG16PUSHDRAGINFO "DRG16PUSHDRAGINFO"

/* ================================================================== */
/* ord 13: DRG16QUERYDRAGITEM -> PMCTLS.53 (DrgQueryDragitem)        */
/* ================================================================== */
BOOL APIENTRY DRG16QUERYDRAGITEM(PDRAGINFO pdinfo, ULONG cb,
                                    PDRAGITEM pditem, ULONG iitem)
{
    return DrgQueryDragitem(pdinfo, cb, pditem, iitem);
}
#pragma aux DRG16QUERYDRAGITEM "DRG16QUERYDRAGITEM"

/* ================================================================== */
/* ord 14: DRG16QUERYDRAGITEMCOUNT -> PMCTLS.54                      */
/* ================================================================== */
ULONG APIENTRY DRG16QUERYDRAGITEMCOUNT(PDRAGINFO pdinfo)
{
    return DrgQueryDragitemCount(pdinfo);
}
#pragma aux DRG16QUERYDRAGITEMCOUNT "DRG16QUERYDRAGITEMCOUNT"

/* ================================================================== */
/* ord 15: DRG16QUERYDRAGITEMPTR -> PMCTLS.55 (DrgQueryDragitemPtr)  */
/* ================================================================== */
PDRAGITEM APIENTRY DRG16QUERYDRAGITEMPTR(PDRAGINFO pdinfo, ULONG iitem)
{
    return DrgQueryDragitemPtr(pdinfo, iitem);
}
#pragma aux DRG16QUERYDRAGITEMPTR "DRG16QUERYDRAGITEMPTR"

/* ================================================================== */
/* ord 16: DRG16QUERYNATIVERMF -> PMCTLS.56 (DrgQueryNativeRMF)      */
/* ================================================================== */
BOOL APIENTRY DRG16QUERYNATIVERMF(PDRAGITEM pditem, ULONG cbBuf,
                                    PSZ pszBuf)
{
    return DrgQueryNativeRMF(pditem, cbBuf, pszBuf);
}
#pragma aux DRG16QUERYNATIVERMF "DRG16QUERYNATIVERMF"

/* ================================================================== */
/* ord 17: DRG16QUERYNATIVERMFLEN -> PMCTLS.57 (DrgQueryNativeRMFLen)*/
/* ================================================================== */
ULONG APIENTRY DRG16QUERYNATIVERMFLEN(PDRAGITEM pditem)
{
    return DrgQueryNativeRMFLen(pditem);
}
#pragma aux DRG16QUERYNATIVERMFLEN "DRG16QUERYNATIVERMFLEN"

/* ================================================================== */
/* ord 18: DRG16QUERYSTRNAME -> PMCTLS.58 (DrgQueryStrName)          */
/* ================================================================== */
ULONG APIENTRY DRG16QUERYSTRNAME(HSTR hstr, ULONG cbBuf, PSZ pszBuf)
{
    return DrgQueryStrName(hstr, cbBuf, pszBuf);
}
#pragma aux DRG16QUERYSTRNAME "DRG16QUERYSTRNAME"

/* ================================================================== */
/* ord 19: DRG16QUERYSTRNAMELEN -> PMCTLS.59 (DrgQueryStrNameLen)    */
/* ================================================================== */
ULONG APIENTRY DRG16QUERYSTRNAMELEN(HSTR hstr)
{
    return DrgQueryStrNameLen(hstr);
}
#pragma aux DRG16QUERYSTRNAMELEN "DRG16QUERYSTRNAMELEN"

/* ================================================================== */
/* ord 20: DRG16QUERYTRUETYPE -> PMCTLS.60 (DrgQueryTrueType)        */
/* ================================================================== */
BOOL APIENTRY DRG16QUERYTRUETYPE(PDRAGITEM pditem, ULONG cbBuf,
                                    PSZ pszBuf)
{
    return DrgQueryTrueType(pditem, cbBuf, pszBuf);
}
#pragma aux DRG16QUERYTRUETYPE "DRG16QUERYTRUETYPE"

/* ================================================================== */
/* ord 21: DRG16QUERYTRUETYPELEN -> PMCTLS.61 (DrgQueryTrueTypeLen)  */
/* ================================================================== */
ULONG APIENTRY DRG16QUERYTRUETYPELEN(PDRAGITEM pditem)
{
    return DrgQueryTrueTypeLen(pditem);
}
#pragma aux DRG16QUERYTRUETYPELEN "DRG16QUERYTRUETYPELEN"

/* ================================================================== */
/* ord 22: DRG16RELEASEPS -> PMCTLS.62 (DrgReleasePS)                */
/* ================================================================== */
BOOL APIENTRY DRG16RELEASEPS(HPS hps)
{
    return DrgReleasePS(hps);
}
#pragma aux DRG16RELEASEPS "DRG16RELEASEPS"

/* ================================================================== */
/* ord 23: DRG16SENDTRANSFERMSG -> PMCTLS.63 (DrgSendTransferMsg)    */
/* ================================================================== */
MRESULT APIENTRY DRG16SENDTRANSFERMSG(HWND hwnd, ULONG msg,
                                        PDRAGTRANSFER pdxfer,
                                        USHORT fs)
{
    return DrgSendTransferMsg(hwnd, msg,
                              (MPARAM)pdxfer, (MPARAM)(ULONG)fs);
}
#pragma aux DRG16SENDTRANSFERMSG "DRG16SENDTRANSFERMSG"

/* ================================================================== */
/* ord 24: DRG16SETDRAGPOINTER -> PMCTLS.64 (DrgSetDragPointer)      */
/* ================================================================== */
BOOL APIENTRY DRG16SETDRAGPOINTER(PDRAGINFO pdinfo, HWND hwnd)
{
    return DrgSetDragPointer(pdinfo, hwnd);
}
#pragma aux DRG16SETDRAGPOINTER "DRG16SETDRAGPOINTER"

/* ================================================================== */
/* ord 25: DRG16SETDRAGIMAGE -> PMCTLS.65 (DrgSetDragimage)          */
/* ================================================================== */
BOOL APIENTRY DRG16SETDRAGIMAGE(PDRAGINFO pdinfo,
                                   PDRAGIMAGE pdimg, ULONG cdimg,
                                   PVOID pReserved)
{
    return DrgSetDragImage(pdinfo, pdimg, cdimg, pReserved);
}
#pragma aux DRG16SETDRAGIMAGE "DRG16SETDRAGIMAGE"

/* ================================================================== */
/* ord 26: DRG16SETDRAGITEM -> PMCTLS.66 (DrgSetDragitem)            */
/* ================================================================== */
BOOL APIENTRY DRG16SETDRAGITEM(PDRAGINFO pdinfo, PDRAGITEM pditem,
                                  ULONG cbitem, ULONG iitem)
{
    return DrgSetDragitem(pdinfo, pditem, cbitem, iitem);
}
#pragma aux DRG16SETDRAGITEM "DRG16SETDRAGITEM"

/* ================================================================== */
/* ord 27: DRG16VERIFYNATIVERMF -> PMCTLS.67 (DrgVerifyNativeRMF)    */
/* ================================================================== */
BOOL APIENTRY DRG16VERIFYNATIVERMF(PDRAGITEM pditem, PSZ pszRMF)
{
    return DrgVerifyNativeRMF(pditem, pszRMF);
}
#pragma aux DRG16VERIFYNATIVERMF "DRG16VERIFYNATIVERMF"

/* ================================================================== */
/* ord 28: DRG16VERIFYRMF -> PMCTLS.68 (DrgVerifyRMF)               */
/* ================================================================== */
BOOL APIENTRY DRG16VERIFYRMF(PDRAGITEM pditem, PSZ pszMech,
                                PSZ pszFmt)
{
    return DrgVerifyRMF(pditem, pszMech, pszFmt);
}
#pragma aux DRG16VERIFYRMF "DRG16VERIFYRMF"

/* ================================================================== */
/* ord 29: DRG16VERIFYTRUETYPE -> PMCTLS.69 (DrgVerifyTrueType)      */
/* ================================================================== */
BOOL APIENTRY DRG16VERIFYTRUETYPE(PDRAGITEM pditem, PSZ pszType)
{
    return DrgVerifyTrueType(pditem, pszType);
}
#pragma aux DRG16VERIFYTRUETYPE "DRG16VERIFYTRUETYPE"

/* ================================================================== */
/* ord 30: DRG16VERIFYTYPE -> PMCTLS.70 (DrgVerifyType)             */
/* ================================================================== */
BOOL APIENTRY DRG16VERIFYTYPE(PDRAGITEM pditem, PSZ pszType)
{
    return DrgVerifyType(pditem, pszType);
}
#pragma aux DRG16VERIFYTYPE "DRG16VERIFYTYPE"

/* ================================================================== */
/* ord 31: DRG16VERIFYTYPESET -> PMCTLS.71 (DrgVerifyTypeSet)        */
/* ================================================================== */
BOOL APIENTRY DRG16VERIFYTYPESET(PDRAGITEM pditem, PCSZ pszTypeSet,
                                    HSTR hstrType, PSZ pszSubType)
{
    return DrgVerifyTypeSet(pditem, pszTypeSet, hstrType, pszSubType);
}
#pragma aux DRG16VERIFYTYPESET "DRG16VERIFYTYPESET"

/* ================================================================== */
/* ord 63: DRG16DRAGFILES -> PMCTLS.72 (DrgDragFiles)               */
/* ================================================================== */
ULONG APIENTRY DRG16DRAGFILES(HWND hwnd, PCSZ *apszFiles, PCSZ *apszTypes,
                                 PCSZ *apszTargets, ULONG cFiles,
                                 HPOINTER hptrDrag, ULONG vkDrag,
                                 BOOL fSourceRM, ULONG ulReserved)
{
    return DrgDragFiles(hwnd, apszFiles, apszTypes, apszTargets,
                        cFiles, hptrDrag, vkDrag, fSourceRM, ulReserved);
}
#pragma aux DRG16DRAGFILES "DRG16DRAGFILES"

/* ================================================================== */
/* ord 64: DRG16ACCEPTDROPPEDFILES -> PMCTLS.73 (DrgAcceptDroppedFiles)*/
/* ================================================================== */
BOOL DRG16ACCEPTDROPPEDFILESStub(HWND hwnd, PCSZ pszPath,
                                    HSTR hstrTypes, PSZ pszMech,
                                    ULONG ulReserved)
{
    /* DrgAcceptDroppedFiles has HSTR params; forward via ULONG casts */
    typedef BOOL APIENTRY PFNACCEPT(HWND,ULONG,ULONG,ULONG,ULONG);
    PFNACCEPT *pfn = (PFNACCEPT*)DrgAcceptDroppedFiles;
    return pfn(hwnd,(ULONG)pszPath,(ULONG)hstrTypes,(ULONG)pszMech,ulReserved);
}
#pragma aux DRG16ACCEPTDROPPEDFILESStub "DRG16ACCEPTDROPPEDFILESStub"
