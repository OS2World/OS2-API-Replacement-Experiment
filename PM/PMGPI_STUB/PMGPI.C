/*
 * PMGPI.C -- OS/2 PM GPI (Graphics Programming Interface) stub DLL
 *
 * Replacement for PMGPI.DLL (LX, 248446 bytes, IBM v14.x)
 * Module description: "Gpi functions and 32-bit thunks"
 *
 * Build (32-bit):
 *   wcc386 -bt=OS2 -bm -6s -sg -wx -s -fo=PMGPI.OBJ PMGPI.C
 *   wlink @PMGPI.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format: LX, 6 objects
 *   Obj1: 32-bit CODE  42KB  -- 32-bit thunk dispatchers
 *   Obj2: 16-bit CODE   3KB  -- 16-bit thunk entry table (298 slots)
 *   Obj3: 32-bit CODE 275KB  -- core GPI implementation (~77 functions)
 *   Obj4: 32-bit RW SHR 0.3KB
 *   Obj5: 16-bit RW SHR BSS
 *   Obj6: 32-bit RW SHR 26KB -- shared data, metafile state, font tables
 *
 * Imports: PMMERGE(2), DOSCALLS(17), PMGRE(15), PMWIN(1),
 *          VIOCALLS(3), PMSHAPI(1), PMSPL(1), MSG(3)
 *
 * All 298 exported functions are stubs returning GPI_ERROR/FALSE/0.
 * Signatures match OS/2 Toolkit 4.5 PMGPI.H exactly.
 *
 * Function groups:
 *   DEV*   9  -- Device context operations
 *   GPI*  274 -- Graphics primitives, transforms, fonts, bitmaps, paths
 *   MT*    10  -- Metafile/graphics data streaming
 *   SEG*    3  -- Segment window operations
 *   DSP*    1  -- Display system init
 *   FMT*    1  -- Format conversion
 * ============================================================
 */

#define INCL_GPI
#define INCL_DEV
#define INCL_GPIBITMAPS
#define INCL_GPIREGIONS
#define INCL_GPIMETAFILES
#define INCL_GPIPATHS
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#define INCL_GPIPRIMITIVES
#define INCL_GPISEGEDITING
#define INCL_GPITRANSFORMS
#define INCL_ERRORS
#include <os2.h>

/* GPI_ERROR, DEV_ERROR, DCTL_ERROR defined in pmgpi.h / pmdev.h */

/* ================================================================== */
/* DEV functions (ord 1-6, 165, 244, 729)                            */
/* ================================================================== */

HDC APIENTRY DevOpenDC(HAB hab, LONG lType, PCSZ pszToken,
                        LONG lCount, PDEVOPENDATA pdopData,
                        HDC hdcComp)
{ (void)hab;(void)lType;(void)pszToken;(void)lCount;(void)pdopData;(void)hdcComp;
  return DEV_ERROR; }
#pragma aux DevOpenDC "DEVOPENDC"

BOOL APIENTRY DevCloseDC(HDC hdc)
{ (void)hdc; return FALSE; }
#pragma aux DevCloseDC "DEVCLOSEDC"

LONG APIENTRY DevPostDeviceModes(HAB hab, PDRIVDATA pdrivdatOut,
                                   PCSZ pszDriverName, PCSZ pszDeviceName,
                                   PCSZ pszOutputName, ULONG fl)
{ (void)hab;(void)pdrivdatOut;(void)pszDriverName;(void)pszDeviceName;
  (void)pszOutputName;(void)fl; return DEV_ERROR; }
#pragma aux DevPostDeviceModes "DEVPOSTDEVICEMODES"

LONG APIENTRY DevEscape(HDC hdc, LONG lCode, LONG lInCount,
                          PBYTE pbInData, PLONG plOutCount,
                          PBYTE pbOutData)
{ (void)hdc;(void)lCode;(void)lInCount;(void)pbInData;
  (void)plOutCount;(void)pbOutData; return DEV_ERROR; }
#pragma aux DevEscape "DEVESCAPE"

BOOL APIENTRY DevQueryHardCopyCaps(HDC hdc, LONG lStartForm,
                                     LONG lForms, PHCINFO phciInfo)
{ (void)hdc;(void)lStartForm;(void)lForms;(void)phciInfo; return FALSE; }
#pragma aux DevQueryHardCopyCaps "DEVQUERYHARDCOPYCAPS"

BOOL APIENTRY DevQueryCaps(HDC hdc, LONG lStart,
                             LONG lCount, PLONG alArray)
{ (void)hdc;(void)lStart;(void)lCount;(void)alArray; return FALSE; }
#pragma aux DevQueryCaps "DEVQUERYCAPS"

BOOL APIENTRY DevQueryDeviceNames(HAB hab, PCSZ pszDriverName,
                                    PLONG pldn, PSTR32 aDeviceName,
                                    PSTR64 aDeviceDesc,
                                    PLONG pldt, PSTR16 aDataType)
{ (void)hab;(void)pszDriverName;(void)pldn;(void)aDeviceName;
  (void)aDeviceDesc;(void)pldt;(void)aDataType; return FALSE; }
#pragma aux DevQueryDeviceNames "DEVQUERYDEVICENAMES"

HDC APIENTRY DevStdOpen(PCSZ pszDeviceName)
{ (void)pszDeviceName; return DEV_ERROR; }
#pragma aux DevStdOpen "DEVSTDOPEN"

/* DevPostEscape: the pmdev.h prototype has an unusual signature that
 * varies between Toolkit versions. We provide a stub under an internal
 * name and export it as DEVPOSTESCAPE to avoid the prototype conflict. */
LONG DevPostEscapeStub(void) { return DEV_ERROR; }
#pragma aux DevPostEscapeStub "DevPostEscapeStub"

/* ================================================================== */
/* Presentation Space                                                  */
/* ================================================================== */

HPS APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions)
{ (void)hab;(void)hdc;(void)psizlSize;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiCreatePS "GPICREATEPS"

BOOL APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize)
{ (void)hps;(void)psizlSize; return FALSE; }
#pragma aux GpiQueryPS "GPIQUERYPS"

BOOL APIENTRY GpiDestroyPS(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiDestroyPS "GPIDESTROYPS"

BOOL APIENTRY GpiResetPS(HPS hps, ULONG flOptions)
{ (void)hps;(void)flOptions; return FALSE; }
#pragma aux GpiResetPS "GPIRESETPS"

LONG APIENTRY GpiSavePS(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiSavePS "GPISAVEPS"

BOOL APIENTRY GpiRestorePS(HPS hps, LONG lPSid)
{ (void)hps;(void)lPSid; return FALSE; }
#pragma aux GpiRestorePS "GPIRESTOREPS"

BOOL APIENTRY GpiAssociate(HPS hps, HDC hdc)
{ (void)hps;(void)hdc; return FALSE; }
#pragma aux GpiAssociate "GPIASSOCIATE"

BOOL APIENTRY GpiErrorSegmentData(HPS hps, PLONG plSegment, PLONG plContext)
{ (void)hps;(void)plSegment;(void)plContext; return FALSE; }
#pragma aux GpiErrorSegmentData "GPIERRORSEGMENTDATA"

BOOL APIENTRY GpiErase(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiErase "GPIERASE"

/* ================================================================== */
/* Drawing controls                                                    */
/* ================================================================== */

BOOL APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue)
{ (void)hps;(void)lControl;(void)lValue; return FALSE; }
#pragma aux GpiSetDrawControl "GPISETDRAWCONTROL"

LONG APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl)
{ (void)hps;(void)lControl; return GPI_ERROR; }
#pragma aux GpiQueryDrawControl "GPIQUERYDRAWCONTROL"

BOOL APIENTRY GpiDrawChain(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiDrawChain "GPIDRAWCHAIN"

BOOL APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment)
{ (void)hps;(void)lFirstSegment;(void)lLastSegment; return FALSE; }
#pragma aux GpiDrawFrom "GPIDRAWFROM"

BOOL APIENTRY GpiDrawSegment(HPS hps, LONG lSegment)
{ (void)hps;(void)lSegment; return FALSE; }
#pragma aux GpiDrawSegment "GPIDRAWSEGMENT"

BOOL APIENTRY GpiSetStopDraw(HPS hps, LONG lValue)
{ (void)hps;(void)lValue; return FALSE; }
#pragma aux GpiSetStopDraw "GPISETSTOPDRAW"

LONG APIENTRY GpiQueryStopDraw(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryStopDraw "GPIQUERYSTOPDRAW"

BOOL APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstTag, LONG lLastTag)
{ (void)hps;(void)lFirstTag;(void)lLastTag; return FALSE; }
#pragma aux GpiRemoveDynamics "GPIREMOVEDYNAMICS"

BOOL APIENTRY GpiDrawDynamics(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiDrawDynamics "GPIDRAWDYNAMICS"

BOOL APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode)
{ (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetDrawingMode "GPISETDRAWINGMODE"

LONG APIENTRY GpiQueryDrawingMode(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryDrawingMode "GPIQUERYDRAWINGMODE"

LONG APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset,
                          LONG lFormat, LONG lLength, PBYTE pbData)
{ (void)hps;(void)lSegid;(void)plOffset;(void)lFormat;
  (void)lLength;(void)pbData; return GPI_ERROR; }
#pragma aux GpiGetData "GPIGETDATA"

LONG APIENTRY GpiPutData(HPS hps, LONG lFormat,
                          PLONG plCount, PBYTE pbData)
{ (void)hps;(void)lFormat;(void)plCount;(void)pbData; return GPI_ERROR; }
#pragma aux GpiPutData "GPIPUTDATA"

/* ================================================================== */
/* Pick aperture                                                       */
/* ================================================================== */

BOOL APIENTRY GpiSetPickApertureSize(HPS hps, LONG lOptions, PSIZEL psizl)
{ (void)hps;(void)lOptions;(void)psizl; return FALSE; }
#pragma aux GpiSetPickApertureSize "GPISETPICKAPERTURESIZE"

BOOL APIENTRY GpiQueryPickApertureSize(HPS hps, PSIZEL psizlSize)
{ (void)hps;(void)psizlSize; return FALSE; }
#pragma aux GpiQueryPickApertureSize "GPIQUERYPICKAPERTURESIZE"

BOOL APIENTRY GpiSetPickAperturePosition(HPS hps, PPOINTL pptlPick)
{ (void)hps;(void)pptlPick; return FALSE; }
#pragma aux GpiSetPickAperturePosition "GPISETPICKAPERTUREPOSITION"

BOOL APIENTRY GpiQueryPickAperturePosition(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiQueryPickAperturePosition "GPIQUERYPICKAPERTUREPOSITION"

/* ================================================================== */
/* Tags and correlation                                                */
/* ================================================================== */

BOOL APIENTRY GpiSetTag(HPS hps, LONG lTag)
{ (void)hps;(void)lTag; return FALSE; }
#pragma aux GpiSetTag "GPISETTAG"

ULONG APIENTRY GpiQueryTag(HPS hps, PLONG plTag)
{ (void)hps;(void)plTag; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryTag "GPIQUERYTAG"

LONG APIENTRY GpiCorrelateChain(HPS hps, LONG lType, PPOINTL pptlPick,
                                  LONG lMaxHits, LONG lMaxDepth,
                                  PLONG alBuffer)
{ (void)hps;(void)lType;(void)pptlPick;(void)lMaxHits;
  (void)lMaxDepth;(void)alBuffer; return GPI_ERROR; }
#pragma aux GpiCorrelateChain "GPICORRELATECHAIN"

LONG APIENTRY GpiCorrelateFrom(HPS hps, LONG lFirstSeg, LONG lLastSeg,
                                 LONG lType, PPOINTL pptlPick,
                                 LONG lMaxHits, LONG lMaxDepth,
                                 PLONG alBuffer)
{ (void)hps;(void)lFirstSeg;(void)lLastSeg;(void)lType;(void)pptlPick;
  (void)lMaxHits;(void)lMaxDepth;(void)alBuffer; return GPI_ERROR; }
#pragma aux GpiCorrelateFrom "GPICORRELATEFROM"

LONG APIENTRY GpiCorrelateSegment(HPS hps, LONG lSegment, LONG lType,
                                    PPOINTL pptlPick, LONG lMaxDepth,
                                    LONG lMaxHits, PLONG alBuffer)
{ (void)hps;(void)lSegment;(void)lType;(void)pptlPick;
  (void)lMaxDepth;(void)lMaxHits;(void)alBuffer; return GPI_ERROR; }
#pragma aux GpiCorrelateSegment "GPICORRELATESEGMENT"

/* ================================================================== */
/* Boundary data                                                       */
/* ================================================================== */

BOOL APIENTRY GpiResetBoundaryData(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiResetBoundaryData "GPIRESETBOUNDARYDATA"

BOOL APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary)
{ (void)hps;(void)prclBoundary; return FALSE; }
#pragma aux GpiQueryBoundaryData "GPIQUERYBOUNDARYDATA"

/* ================================================================== */
/* Segments                                                            */
/* ================================================================== */

BOOL APIENTRY GpiOpenSegment(HPS hps, LONG lSegment)
{ (void)hps;(void)lSegment; return FALSE; }
#pragma aux GpiOpenSegment "GPIOPENSEGMENT"

BOOL APIENTRY GpiCloseSegment(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiCloseSegment "GPICLOSESEGMENT"

BOOL APIENTRY GpiDeleteSegment(HPS hps, LONG lSegid)
{ (void)hps;(void)lSegid; return FALSE; }
#pragma aux GpiDeleteSegment "GPIDELETESEGMENT"

BOOL APIENTRY GpiDeleteSegments(HPS hps, LONG lFirstSegment,
                                  LONG lLastSegment)
{ (void)hps;(void)lFirstSegment;(void)lLastSegment; return FALSE; }
#pragma aux GpiDeleteSegments "GPIDELETESEGMENTS"

LONG APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid,
                                     LONG lLastSegid, LONG lMax,
                                     PLONG alSegids)
{ (void)hps;(void)lFirstSegid;(void)lLastSegid;(void)lMax;
  (void)alSegids; return GPI_ERROR; }
#pragma aux GpiQuerySegmentNames "GPIQUERYSEGMENTNAMES"

BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttribute, LONG lValue)
{ (void)hps;(void)lAttribute;(void)lValue; return FALSE; }
#pragma aux GpiSetInitialSegmentAttrs "GPISETINITIALSEGMENTATTRS"

LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttribute)
{ (void)hps;(void)lAttribute; return GPI_ERROR; }
#pragma aux GpiQueryInitialSegmentAttrs "GPIQUERYINITIALSEGMENTATTRS"

BOOL APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegid,
                                   LONG lAttribute, LONG lValue)
{ (void)hps;(void)lSegid;(void)lAttribute;(void)lValue; return FALSE; }
#pragma aux GpiSetSegmentAttrs "GPISETSEGMENTATTRS"

LONG APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute)
{ (void)hps;(void)lSegid;(void)lAttribute; return GPI_ERROR; }
#pragma aux GpiQuerySegmentAttrs "GPIQUERYSEGMENTATTRS"

BOOL APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegid,
                                      LONG lRefSegid, LONG lOrder)
{ (void)hps;(void)lSegid;(void)lRefSegid;(void)lOrder; return FALSE; }
#pragma aux GpiSetSegmentPriority "GPISETSEGMENTPRIORITY"

LONG APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegid, LONG lOrder)
{ (void)hps;(void)lRefSegid;(void)lOrder; return GPI_ERROR; }
#pragma aux GpiQuerySegmentPriority "GPIQUERYSEGMENTPRIORITY"

/* ================================================================== */
/* Segment editing                                                     */
/* ================================================================== */

BOOL APIENTRY GpiSetEditMode(HPS hps, LONG lMode)
{ (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetEditMode "GPISETEDITMODE"

LONG APIENTRY GpiQueryEditMode(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryEditMode "GPIQUERYEDITMODE"

BOOL APIENTRY GpiSetElementPointer(HPS hps, LONG lElement)
{ (void)hps;(void)lElement; return FALSE; }
#pragma aux GpiSetElementPointer "GPISETELEMENTPOINTER"

LONG APIENTRY GpiQueryElementPointer(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryElementPointer "GPIQUERYELEMENTPOINTER"

BOOL APIENTRY GpiOffsetElementPointer(HPS hps, LONG lOffset)
{ (void)hps;(void)lOffset; return FALSE; }
#pragma aux GpiOffsetElementPointer "GPIOFFSETELEMENTPOINTER"

BOOL APIENTRY GpiDeleteElement(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiDeleteElement "GPIDELETEELEMENT"

BOOL APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirstElement,
                                      LONG lLastElement)
{ (void)hps;(void)lFirstElement;(void)lLastElement; return FALSE; }
#pragma aux GpiDeleteElementRange "GPIDELETEELEMENTRANGE"

BOOL APIENTRY GpiLabel(HPS hps, LONG lLabel)
{ (void)hps;(void)lLabel; return FALSE; }
#pragma aux GpiLabel "GPILABEL"

BOOL APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel)
{ (void)hps;(void)lLabel; return FALSE; }
#pragma aux GpiSetElementPointerAtLabel "GPISETELEMENTPOINTERATLABEL"

BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS hps, LONG lFirstLabel,
                                               LONG lLastLabel)
{ (void)hps;(void)lFirstLabel;(void)lLastLabel; return FALSE; }
#pragma aux GpiDeleteElementsBetweenLabels "GPIDELETEELEMENTSBETWEENLABELS"

LONG APIENTRY GpiQueryElementType(HPS hps, PLONG plType,
                                    LONG lLength, PBYTE pbData)
{ (void)hps;(void)plType;(void)lLength;(void)pbData; return GPI_ERROR; }
#pragma aux GpiQueryElementType "GPIQUERYELEMENTTYPE"

LONG APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength,
                                PBYTE pbData)
{ (void)hps;(void)lOff;(void)lMaxLength;(void)pbData; return GPI_ERROR; }
#pragma aux GpiQueryElement "GPIQUERYELEMENT"

BOOL APIENTRY GpiElement(HPS hps, LONG lType, PCSZ pszDesc,
                           LONG lLength, PBYTE pbData)
{ (void)hps;(void)lType;(void)pszDesc;(void)lLength;(void)pbData;
  return FALSE; }
#pragma aux GpiElement "GPIELEMENT"

BOOL APIENTRY GpiBeginElement(HPS hps, LONG lType, PCSZ pszDesc)
{ (void)hps;(void)lType;(void)pszDesc; return FALSE; }
#pragma aux GpiBeginElement "GPIBEGINELEMENT"

BOOL APIENTRY GpiEndElement(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiEndElement "GPIENDELEMENT"

/* ================================================================== */
/* Transforms                                                          */
/* ================================================================== */

BOOL APIENTRY GpiSetSegmentTransformMatrix(HPS hps, LONG lSegid,
                                             LONG lCount, PMATRIXLF pmatlfArray,
                                             LONG lOptions)
{ (void)hps;(void)lSegid;(void)lCount;(void)pmatlfArray;(void)lOptions;
  return FALSE; }
#pragma aux GpiSetSegmentTransformMatrix "GPISETSEGMENTTRANSFORMMATRIX"

BOOL APIENTRY GpiQuerySegmentTransformMatrix(HPS hps, LONG lSegid,
                                               LONG lCount,
                                               PMATRIXLF pmatlfArray)
{ (void)hps;(void)lSegid;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQuerySegmentTransformMatrix "GPIQUERYSEGMENTTRANSFORMMATRIX"

BOOL APIENTRY GpiSetModelTransformMatrix(HPS hps, LONG lCount,
                                           PMATRIXLF pmatlfArray, LONG lOptions)
{ (void)hps;(void)lCount;(void)pmatlfArray;(void)lOptions; return FALSE; }
#pragma aux GpiSetModelTransformMatrix "GPISETMODELTRANSFORMMATRIX"

BOOL APIENTRY GpiQueryModelTransformMatrix(HPS hps, LONG lCount,
                                             PMATRIXLF pmatlfArray)
{ (void)hps;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQueryModelTransformMatrix "GPIQUERYMODELTRANSFORMMATRIX"

LONG APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegment, LONG lCount,
                                     PMATRIXLF pmatlfArray, LONG lOptions)
{ (void)hps;(void)lSegment;(void)lCount;(void)pmatlfArray;(void)lOptions;
  return GPI_ERROR; }
#pragma aux GpiCallSegmentMatrix "GPICALLSEGMENTMATRIX"

BOOL APIENTRY GpiSetDefaultViewMatrix(HPS hps, LONG lCount,
                                        PMATRIXLF pmatlfArray, LONG lOptions)
{ (void)hps;(void)lCount;(void)pmatlfArray;(void)lOptions; return FALSE; }
#pragma aux GpiSetDefaultViewMatrix "GPISETDEFAULTVIEWMATRIX"

BOOL APIENTRY GpiQueryDefaultViewMatrix(HPS hps, LONG lCount,
                                          PMATRIXLF pmatlfArray)
{ (void)hps;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQueryDefaultViewMatrix "GPIQUERYDEFAULTVIEWMATRIX"

BOOL APIENTRY GpiSetPageViewport(HPS hps, PRECTL prclViewport)
{ (void)hps;(void)prclViewport; return FALSE; }
#pragma aux GpiSetPageViewport "GPISETPAGEVIEWPORT"

BOOL APIENTRY GpiQueryPageViewport(HPS hps, PRECTL prclViewport)
{ (void)hps;(void)prclViewport; return FALSE; }
#pragma aux GpiQueryPageViewport "GPIQUERYPAGEVIEWPORT"

BOOL APIENTRY GpiSetViewingTransformMatrix(HPS hps, LONG lCount,
                                             PMATRIXLF pmatlfArray,
                                             LONG lOptions)
{ (void)hps;(void)lCount;(void)pmatlfArray;(void)lOptions; return FALSE; }
#pragma aux GpiSetViewingTransformMatrix "GPISETVIEWINGTRANSFORMMATRIX"

BOOL APIENTRY GpiQueryViewingTransformMatrix(HPS hps, LONG lCount,
                                               PMATRIXLF pmatlfArray)
{ (void)hps;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQueryViewingTransformMatrix "GPIQUERYVIEWINGTRANSFORMMATRIX"

BOOL APIENTRY GpiSetGraphicsField(HPS hps, PRECTL prclField)
{ (void)hps;(void)prclField; return FALSE; }
#pragma aux GpiSetGraphicsField "GPISETGRAPHICSFIELD"

BOOL APIENTRY GpiQueryGraphicsField(HPS hps, PRECTL prclField)
{ (void)hps;(void)prclField; return FALSE; }
#pragma aux GpiQueryGraphicsField "GPIQUERYGRAPHICSFIELD"

BOOL APIENTRY GpiSetViewingLimits(HPS hps, PRECTL prclLimits)
{ (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiSetViewingLimits "GPISETVIEWINGLIMITS"

BOOL APIENTRY GpiQueryViewingLimits(HPS hps, PRECTL prclLimits)
{ (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiQueryViewingLimits "GPIQUERYVIEWINGLIMITS"

BOOL APIENTRY GpiConvert(HPS hps, LONG lSrc, LONG lTarg,
                           LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lSrc;(void)lTarg;(void)lCount;(void)aptlPoints;
  return FALSE; }
#pragma aux GpiConvert "GPICONVERT"

BOOL APIENTRY GpiConvertWithMatrix(HPS hps, LONG lCount,
                                     PPOINTL aptlPoints, LONG lMcount,
                                     PMATRIXLF pmatlfArray)
{ (void)hps;(void)lCount;(void)aptlPoints;(void)lMcount;(void)pmatlfArray;
  return FALSE; }
#pragma aux GpiConvertWithMatrix "GPICONVERTWITHMATRIX"

BOOL APIENTRY GpiTranslate(HPS hps, PMATRIXLF pmatlfArray,
                              LONG lOptions, PPOINTL pptlTranslate)
{ (void)hps;(void)pmatlfArray;(void)lOptions;(void)pptlTranslate;
  return FALSE; }
#pragma aux GpiTranslate "GPITRANSLATE"

BOOL APIENTRY GpiScale(HPS hps, PMATRIXLF pmatlfArray, LONG lOptions,
                         PFIXED afxScale, PPOINTL pptlCenter)
{ (void)hps;(void)pmatlfArray;(void)lOptions;(void)afxScale;(void)pptlCenter;
  return FALSE; }
#pragma aux GpiScale "GPISCALE"

BOOL APIENTRY GpiRotate(HPS hps, PMATRIXLF pmatlfArray, LONG lOptions,
                          FIXED fxAngle, PPOINTL pptlCenter)
{ (void)hps;(void)pmatlfArray;(void)lOptions;(void)fxAngle;(void)pptlCenter;
  return FALSE; }
#pragma aux GpiRotate "GPIROTATE"

/* ================================================================== */
/* Attributes                                                          */
/* ================================================================== */

BOOL APIENTRY GpiSetAttrMode(HPS hps, LONG lMode)
{ (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetAttrMode "GPISETATTRMODE"

LONG APIENTRY GpiQueryAttrMode(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryAttrMode "GPIQUERYATTRMODE"

BOOL APIENTRY GpiPop(HPS hps, LONG lCount)
{ (void)hps;(void)lCount; return FALSE; }
#pragma aux GpiPop "GPIPOP"

BOOL APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask,
                            ULONG flDefMask, PBUNDLE ppbunAttrs)
{ (void)hps;(void)lPrimType;(void)flAttrMask;(void)flDefMask;
  (void)ppbunAttrs; return FALSE; }
#pragma aux GpiSetAttrs "GPISETATTRS"

LONG APIENTRY GpiQueryAttrs(HPS hps, LONG lPrimType,
                              ULONG flAttrMask, PBUNDLE ppbunAttrs)
{ (void)hps;(void)lPrimType;(void)flAttrMask;(void)ppbunAttrs; return GPI_ERROR; }
#pragma aux GpiQueryAttrs "GPIQUERYATTRS"

/* ================================================================== */
/* Color tables                                                        */
/* ================================================================== */

BOOL APIENTRY GpiCreateLogColorTable(HPS hps, ULONG flOptions,
                                       LONG lFormat, LONG lStart,
                                       LONG lCount, PLONG alTable)
{ (void)hps;(void)flOptions;(void)lFormat;(void)lStart;
  (void)lCount;(void)alTable; return FALSE; }
#pragma aux GpiCreateLogColorTable "GPICREATELOGCOLORTABLE"

BOOL APIENTRY GpiRealizeColorTable(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiRealizeColorTable "GPIREALIZECOLORTABLE"

BOOL APIENTRY GpiUnrealizeColorTable(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiUnrealizeColorTable "GPIUNREALIZECOLORTABLE"

BOOL APIENTRY GpiQueryColorData(HPS hps, LONG lCount, PLONG alArray)
{ (void)hps;(void)lCount;(void)alArray; return FALSE; }
#pragma aux GpiQueryColorData "GPIQUERYCOLORDATA"

LONG APIENTRY GpiQueryLogColorTable(HPS hps, ULONG flOptions, LONG lStart,
                                      LONG lCount, PLONG alArray)
{ (void)hps;(void)flOptions;(void)lStart;(void)lCount;(void)alArray;
  return GPI_ERROR; }
#pragma aux GpiQueryLogColorTable "GPIQUERYLOGCOLORTABLE"

LONG APIENTRY GpiQueryRealColors(HPS hps, ULONG flOptions, LONG lStart,
                                   LONG lCount, PLONG alColors)
{ (void)hps;(void)flOptions;(void)lStart;(void)lCount;(void)alColors;
  return GPI_ERROR; }
#pragma aux GpiQueryRealColors "GPIQUERYREALCOLORS"

LONG APIENTRY GpiQueryNearestColor(HPS hps, ULONG flOptions, LONG lRgbIn)
{ (void)hps;(void)flOptions;(void)lRgbIn; return GPI_ERROR; }
#pragma aux GpiQueryNearestColor "GPIQUERYNEARESTCOLOR"

LONG APIENTRY GpiQueryColorIndex(HPS hps, ULONG flOptions, LONG lRgbColor)
{ (void)hps;(void)flOptions;(void)lRgbColor; return GPI_ERROR; }
#pragma aux GpiQueryColorIndex "GPIQUERYCOLORINDEX"

LONG APIENTRY GpiQueryRGBColor(HPS hps, ULONG flOptions, LONG lColorIndex)
{ (void)hps;(void)flOptions;(void)lColorIndex; return GPI_ERROR; }
#pragma aux GpiQueryRGBColor "GPIQUERYRGBCOLOR"

/* ================================================================== */
/* Color/mix primitives                                                */
/* ================================================================== */

BOOL APIENTRY GpiSetColor(HPS hps, LONG lColor)
{ (void)hps;(void)lColor; return FALSE; }
#pragma aux GpiSetColor "GPISETCOLOR"

LONG APIENTRY GpiQueryColor(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryColor "GPIQUERYCOLOR"

BOOL APIENTRY GpiSetBackColor(HPS hps, LONG lColor)
{ (void)hps;(void)lColor; return FALSE; }
#pragma aux GpiSetBackColor "GPISETBACKCOLOR"

LONG APIENTRY GpiQueryBackColor(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryBackColor "GPIQUERYBACKCOLOR"

BOOL APIENTRY GpiSetMix(HPS hps, LONG lMixMode)
{ (void)hps;(void)lMixMode; return FALSE; }
#pragma aux GpiSetMix "GPISETMIX"

LONG APIENTRY GpiQueryMix(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryMix "GPIQUERYMIX"

BOOL APIENTRY GpiSetBackMix(HPS hps, LONG lMixMode)
{ (void)hps;(void)lMixMode; return FALSE; }
#pragma aux GpiSetBackMix "GPISETBACKMIX"

LONG APIENTRY GpiQueryBackMix(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryBackMix "GPIQUERYBACKMIX"

/* ================================================================== */
/* Line attributes                                                     */
/* ================================================================== */

BOOL APIENTRY GpiSetLineType(HPS hps, LONG lLineType)
{ (void)hps;(void)lLineType; return FALSE; }
#pragma aux GpiSetLineType "GPISETLINETYPE"

LONG APIENTRY GpiQueryLineType(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryLineType "GPIQUERYLINETYPE"

BOOL APIENTRY GpiSetLineWidth(HPS hps, FIXED fxLineWidth)
{ (void)hps;(void)fxLineWidth; return FALSE; }
#pragma aux GpiSetLineWidth "GPISETLINEWIDTH"

FIXED APIENTRY GpiQueryLineWidth(HPS hps)
{ (void)hps; return (FIXED)GPI_ERROR; }
#pragma aux GpiQueryLineWidth "GPIQUERYLINEWIDTH"

BOOL APIENTRY GpiSetLineWidthGeom(HPS hps, LONG lLineWidth)
{ (void)hps;(void)lLineWidth; return FALSE; }
#pragma aux GpiSetLineWidthGeom "GPISETLINEWIDTHGEOM"

LONG APIENTRY GpiQueryLineWidthGeom(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryLineWidthGeom "GPIQUERYLINEWIDTHGEOM"

BOOL APIENTRY GpiSetLineEnd(HPS hps, LONG lLineEnd)
{ (void)hps;(void)lLineEnd; return FALSE; }
#pragma aux GpiSetLineEnd "GPISETLINEEND"

LONG APIENTRY GpiQueryLineEnd(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryLineEnd "GPIQUERYLINEEND"

BOOL APIENTRY GpiSetLineJoin(HPS hps, LONG lLineJoin)
{ (void)hps;(void)lLineJoin; return FALSE; }
#pragma aux GpiSetLineJoin "GPISETLINEJOIN"

LONG APIENTRY GpiQueryLineJoin(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryLineJoin "GPIQUERYLINEJOIN"

/* ================================================================== */
/* Current position and line drawing                                   */
/* ================================================================== */

BOOL APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiSetCurrentPosition "GPISETCURRENTPOSITION"

BOOL APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiQueryCurrentPosition "GPIQUERYCURRENTPOSITION"

BOOL APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiMove "GPIMOVE"

LONG APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint)
{ (void)hps;(void)pptlEndPoint; return GPI_ERROR; }
#pragma aux GpiLine "GPILINE"

LONG APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyLine "GPIPOLYLINE"

LONG APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyLineDisjoint "GPIPOLYLINEDISJOINT"

LONG APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlCorner,
                      LONG lHRound, LONG lVRound)
{ (void)hps;(void)lControl;(void)pptlCorner;(void)lHRound;(void)lVRound;
  return GPI_ERROR; }
#pragma aux GpiBox "GPIBOX"

LONG APIENTRY GpiPtVisible(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiPtVisible "GPIPTVISIBLE"

LONG APIENTRY GpiRectVisible(HPS hps, PRECTL prclRect)
{ (void)hps;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiRectVisible "GPIRECTVISIBLE"

/* ================================================================== */
/* Arcs and curves                                                     */
/* ================================================================== */

BOOL APIENTRY GpiSetArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiSetArcParams "GPISETARCPARAMS"

BOOL APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiQueryArcParams "GPIQUERYARCPARAMS"

LONG APIENTRY GpiPointArc(HPS hps, PPOINTL pptl)
{ (void)hps;(void)pptl; return GPI_ERROR; }
#pragma aux GpiPointArc "GPIPOINTARC"

LONG APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMult)
{ (void)hps;(void)lControl;(void)fxMult; return GPI_ERROR; }
#pragma aux GpiFullArc "GPIFULLARC"

LONG APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter, FIXED fxMult,
                               FIXED fxStartAngle, FIXED fxSweepAngle)
{ (void)hps;(void)pptlCenter;(void)fxMult;(void)fxStartAngle;
  (void)fxSweepAngle; return GPI_ERROR; }
#pragma aux GpiPartialArc "GPIPARTIALARC"

LONG APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyFillet "GPIPOLYFILLET"

LONG APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount,
                                    PPOINTL aptlPoints, PFIXED afxSharpness)
{ (void)hps;(void)lCount;(void)aptlPoints;(void)afxSharpness;
  return GPI_ERROR; }
#pragma aux GpiPolyFilletSharp "GPIPOLYFILLETSHARP"

LONG APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolySpline "GPIPOLYSPLINE"

/* ================================================================== */
/* Pattern/fill                                                        */
/* ================================================================== */

BOOL APIENTRY GpiSetBitmapId(HPS hps, HBITMAP hbm, LONG lLcid)
{ (void)hps;(void)hbm;(void)lLcid; return FALSE; }
#pragma aux GpiSetBitmapId "GPISETBITMAPID"

HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps, LONG lLcid)
{ (void)hps;(void)lLcid; return (HBITMAP)GPI_ERROR; }
#pragma aux GpiQueryBitmapHandle "GPIQUERYBITMAPHANDLE"

BOOL APIENTRY GpiSetPatternSet(HPS hps, LONG lSet)
{ (void)hps;(void)lSet; return FALSE; }
#pragma aux GpiSetPatternSet "GPISETPATTERNSET"

LONG APIENTRY GpiQueryPatternSet(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryPatternSet "GPIQUERYPATTERNSET"

BOOL APIENTRY GpiSetPattern(HPS hps, LONG lPattern)
{ (void)hps;(void)lPattern; return FALSE; }
#pragma aux GpiSetPattern "GPISETPATTERN"

LONG APIENTRY GpiQueryPattern(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryPattern "GPIQUERYPATTERN"

BOOL APIENTRY GpiSetPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{ (void)hps;(void)pptlRefPoint; return FALSE; }
#pragma aux GpiSetPatternRefPoint "GPISETPATTERNREFPOINT"

BOOL APIENTRY GpiQueryPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{ (void)hps;(void)pptlRefPoint; return FALSE; }
#pragma aux GpiQueryPatternRefPoint "GPIQUERYPATTERNREFPOINT"

BOOL APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{ (void)hps;(void)flOptions; return FALSE; }
#pragma aux GpiBeginArea "GPIBEGINAREA"

LONG APIENTRY GpiEndArea(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiEndArea "GPIENDAREA"

/* ================================================================== */
/* Fonts                                                               */
/* ================================================================== */

BOOL APIENTRY GpiLoadFonts(HAB hab, PCSZ pszFilename)
{ (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiLoadFonts "GPILOADFONTS"

BOOL APIENTRY GpiUnloadFonts(HAB hab, PCSZ pszFilename)
{ (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiUnloadFonts "GPIUNLOADFONTS"

LONG APIENTRY GpiCreateLogFont(HPS hps, PSTR8 pName, LONG lLcid,
                                  PFATTRS pfattrs)
{ (void)hps;(void)pName;(void)lLcid;(void)pfattrs; return GPI_ERROR; }
#pragma aux GpiCreateLogFont "GPICREATELOGFONT"

BOOL APIENTRY GpiDeleteSetId(HPS hps, LONG lLcid)
{ (void)hps;(void)lLcid; return FALSE; }
#pragma aux GpiDeleteSetId "GPIDELETESETID"

LONG APIENTRY GpiQueryNumberSetIds(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryNumberSetIds "GPIQUERYNUMBERSETIDS"

BOOL APIENTRY GpiQuerySetIds(HPS hps, LONG lCount, PLONG alTypes,
                               PSTR8 aNames, PLONG allcids)
{ (void)hps;(void)lCount;(void)alTypes;(void)aNames;(void)allcids;
  return FALSE; }
#pragma aux GpiQuerySetIds "GPIQUERYSETIDS"

LONG APIENTRY GpiQueryFonts(HPS hps, ULONG flOptions, PCSZ pszFacename,
                              PLONG plReqFonts, LONG lMetricsLength,
                              PFONTMETRICS afmMetrics)
{ (void)hps;(void)flOptions;(void)pszFacename;(void)plReqFonts;
  (void)lMetricsLength;(void)afmMetrics; return GPI_ERROR; }
#pragma aux GpiQueryFonts "GPIQUERYFONTS"

BOOL APIENTRY GpiQueryFontMetrics(HPS hps, LONG lMetricsLength,
                                    PFONTMETRICS pfmMetrics)
{ (void)hps;(void)lMetricsLength;(void)pfmMetrics; return FALSE; }
#pragma aux GpiQueryFontMetrics "GPIQUERYFONTMETRICS"

LONG APIENTRY GpiQueryKerningPairs(HPS hps, LONG lCount,
                                     PKERNINGPAIRS akrnprData)
{ (void)hps;(void)lCount;(void)akrnprData; return GPI_ERROR; }
#pragma aux GpiQueryKerningPairs "GPIQUERYKERNINGPAIRS"

BOOL APIENTRY GpiQueryWidthTable(HPS hps, LONG lFirstChar,
                                   LONG lCount, PLONG alData)
{ (void)hps;(void)lFirstChar;(void)lCount;(void)alData; return FALSE; }
#pragma aux GpiQueryWidthTable "GPIQUERYWIDTHTABLE"

BOOL APIENTRY GpiSetCp(HPS hps, ULONG ulCodePage)
{ (void)hps;(void)ulCodePage; return FALSE; }
#pragma aux GpiSetCp "GPISETCP"

ULONG APIENTRY GpiQueryCp(HPS hps)
{ (void)hps; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryCp "GPIQUERYCP"

BOOL APIENTRY GpiQueryTextBox(HPS hps, LONG lCount1, PCH pchString,
                                LONG lCount2, PPOINTL aptlPoints)
{ (void)hps;(void)lCount1;(void)pchString;(void)lCount2;(void)aptlPoints;
  return FALSE; }
#pragma aux GpiQueryTextBox "GPIQUERYTEXTBOX"

BOOL APIENTRY GpiQueryDefCharBox(HPS hps, PSIZEL psizlSize)
{ (void)hps;(void)psizlSize; return FALSE; }
#pragma aux GpiQueryDefCharBox "GPIQUERYDEFCHARBOX"

LONG APIENTRY GpiQueryFontFileDescriptions(HAB hab, PCSZ pszFilename,
                                              PLONG plCount,
                                              PFFDESCS affdescsNames)
{ (void)hab;(void)pszFilename;(void)plCount;(void)affdescsNames; return GPI_ERROR; }
#pragma aux GpiQueryFontFileDescriptions "GPIQUERYFONTFILEDESCRIPTIONS"

/* ================================================================== */
/* Character attributes                                                */
/* ================================================================== */

BOOL APIENTRY GpiSetCharSet(HPS hps, LONG lSet)
{ (void)hps;(void)lSet; return FALSE; }
#pragma aux GpiSetCharSet "GPISETCHARSET"

LONG APIENTRY GpiQueryCharSet(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryCharSet "GPIQUERYCHARSET"

BOOL APIENTRY GpiSetCharBox(HPS hps, PSIZEF psizfxBox)
{ (void)hps;(void)psizfxBox; return FALSE; }
#pragma aux GpiSetCharBox "GPISETCHARBOX"

BOOL APIENTRY GpiQueryCharBox(HPS hps, PSIZEF psizfxSize)
{ (void)hps;(void)psizfxSize; return FALSE; }
#pragma aux GpiQueryCharBox "GPIQUERYCHARBOX"

BOOL APIENTRY GpiSetCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{ (void)hps;(void)pgradlAngle; return FALSE; }
#pragma aux GpiSetCharAngle "GPISETCHARANGLE"

BOOL APIENTRY GpiQueryCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{ (void)hps;(void)pgradlAngle; return FALSE; }
#pragma aux GpiQueryCharAngle "GPIQUERYCHARANGLE"

BOOL APIENTRY GpiSetCharShear(HPS hps, PPOINTL pptlAngle)
{ (void)hps;(void)pptlAngle; return FALSE; }
#pragma aux GpiSetCharShear "GPISETCHARSHEAR"

BOOL APIENTRY GpiQueryCharShear(HPS hps, PPOINTL pptlShear)
{ (void)hps;(void)pptlShear; return FALSE; }
#pragma aux GpiQueryCharShear "GPIQUERYCHARSHEAR"

BOOL APIENTRY GpiSetCharDirection(HPS hps, LONG lDirection)
{ (void)hps;(void)lDirection; return FALSE; }
#pragma aux GpiSetCharDirection "GPISETCHARDIRECTION"

LONG APIENTRY GpiQueryCharDirection(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryCharDirection "GPIQUERYCHARDIRECTION"

BOOL APIENTRY GpiSetCharMode(HPS hps, LONG lMode)
{ (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetCharMode "GPISETCHARMODE"

LONG APIENTRY GpiQueryCharMode(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryCharMode "GPIQUERYCHARMODE"

BOOL APIENTRY GpiSetCharExtra(HPS hps, LONG lExtra)
{ (void)hps;(void)lExtra; return FALSE; }
#pragma aux GpiSetCharExtra "GPISETCHAREXTRA"

ULONG APIENTRY GpiQueryCharExtra(HPS hps, PLONG plExtra)
{ (void)hps;(void)plExtra; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryCharExtra "GPIQUERYCHAREXTRA"

BOOL APIENTRY GpiSetCharBreakExtra(HPS hps, LONG lBreakExtra)
{ (void)hps;(void)lBreakExtra; return FALSE; }
#pragma aux GpiSetCharBreakExtra "GPISETCHARBREAKEXTRA"

ULONG APIENTRY GpiQueryCharBreakExtra(HPS hps, PLONG plBreakExtra)
{ (void)hps;(void)plBreakExtra; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryCharBreakExtra "GPIQUERYCHARBREAKEXTRA"

/* ================================================================== */
/* Character output                                                    */
/* ================================================================== */

LONG APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString)
{ (void)hps;(void)lCount;(void)pchString; return GPI_ERROR; }
#pragma aux GpiCharString "GPICHARSTRING"

LONG APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint,
                                LONG lCount, PCH pchString)
{ (void)hps;(void)pptlPoint;(void)lCount;(void)pchString; return GPI_ERROR; }
#pragma aux GpiCharStringAt "GPICHARSTRINGAT"

LONG APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect, ULONG flOptions,
                                  LONG lCount, PCH pchString, PLONG alAdx)
{ (void)hps;(void)prclRect;(void)flOptions;(void)lCount;
  (void)pchString;(void)alAdx; return GPI_ERROR; }
#pragma aux GpiCharStringPos "GPICHARSTRINGPOS"

LONG APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlStart,
                                    PRECTL prclRect, ULONG flOptions,
                                    LONG lCount, PCH pchString, PLONG alAdx)
{ (void)hps;(void)pptlStart;(void)prclRect;(void)flOptions;
  (void)lCount;(void)pchString;(void)alAdx; return GPI_ERROR; }
#pragma aux GpiCharStringPosAt "GPICHARSTRINGPOSAT"

BOOL APIENTRY GpiQueryCharStringPos(HPS hps, ULONG flOptions, LONG lCount,
                                      PCH pchString, PLONG alAdx,
                                      PPOINTL aptlPos)
{ (void)hps;(void)flOptions;(void)lCount;(void)pchString;
  (void)alAdx;(void)aptlPos; return FALSE; }
#pragma aux GpiQueryCharStringPos "GPIQUERYCHARSTRINGPOS"

BOOL APIENTRY GpiQueryCharStringPosAt(HPS hps, PPOINTL pptlStart,
                                         ULONG flOptions, LONG lCount,
                                         PCH pchString, PLONG alAdx,
                                         PPOINTL aptlPos)
{ (void)hps;(void)pptlStart;(void)flOptions;(void)lCount;
  (void)pchString;(void)alAdx;(void)aptlPos; return FALSE; }
#pragma aux GpiQueryCharStringPosAt "GPIQUERYCHARSTRINGPOSAT"

/* ================================================================== */
/* Markers                                                             */
/* ================================================================== */

BOOL APIENTRY GpiSetMarkerSet(HPS hps, LONG lSet)
{ (void)hps;(void)lSet; return FALSE; }
#pragma aux GpiSetMarkerSet "GPISETMARKERSET"

LONG APIENTRY GpiQueryMarkerSet(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryMarkerSet "GPIQUERYMARKERSET"

BOOL APIENTRY GpiSetMarker(HPS hps, LONG lSymbol)
{ (void)hps;(void)lSymbol; return FALSE; }
#pragma aux GpiSetMarker "GPISETMARKER"

LONG APIENTRY GpiQueryMarker(HPS hps)
{ (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryMarker "GPIQUERYMARKER"

BOOL APIENTRY GpiSetMarkerBox(HPS hps, PSIZEF psizfxSize)
{ (void)hps;(void)psizfxSize; return FALSE; }
#pragma aux GpiSetMarkerBox "GPISETMARKERBOX"

BOOL APIENTRY GpiQueryMarkerBox(HPS hps, PSIZEF psizfxSize)
{ (void)hps;(void)psizfxSize; return FALSE; }
#pragma aux GpiQueryMarkerBox "GPIQUERYMARKERBOX"

LONG APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiMarker "GPIMARKER"

LONG APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyMarker "GPIPOLYMARKER"

/* ================================================================== */
/* Image                                                               */
/* ================================================================== */

LONG APIENTRY GpiImage(HPS hps, LONG lFormat, PSIZEL psizlImageSize,
                         LONG lLength, PBYTE pbData)
{ (void)hps;(void)lFormat;(void)psizlImageSize;(void)lLength;(void)pbData;
  return GPI_ERROR; }
#pragma aux GpiImage "GPIIMAGE"

/* ================================================================== */
/* Bitmaps                                                             */
/* ================================================================== */

HBITMAP APIENTRY GpiCreateBitmap(HPS hps, PBITMAPINFOHEADER2 pbmpData,
                                    ULONG fl, PBYTE pbInitData,
                                    PBITMAPINFO2 pbmiInfoTable)
{ (void)hps;(void)pbmpData;(void)fl;(void)pbInitData;(void)pbmiInfoTable;
  return (HBITMAP)GPI_ERROR; }
#pragma aux GpiCreateBitmap "GPICREATEBITMAP"

BOOL APIENTRY GpiDeleteBitmap(HBITMAP hbm)
{ (void)hbm; return FALSE; }
#pragma aux GpiDeleteBitmap "GPIDELETEBITMAP"

HBITMAP APIENTRY GpiSetBitmap(HPS hps, HBITMAP hbm)
{ (void)hps;(void)hbm; return (HBITMAP)GPI_ERROR; }
#pragma aux GpiSetBitmap "GPISETBITMAP"

BOOL APIENTRY GpiSetBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{ (void)hbm;(void)psizlBitmapDimension; return FALSE; }
#pragma aux GpiSetBitmapDimension "GPISETBITMAPDIMENSION"

BOOL APIENTRY GpiQueryBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{ (void)hbm;(void)psizlBitmapDimension; return FALSE; }
#pragma aux GpiQueryBitmapDimension "GPIQUERYBITMAPDIMENSION"

BOOL APIENTRY GpiQueryDeviceBitmapFormats(HPS hps, LONG lCount,
                                            PLONG alArray)
{ (void)hps;(void)lCount;(void)alArray; return FALSE; }
#pragma aux GpiQueryDeviceBitmapFormats "GPIQUERYDEVICEBITMAPFORMATS"

BOOL APIENTRY GpiQueryBitmapParameters(HBITMAP hbm,
                                          PBITMAPINFOHEADER pbmpData)
{ (void)hbm;(void)pbmpData; return FALSE; }
#pragma aux GpiQueryBitmapParameters "GPIQUERYBITMAPPARAMETERS"

BOOL APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm,
                                          PBITMAPINFOHEADER2 pbmpData)
{ (void)hbm;(void)pbmpData; return FALSE; }
#pragma aux GpiQueryBitmapInfoHeader "GPIQUERYBITMAPINFOHEADER"

LONG APIENTRY GpiSetBitmapBits(HPS hps, LONG lScanStart, LONG lScans,
                                  PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{ (void)hps;(void)lScanStart;(void)lScans;(void)pbBuffer;(void)pbmiInfoTable;
  return GPI_ERROR; }
#pragma aux GpiSetBitmapBits "GPISETBITMAPBITS"

LONG APIENTRY GpiQueryBitmapBits(HPS hps, LONG lScanStart, LONG lScans,
                                    PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{ (void)hps;(void)lScanStart;(void)lScans;(void)pbBuffer;(void)pbmiInfoTable;
  return GPI_ERROR; }
#pragma aux GpiQueryBitmapBits "GPIQUERYBITMAPBITS"

LONG APIENTRY GpiBitBlt(HPS hpsTarget, HPS hpsSource, LONG lCount,
                          PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{ (void)hpsTarget;(void)hpsSource;(void)lCount;(void)aptlPoints;
  (void)lRop;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiBitBlt "GPIBITBLT"

LONG APIENTRY GpiWCBitBlt(HPS hpsTarget, HBITMAP hbmSource, LONG lCount,
                             PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{ (void)hpsTarget;(void)hbmSource;(void)lCount;(void)aptlPoints;
  (void)lRop;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiWCBitBlt "GPIWCBITBLT"

LONG APIENTRY GpiDrawBits(HPS hps, PVOID pBits,
                            PBITMAPINFO2 pbmiInfoTable, LONG lCount,
                            PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{ (void)hps;(void)pBits;(void)pbmiInfoTable;(void)lCount;
  (void)aptlPoints;(void)lRop;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiDrawBits "GPIDRAWBITS"

LONG APIENTRY GpiSetPel(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiSetPel "GPISETPEL"

LONG APIENTRY GpiQueryPel(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiQueryPel "GPIQUERYPEL"

LONG APIENTRY GpiFloodFill(HPS hps, LONG lOptions, LONG lColor)
{ (void)hps;(void)lOptions;(void)lColor; return GPI_ERROR; }
#pragma aux GpiFloodFill "GPIFLOODFILL"

/* ================================================================== */
/* Regions                                                             */
/* ================================================================== */

HRGN APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles)
{ (void)hps;(void)lCount;(void)arclRectangles; return (HRGN)GPI_ERROR; }
#pragma aux GpiCreateRegion "GPICREATEREGION"

BOOL APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lCount,
                              PRECTL arclRectangles)
{ (void)hps;(void)hrgn;(void)lCount;(void)arclRectangles; return FALSE; }
#pragma aux GpiSetRegion "GPISETREGION"

BOOL APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn)
{ (void)hps;(void)hrgn; return FALSE; }
#pragma aux GpiDestroyRegion "GPIDESTROYREGION"

LONG APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest,
                                  HRGN hrgnSrc1, HRGN hrgnSrc2, LONG lMode)
{ (void)hps;(void)hrgnDest;(void)hrgnSrc1;(void)hrgnSrc2;(void)lMode;
  return GPI_ERROR; }
#pragma aux GpiCombineRegion "GPICOMBINEREGION"

LONG APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2)
{ (void)hps;(void)hrgnSrc1;(void)hrgnSrc2; return GPI_ERROR; }
#pragma aux GpiEqualRegion "GPIEQUALREGION"

BOOL APIENTRY GpiOffsetRegion(HPS hps, HRGN hrgn, PPOINTL pptlOffset)
{ (void)hps;(void)hrgn;(void)pptlOffset; return FALSE; }
#pragma aux GpiOffsetRegion "GPIOFFSETREGION"

LONG APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint)
{ (void)hps;(void)hrgn;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiPtInRegion "GPIPTINREGION"

LONG APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect)
{ (void)hps;(void)hrgn;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiRectInRegion "GPIRECTINREGION"

LONG APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound)
{ (void)hps;(void)hrgn;(void)prclBound; return GPI_ERROR; }
#pragma aux GpiQueryRegionBox "GPIQUERYREGIONBOX"

BOOL APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound,
                                     PRGNRECT prgnrcControl, PRECTL arclRects)
{ (void)hps;(void)hrgn;(void)prclBound;(void)prgnrcControl;(void)arclRects;
  return FALSE; }
#pragma aux GpiQueryRegionRects "GPIQUERYREGIONRECTS"

LONG APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld)
{ (void)hps;(void)hrgn;(void)phrgnOld; return GPI_ERROR; }
#pragma aux GpiSetClipRegion "GPISETCLIPREGION"

HRGN APIENTRY GpiQueryClipRegion(HPS hps)
{ (void)hps; return (HRGN)GPI_ERROR; }
#pragma aux GpiQueryClipRegion "GPIQUERYCLIPREGION"

LONG APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound)
{ (void)hps;(void)prclBound; return GPI_ERROR; }
#pragma aux GpiQueryClipBox "GPIQUERYCLIPBOX"

LONG APIENTRY GpiIntersectClipRectangle(HPS hps, PRECTL prclRect)
{ (void)hps;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiIntersectClipRectangle "GPIINTERSECTCLIPRECTANGLE"

LONG APIENTRY GpiExcludeClipRectangle(HPS hps, PRECTL prclRect)
{ (void)hps;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiExcludeClipRectangle "GPIEXCLUDECLIPRECTANGLE"

LONG APIENTRY GpiOffsetClipRegion(HPS hps, PPOINTL pptlPoint)
{ (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiOffsetClipRegion "GPIOFFSETCLIPREGION"

LONG APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn)
{ (void)hps;(void)hrgn; return GPI_ERROR; }
#pragma aux GpiPaintRegion "GPIPAINTREGION"

LONG APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness)
{ (void)hps;(void)hrgn;(void)thickness; return GPI_ERROR; }
#pragma aux GpiFrameRegion "GPIFRAMEREGION"

/* ================================================================== */
/* Metafiles                                                           */
/* ================================================================== */

HMF APIENTRY GpiLoadMetaFile(HAB hab, PCSZ pszFilename)
{ (void)hab;(void)pszFilename; return (HMF)GPI_ERROR; }
#pragma aux GpiLoadMetaFile "GPILOADMETAFILE"

HMF APIENTRY GpiCopyMetaFile(HMF hmf)
{ (void)hmf; return (HMF)GPI_ERROR; }
#pragma aux GpiCopyMetaFile "GPICOPYMETAFILE"

LONG APIENTRY GpiPlayMetaFile(HPS hps, HMF hmf, LONG lCount1,
                                PLONG alOptarray, PLONG plSegCount,
                                LONG lCount2, PCSZ pszDesc)
{ (void)hps;(void)hmf;(void)lCount1;(void)alOptarray;(void)plSegCount;
  (void)lCount2;(void)pszDesc; return GPI_ERROR; }
#pragma aux GpiPlayMetaFile "GPIPLAYMETAFILE"

BOOL APIENTRY GpiSaveMetaFile(HMF hmf, PCSZ pszFilename)
{ (void)hmf;(void)pszFilename; return FALSE; }
#pragma aux GpiSaveMetaFile "GPISAVEMETAFILE"

BOOL APIENTRY GpiDeleteMetaFile(HMF hmf)
{ (void)hmf; return FALSE; }
#pragma aux GpiDeleteMetaFile "GPIDELETEMETAFILE"

ULONG APIENTRY GpiQueryMetaFileBits(HMF hmf, LONG lOffset,
                                      LONG lLength, PBYTE pbData)
{ (void)hmf;(void)lOffset;(void)lLength;(void)pbData; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryMetaFileBits "GPIQUERYMETAFILEBITS"

BOOL APIENTRY GpiSetMetaFileBits(HMF hmf, LONG lOffset,
                                    LONG lLength, PBYTE pbBuffer)
{ (void)hmf;(void)lOffset;(void)lLength;(void)pbBuffer; return FALSE; }
#pragma aux GpiSetMetaFileBits "GPISETMETAFILEBITS"

LONG APIENTRY GpiQueryMetaFileLength(HMF hmf)
{ (void)hmf; return GPI_ERROR; }
#pragma aux GpiQueryMetaFileLength "GPIQUERYMETAFILELENGTH"

HMF APIENTRY GpiOpenMetaFile(HAB hab)
{ (void)hab; return (HMF)GPI_ERROR; }
#pragma aux GpiOpenMetaFile "GPIOPENMETAFILE"

BOOL APIENTRY GpiCloseMetaFile(HMF hmf)
{ (void)hmf; return FALSE; }
#pragma aux GpiCloseMetaFile "GPICLOSEMETAFILE"

BOOL APIENTRY GpiAccessMetaFile(HMF hmf)
{ (void)hmf; return FALSE; }
#pragma aux GpiAccessMetaFile "GPIACCESSMETAFILE"

BOOL APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData)
{ (void)hps;(void)lLength;(void)pbData; return FALSE; }
#pragma aux GpiComment "GPICOMMENT"

/* ================================================================== */
/* Paths and clipping                                                  */
/* ================================================================== */

BOOL APIENTRY GpiSetClipPath(HPS hps, LONG lPath, LONG lOptions)
{ (void)hps;(void)lPath;(void)lOptions; return FALSE; }
#pragma aux GpiSetClipPath "GPISETCLIPPATH"

BOOL APIENTRY GpiBeginPath(HPS hps, LONG lPath)
{ (void)hps;(void)lPath; return FALSE; }
#pragma aux GpiBeginPath "GPIBEGINPATH"

BOOL APIENTRY GpiEndPath(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiEndPath "GPIENDPATH"

BOOL APIENTRY GpiCloseFigure(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiCloseFigure "GPICLOSEFIGURE"

BOOL APIENTRY GpiModifyPath(HPS hps, LONG lPath, LONG lMode)
{ (void)hps;(void)lPath;(void)lMode; return FALSE; }
#pragma aux GpiModifyPath "GPIMODIFYPATH"

LONG APIENTRY GpiFillPath(HPS hps, LONG lPath, LONG lOptions)
{ (void)hps;(void)lPath;(void)lOptions; return GPI_ERROR; }
#pragma aux GpiFillPath "GPIFILLPATH"

LONG APIENTRY GpiStrokePath(HPS hps, LONG lPath, ULONG flOptions)
{ (void)hps;(void)lPath;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiStrokePath "GPISTROKEPATH"

LONG APIENTRY GpiOutlinePath(HPS hps, LONG lPath, LONG lOptions)
{ (void)hps;(void)lPath;(void)lOptions; return GPI_ERROR; }
#pragma aux GpiOutlinePath "GPIOUTLINEPATH"

HRGN APIENTRY GpiPathToRegion(HPS hps, LONG lPath, LONG lOptions)
{ (void)hps;(void)lPath;(void)lOptions; return (HRGN)GPI_ERROR; }
#pragma aux GpiPathToRegion "GPIPATHTOREGION"

/* ================================================================== */
/* Miscellaneous GPI                                                   */
/* ================================================================== */

HDC APIENTRY GpiQueryDevice(HPS hps)
{ (void)hps; return (HDC)GPI_ERROR; }
#pragma aux GpiQueryDevice "GPIQUERYDEVICE"

BOOL APIENTRY GpiVectorSymbol(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ (void)hps;(void)lCount;(void)aptlPoints; return FALSE; }
#pragma aux GpiVectorSymbol "GPIVECTORSYMBOL"

BOOL APIENTRY GpiConvPSH(HPS hps, LONG lAction, PVOID pvConvData)
{ (void)hps;(void)lAction;(void)pvConvData; return FALSE; }
#pragma aux GpiConvPSH "GPICONVPSH"

BOOL APIENTRY GpiSetPS(HPS hps, PSIZEL psizl, ULONG fl)
{ (void)hps;(void)psizl;(void)fl; return FALSE; }
#pragma aux GpiSetPS "GPISETPS"

BOOL APIENTRY GpiSetDCOwner(HDC hdc, ULONG idProcess)
{ (void)hdc;(void)idProcess; return FALSE; }
#pragma aux GpiSetDCOwner "GPISETDCOWNER"

BOOL APIENTRY GpiSetBitmapOwner(HBITMAP hbm, ULONG idProcess)
{ (void)hbm;(void)idProcess; return FALSE; }
#pragma aux GpiSetBitmapOwner "GPISETBITMAPOWNER"

BOOL APIENTRY GpiSetRegionOwner(HPS hps, HRGN hrgn, ULONG idProcess)
{ (void)hps;(void)hrgn;(void)idProcess; return FALSE; }
#pragma aux GpiSetRegionOwner "GPISETREGIONOWNER"

BOOL APIENTRY GpiInstallIFIFont(HAB hab, PCSZ pszFileName)
{ (void)hab;(void)pszFileName; return FALSE; }
#pragma aux GpiInstallIFIFont "GPIINSTALLIFIFONT"

/* ================================================================== */
/* Default view / def attrs                                            */
/* ================================================================== */

BOOL APIENTRY GpiSetDefTag(HPS hps, LONG lTag)
{ (void)hps;(void)lTag; return FALSE; }
#pragma aux GpiSetDefTag "GPISETDEFTAG"

ULONG APIENTRY GpiQueryDefTag(HPS hps, PLONG plTag)
{ (void)hps;(void)plTag; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryDefTag "GPIQUERYDEFTAG"

BOOL APIENTRY GpiSetDefAttrs(HPS hps, LONG lPrimType,
                               ULONG flAttrMask, PBUNDLE ppbunAttrs)
{ (void)hps;(void)lPrimType;(void)flAttrMask;(void)ppbunAttrs; return FALSE; }
#pragma aux GpiSetDefAttrs "GPISETDEFATTRS"

BOOL APIENTRY GpiQueryDefAttrs(HPS hps, LONG lPrimType,
                                  ULONG flAttrMask, PBUNDLE ppbunAttrs)
{ (void)hps;(void)lPrimType;(void)flAttrMask;(void)ppbunAttrs; return FALSE; }
#pragma aux GpiQueryDefAttrs "GPIQUERYDEFATTRS"

BOOL APIENTRY GpiSetDefViewingLimits(HPS hps, PRECTL prclLimits)
{ (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiSetDefViewingLimits "GPISETDEFVIEWINGLIMITS"

BOOL APIENTRY GpiQueryDefViewingLimits(HPS hps, PRECTL prclLimits)
{ (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiQueryDefViewingLimits "GPIQUERYDEFVIEWINGLIMITS"

BOOL APIENTRY GpiSetDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiSetDefArcParams "GPISETDEFARCPARAMS"

BOOL APIENTRY GpiQueryDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiQueryDefArcParams "GPIQUERYDEFARCPARAMS"

/* ================================================================== */
/* Palette                                                             */
/* ================================================================== */

HPAL APIENTRY GpiCreatePalette(HAB hab, ULONG flOptions, ULONG ulFormat,
                                  ULONG ulCount, PULONG aulTable)
{ (void)hab;(void)flOptions;(void)ulFormat;(void)ulCount;(void)aulTable;
  return (HPAL)GPI_ERROR; }
#pragma aux GpiCreatePalette "GPICREATEPALETTE"

BOOL APIENTRY GpiDeletePalette(HPAL hpal)
{ (void)hpal; return FALSE; }
#pragma aux GpiDeletePalette "GPIDELETEPALETTE"

HPAL APIENTRY GpiSelectPalette(HPS hps, HPAL hpal)
{ (void)hps;(void)hpal; return (HPAL)GPI_ERROR; }
#pragma aux GpiSelectPalette "GPISELECTPALETTE"

LONG APIENTRY GpiAnimatePalette(HPS hps, HPAL hpal, ULONG ulFormat,
                                   ULONG ulStart, PULONG aulTable)
{ (void)hps;(void)hpal;(void)ulFormat;(void)ulStart;
  (void)aulTable; return GPI_ERROR; }
#pragma aux GpiAnimatePalette "GPIANIMATEPALETTE"

BOOL APIENTRY GpiSetPaletteEntries(HPAL hpal, ULONG ulFormat,
                                      ULONG ulStart, ULONG ulCount,
                                      PULONG aulTable)
{ (void)hpal;(void)ulFormat;(void)ulStart;(void)ulCount;(void)aulTable;
  return FALSE; }
#pragma aux GpiSetPaletteEntries "GPISETPALETTEENTRIES"

HPAL APIENTRY GpiQueryPalette(HPS hps)
{ (void)hps; return (HPAL)GPI_ERROR; }
#pragma aux GpiQueryPalette "GPIQUERYPALETTE"

LONG APIENTRY GpiQueryPaletteInfo(HPAL hpal, HPS hps, ULONG flOptions,
                                     ULONG ulStart, ULONG ulCount,
                                     PULONG aulArray)
{ (void)hpal;(void)hps;(void)flOptions;(void)ulStart;
  (void)ulCount;(void)aulArray; return GPI_ERROR; }
#pragma aux GpiQueryPaletteInfo "GPIQUERYPALETTEINFO"

/* ================================================================== */
/* Public fonts                                                        */
/* ================================================================== */

BOOL APIENTRY GpiLoadPublicFonts(HAB hab, PCSZ pszFilename)
{ (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiLoadPublicFonts "GPILOADPUBLICFONTS"

BOOL APIENTRY GpiUnloadPublicFonts(HAB hab, PCSZ pszFilename)
{ (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiUnloadPublicFonts "GPIUNLOADPUBLICFONTS"

BOOL APIENTRY GpiQueryLogicalFont(HPS hps, LONG lLcid, PSTR8 pName,
                                     PFATTRS pfAttrs, LONG lLength)
{ (void)hps;(void)lLcid;(void)pName;(void)pfAttrs;(void)lLength; return FALSE;}
#pragma aux GpiQueryLogicalFont "GPIQUERYLOGICALFONT"

BOOL APIENTRY GpiQueryFaceString(HPS hps, PCSZ pszFamilyname,
                                    PFACENAMEDESC pfnd, LONG lLength,
                                    PCSZ pszFacename)
{ (void)hps;(void)pszFamilyname;(void)pfnd;(void)lLength;(void)pszFacename;
  return FALSE; }
#pragma aux GpiQueryFaceString "GPIQUERYFACESTRING"

ULONG APIENTRY GpiQueryFontAction(HAB hab, ULONG ulOptions)
{ (void)hab;(void)ulOptions; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryFontAction "GPIQUERYFONTACTION"

/* ================================================================== */
/* Play/suspend                                                        */
/* ================================================================== */

BOOL APIENTRY GpiSuspendPlay(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiSuspendPlay "GPISUSPENDPLAY"

BOOL APIENTRY GpiResumePlay(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiResumePlay "GPIRESUMEPLAY"

/* ================================================================== */
/* MT (metafile/graphics streaming) functions                          */
/* ================================================================== */

BOOL APIENTRY MtStartReadRequest(PVOID pvMtCtl)
{ (void)pvMtCtl; return FALSE; }
#pragma aux MtStartReadRequest "MTSTARTREADREQUEST"

BOOL APIENTRY MtEndReadRequest(PVOID pvMtCtl)
{ (void)pvMtCtl; return FALSE; }
#pragma aux MtEndReadRequest "MTENDREADREQUEST"

BOOL APIENTRY MtGetDescription(PVOID pvMtCtl, PVOID pvDesc)
{ (void)pvMtCtl;(void)pvDesc; return FALSE; }
#pragma aux MtGetDescription "MTGETDESCRIPTION"

BOOL APIENTRY MtGetCodePage(PVOID pvMtCtl, PULONG pulCP)
{ (void)pvMtCtl;(void)pulCP; return FALSE; }
#pragma aux MtGetCodePage "MTGETCODEPAGE"

BOOL APIENTRY MtGetLCT(PVOID pvMtCtl, PVOID pvLCT)
{ (void)pvMtCtl;(void)pvLCT; return FALSE; }
#pragma aux MtGetLCT "MTGETLCT"

BOOL APIENTRY MtGetGDDInfo(PVOID pvMtCtl, PVOID pvGDDInfo)
{ (void)pvMtCtl;(void)pvGDDInfo; return FALSE; }
#pragma aux MtGetGDDInfo "MTGETGDDINFO"

BOOL APIENTRY MtGetFirstFont(PVOID pvMtCtl, PVOID pvFontData)
{ (void)pvMtCtl;(void)pvFontData; return FALSE; }
#pragma aux MtGetFirstFont "MTGETFIRSTFONT"

BOOL APIENTRY MtGetNextFont(PVOID pvMtCtl, PVOID pvFontData)
{ (void)pvMtCtl;(void)pvFontData; return FALSE; }
#pragma aux MtGetNextFont "MTGETNEXTFONT"

BOOL APIENTRY MtGetFirstGraphicsData(PVOID pvMtCtl, PVOID pvGfxData)
{ (void)pvMtCtl;(void)pvGfxData; return FALSE; }
#pragma aux MtGetFirstGraphicsData "MTGETFIRSTGRAPHICSDATA"

BOOL APIENTRY MtGetNextGraphicsData(PVOID pvMtCtl, PVOID pvGfxData)
{ (void)pvMtCtl;(void)pvGfxData; return FALSE; }
#pragma aux MtGetNextGraphicsData "MTGETNEXTGRAPHICSDATA"

/* ================================================================== */
/* SEGSW segment window operations                                     */
/* ================================================================== */

BOOL APIENTRY SegSGWOpenSegmentWindow(PVOID pvSGWCtl, PVOID pvWinData)
{ (void)pvSGWCtl;(void)pvWinData; return FALSE; }
#pragma aux SegSGWOpenSegmentWindow "SEGSGWOPENSEGMENTWINDOW"

BOOL APIENTRY SegSGWNewPartData(PVOID pvSGWCtl, PVOID pvPartData)
{ (void)pvSGWCtl;(void)pvPartData; return FALSE; }
#pragma aux SegSGWNewPartData "SEGSGWNEWPARTDATA"

BOOL APIENTRY SegSGWNextOrderF(PVOID pvSGWCtl, PVOID pvOrderData)
{ (void)pvSGWCtl;(void)pvOrderData; return FALSE; }
#pragma aux SegSGWNextOrderF "SEGSGWNEXTORDERF"

/* ================================================================== */
/* MT association                                                      */
/* ================================================================== */

BOOL APIENTRY GpiMtAssociate(HPS hps, PVOID pvMtCtl)
{ (void)hps;(void)pvMtCtl; return FALSE; }
#pragma aux GpiMtAssociate "GPIMTASSOCIATE"

BOOL APIENTRY GpiMtDisassociate(HPS hps, PVOID pvMtCtl)
{ (void)hps;(void)pvMtCtl; return FALSE; }
#pragma aux GpiMtDisassociate "GPIMTDISASSOCIATE"

/* ================================================================== */
/* Format conversion                                                   */
/* ================================================================== */

BOOL APIENTRY FmtConvertGOCAPolyLine(PVOID pvIn, PVOID pvOut, PULONG pulLen)
{ (void)pvIn;(void)pvOut;(void)pulLen; return FALSE; }
#pragma aux FmtConvertGOCAPolyLine "FMTCONVERTGOCAPOLY"

/* ================================================================== */
/* Ink path                                                            */
/* ================================================================== */

BOOL APIENTRY GpiBeginInkPath(HPS hps, LONG lPath, ULONG flOptions)
{ (void)hps;(void)lPath;(void)flOptions; return FALSE; }
#pragma aux GpiBeginInkPath "GPIBEGININKPATH"

BOOL APIENTRY GpiEndInkPath(HPS hps)
{ (void)hps; return FALSE; }
#pragma aux GpiEndInkPath "GPIENDINKPATH"

BOOL APIENTRY GpiStrokeInkPath(HPS hps, LONG lPath, ULONG flOptions)
{ (void)hps;(void)lPath;(void)flOptions; return FALSE; }
#pragma aux GpiStrokeInkPath "GPISTROKEINKPATH"

/* ================================================================== */
/* DSP                                                                 */
/* ================================================================== */

BOOL APIENTRY DspInitSystemDriverName(PCSZ pszDriverName)
{ (void)pszDriverName; return FALSE; }
#pragma aux DspInitSystemDriverName "DSPINITSYSTEMDRIVERNAME"
