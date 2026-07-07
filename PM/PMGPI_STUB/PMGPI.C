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
 * All 298 exported functions are stubs. Each returns its DOCUMENTED failure
 * value per the GPI Reference (not a blanket 0): BOOL functions return FALSE;
 * value/attribute queries return their family error sentinel (GPI_ALTERROR/-1
 * for most, CLR_ERROR/-255 for colour queries); functions whose documented
 * error IS 0 (DM_ERROR, SEGEM_ERROR, RGN_/PVIS_/RVIS_/drawing primitives) keep
 * returning 0. Signatures match OS/2 Toolkit 4.5 PMGPI.H exactly.
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
#ifdef PMGPI_LOG
#define INCL_DOSFILEMGR      /* DosOpen/DosWrite/DosSetFilePtr/DosClose */
#define INCL_DOSERRORS       /* NO_ERROR                               */
#endif
#include <os2.h>

/* GPI_ERROR, DEV_ERROR, DCTL_ERROR defined in pmgpi.h / pmdev.h */

/* ==================================================================
 * Optional call logging (build with  -dPMGPI_LOG).
 * Captures, at boot, which Gpi* functions the live PM desktop actually
 * calls -- the empirical hot-path that tells us what to implement first.
 * Each call is appended to C:\PMGPI.LOG with write-through so the log
 * survives a boot hang. ZERO overhead when PMGPI_LOG is not defined.
 * (Idea adapted from osFree's unimplemented(__FUNCTION__) pattern.)
 * ================================================================== */
#ifdef PMGPI_LOG
static void PmgpiLog(const char *pszFn)
{
    static volatile ULONG fInLog = 0;   /* crude non-reentrancy guard */
    HFILE  hf;
    ULONG  ulAction, cb, i;
    char   buf[80];
    if (fInLog) return;                 /* avoid re-entry from within */
    fInLog = 1;
    if (DosOpen((PSZ)"C:\\PMGPI.LOG", &hf, &ulAction, 0UL,
                FILE_NORMAL,
                OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE |
                OPEN_FLAGS_WRITE_THROUGH,
                (PEAOP2)NULL) == NO_ERROR)
    {
        for (i = 0; i < 70UL && pszFn[i]; i++) buf[i] = pszFn[i];
        buf[i++] = '\r'; buf[i++] = '\n';
        DosSetFilePtr(hf, 0L, FILE_END, &cb);   /* append */
        DosWrite(hf, (PVOID)buf, i, &cb);
        DosClose(hf);
    }
    fInLog = 0;
}
#define PMGPILOG(name) PmgpiLog(name)
#else
#define PMGPILOG(name) ((void)0)
#endif

/* ================================================================== */
/* DEV functions (ord 1-6, 165, 244, 729)                            */
/* ================================================================== */

HDC APIENTRY DevOpenDC(HAB hab, LONG lType, PCSZ pszToken,
                        LONG lCount, PDEVOPENDATA pdopData,
                        HDC hdcComp)
{ PMGPILOG("DevOpenDC"); (void)hab;(void)lType;(void)pszToken;(void)lCount;(void)pdopData;(void)hdcComp;
  return DEV_ERROR; }
#pragma aux DevOpenDC "DEVOPENDC"

BOOL APIENTRY DevCloseDC(HDC hdc)
{ PMGPILOG("DevCloseDC"); (void)hdc; return FALSE; }
#pragma aux DevCloseDC "DEVCLOSEDC"

LONG APIENTRY DevPostDeviceModes(HAB hab, PDRIVDATA pdrivdatOut,
                                   PCSZ pszDriverName, PCSZ pszDeviceName,
                                   PCSZ pszOutputName, ULONG fl)
{ PMGPILOG("DevPostDeviceModes"); (void)hab;(void)pdrivdatOut;(void)pszDriverName;(void)pszDeviceName;
  (void)pszOutputName;(void)fl; return DEV_ERROR; }
#pragma aux DevPostDeviceModes "DEVPOSTDEVICEMODES"

LONG APIENTRY DevEscape(HDC hdc, LONG lCode, LONG lInCount,
                          PBYTE pbInData, PLONG plOutCount,
                          PBYTE pbOutData)
{ PMGPILOG("DevEscape"); (void)hdc;(void)lCode;(void)lInCount;(void)pbInData;
  (void)plOutCount;(void)pbOutData; return DEV_ERROR; }
#pragma aux DevEscape "DEVESCAPE"

BOOL APIENTRY DevQueryHardCopyCaps(HDC hdc, LONG lStartForm,
                                     LONG lForms, PHCINFO phciInfo)
{ PMGPILOG("DevQueryHardCopyCaps"); (void)hdc;(void)lStartForm;(void)lForms;(void)phciInfo; return FALSE; }
#pragma aux DevQueryHardCopyCaps "DEVQUERYHARDCOPYCAPS"

/* DevQueryCaps -- report device capabilities.  A pure-error stub breaks
 * callers that size palettes / coordinates off the caps at startup, so we
 * report a conservative standard display per the CAPS_* indices in pmdev.h:
 * 1024x768, 256 colours (8bpp), raster.  Values are intentionally modest for
 * maximum app compatibility (e.g. 256 colours so callers that loop over
 * CAPS_COLORS do not allocate huge palettes). */
static LONG PmgpiDevCap(LONG idx)
{
    switch (idx)
    {
    case CAPS_FAMILY:                 return CAPS_TECH_RASTER_DISPLAY;
    case CAPS_IO_CAPS:                return CAPS_IO_SUPPORTS_OP;
    case CAPS_TECHNOLOGY:             return CAPS_TECH_RASTER_DISPLAY;
    case CAPS_DRIVER_VERSION:         return 0x0100L;
    case CAPS_WIDTH:                  return 1024L;
    case CAPS_HEIGHT:                 return 768L;
    case CAPS_WIDTH_IN_CHARS:         return 128L;   /* 1024 / 8  */
    case CAPS_HEIGHT_IN_CHARS:        return 48L;    /* 768 / 16  */
    case CAPS_HORIZONTAL_RESOLUTION:  return 3780L;  /* ~96 dpi in pels/metre */
    case CAPS_VERTICAL_RESOLUTION:    return 3780L;
    case CAPS_CHAR_WIDTH:             return 8L;
    case CAPS_CHAR_HEIGHT:            return 16L;
    case CAPS_SMALL_CHAR_WIDTH:       return 6L;
    case CAPS_SMALL_CHAR_HEIGHT:      return 12L;
    case CAPS_COLORS:                 return 256L;
    case CAPS_COLOR_PLANES:           return 1L;
    case CAPS_COLOR_BITCOUNT:         return 8L;
    case CAPS_COLOR_TABLE_SUPPORT:    return CAPS_COLTABL_REALIZE;
    case CAPS_MOUSE_BUTTONS:          return 2L;
    case CAPS_FOREGROUND_MIX_SUPPORT: return CAPS_FM_OR | CAPS_FM_OVERPAINT |
                                             CAPS_FM_XOR | CAPS_FM_LEAVEALONE;
    case CAPS_BACKGROUND_MIX_SUPPORT: return CAPS_BM_OVERPAINT | CAPS_BM_LEAVEALONE;
    case CAPS_VIO_LOADABLE_FONTS:     return 0L;
    case CAPS_WINDOW_BYTE_ALIGNMENT:  return CAPS_BYTE_ALIGN_NOT_REQUIRED;
    case CAPS_BITMAP_FORMATS:         return 1L;
    case CAPS_RASTER_CAPS:            return CAPS_RASTER_BITBLT | CAPS_RASTER_SET_PEL |
                                             CAPS_RASTER_FONTS;
    case CAPS_MARKER_HEIGHT:          return 8L;
    case CAPS_MARKER_WIDTH:           return 8L;
    case CAPS_DEVICE_FONTS:           return 0L;
    case CAPS_GRAPHICS_SUBSET:        return 5L;
    case CAPS_GRAPHICS_VERSION:       return 200L;
    case CAPS_GRAPHICS_VECTOR_SUBSET: return 2L;
    case CAPS_DEVICE_WINDOWING:       return CAPS_DEV_WINDOWING_SUPPORT;
    case CAPS_ADDITIONAL_GRAPHICS:    return 0L;
    case CAPS_PHYS_COLORS:            return 262144L;  /* 18-bit DAC */
    case CAPS_COLOR_INDEX:            return 255L;
    case CAPS_GRAPHICS_CHAR_WIDTH:    return 8L;
    case CAPS_GRAPHICS_CHAR_HEIGHT:   return 16L;
    case CAPS_HORIZONTAL_FONT_RES:    return 96L;
    case CAPS_VERTICAL_FONT_RES:      return 96L;
    case CAPS_DEVICE_FONT_SIM:        return 0L;
    case CAPS_LINEWIDTH_THICK:        return 2L;
    case CAPS_DEVICE_POLYSET_POINTS:  return 0L;
    default:                          return 0L;
    }
}

BOOL APIENTRY DevQueryCaps(HDC hdc, LONG lStart,
                             LONG lCount, PLONG alArray)
{
    LONG i;
    PMGPILOG("DevQueryCaps");
    (void)hdc;
    if (alArray == (PLONG)NULL || lCount <= 0L || lStart < 0L)
        return FALSE;
    for (i = 0L; i < lCount; i++)
        alArray[i] = PmgpiDevCap(lStart + i);
    return TRUE;
}
#pragma aux DevQueryCaps "DEVQUERYCAPS"

BOOL APIENTRY DevQueryDeviceNames(HAB hab, PCSZ pszDriverName,
                                    PLONG pldn, PSTR32 aDeviceName,
                                    PSTR64 aDeviceDesc,
                                    PLONG pldt, PSTR16 aDataType)
{ PMGPILOG("DevQueryDeviceNames"); (void)hab;(void)pszDriverName;(void)pldn;(void)aDeviceName;
  (void)aDeviceDesc;(void)pldt;(void)aDataType; return FALSE; }
#pragma aux DevQueryDeviceNames "DEVQUERYDEVICENAMES"

HDC APIENTRY DevStdOpen(PCSZ pszDeviceName)
{ PMGPILOG("DevStdOpen"); (void)pszDeviceName; return DEV_ERROR; }
#pragma aux DevStdOpen "DEVSTDOPEN"

/* DevPostEscape: the pmdev.h prototype has an unusual signature that
 * varies between Toolkit versions. We provide a stub under an internal
 * name and export it as DEVPOSTESCAPE to avoid the prototype conflict. */
LONG DevPostEscapeStub(void) { PMGPILOG("DevPostEscapeStub"); return DEV_ERROR; }
#pragma aux DevPostEscapeStub "DevPostEscapeStub"

/* ================================================================== */
/* Presentation Space                                                  */
/* ================================================================== */

HPS APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions)
{ PMGPILOG("GpiCreatePS"); (void)hab;(void)hdc;(void)psizlSize;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiCreatePS "GPICREATEPS"

BOOL APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize)
{ PMGPILOG("GpiQueryPS"); (void)hps;(void)psizlSize; return FALSE; }
#pragma aux GpiQueryPS "GPIQUERYPS"

BOOL APIENTRY GpiDestroyPS(HPS hps)
{ PMGPILOG("GpiDestroyPS"); (void)hps; return FALSE; }
#pragma aux GpiDestroyPS "GPIDESTROYPS"

BOOL APIENTRY GpiResetPS(HPS hps, ULONG flOptions)
{ PMGPILOG("GpiResetPS"); (void)hps;(void)flOptions; return FALSE; }
#pragma aux GpiResetPS "GPIRESETPS"

LONG APIENTRY GpiSavePS(HPS hps)
{ PMGPILOG("GpiSavePS"); (void)hps; return GPI_ERROR; }
#pragma aux GpiSavePS "GPISAVEPS"

BOOL APIENTRY GpiRestorePS(HPS hps, LONG lPSid)
{ PMGPILOG("GpiRestorePS"); (void)hps;(void)lPSid; return FALSE; }
#pragma aux GpiRestorePS "GPIRESTOREPS"

BOOL APIENTRY GpiAssociate(HPS hps, HDC hdc)
{ PMGPILOG("GpiAssociate"); (void)hps;(void)hdc; return FALSE; }
#pragma aux GpiAssociate "GPIASSOCIATE"

BOOL APIENTRY GpiErrorSegmentData(HPS hps, PLONG plSegment, PLONG plContext)
{ PMGPILOG("GpiErrorSegmentData"); (void)hps;(void)plSegment;(void)plContext; return FALSE; }
#pragma aux GpiErrorSegmentData "GPIERRORSEGMENTDATA"

BOOL APIENTRY GpiErase(HPS hps)
{ PMGPILOG("GpiErase"); (void)hps; return FALSE; }
#pragma aux GpiErase "GPIERASE"

/* ================================================================== */
/* Drawing controls                                                    */
/* ================================================================== */

BOOL APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue)
{ PMGPILOG("GpiSetDrawControl"); (void)hps;(void)lControl;(void)lValue; return FALSE; }
#pragma aux GpiSetDrawControl "GPISETDRAWCONTROL"

LONG APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl)
{ PMGPILOG("GpiQueryDrawControl"); (void)hps;(void)lControl; return GPI_ALTERROR; }
#pragma aux GpiQueryDrawControl "GPIQUERYDRAWCONTROL"

BOOL APIENTRY GpiDrawChain(HPS hps)
{ PMGPILOG("GpiDrawChain"); (void)hps; return FALSE; }
#pragma aux GpiDrawChain "GPIDRAWCHAIN"

BOOL APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment)
{ PMGPILOG("GpiDrawFrom"); (void)hps;(void)lFirstSegment;(void)lLastSegment; return FALSE; }
#pragma aux GpiDrawFrom "GPIDRAWFROM"

BOOL APIENTRY GpiDrawSegment(HPS hps, LONG lSegment)
{ PMGPILOG("GpiDrawSegment"); (void)hps;(void)lSegment; return FALSE; }
#pragma aux GpiDrawSegment "GPIDRAWSEGMENT"

BOOL APIENTRY GpiSetStopDraw(HPS hps, LONG lValue)
{ PMGPILOG("GpiSetStopDraw"); (void)hps;(void)lValue; return FALSE; }
#pragma aux GpiSetStopDraw "GPISETSTOPDRAW"

LONG APIENTRY GpiQueryStopDraw(HPS hps)
{ PMGPILOG("GpiQueryStopDraw"); (void)hps; return GPI_ALTERROR; }
#pragma aux GpiQueryStopDraw "GPIQUERYSTOPDRAW"

BOOL APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstTag, LONG lLastTag)
{ PMGPILOG("GpiRemoveDynamics"); (void)hps;(void)lFirstTag;(void)lLastTag; return FALSE; }
#pragma aux GpiRemoveDynamics "GPIREMOVEDYNAMICS"

BOOL APIENTRY GpiDrawDynamics(HPS hps)
{ PMGPILOG("GpiDrawDynamics"); (void)hps; return FALSE; }
#pragma aux GpiDrawDynamics "GPIDRAWDYNAMICS"

BOOL APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode)
{ PMGPILOG("GpiSetDrawingMode"); (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetDrawingMode "GPISETDRAWINGMODE"

LONG APIENTRY GpiQueryDrawingMode(HPS hps)
{ PMGPILOG("GpiQueryDrawingMode"); (void)hps; return DM_DRAW; }
#pragma aux GpiQueryDrawingMode "GPIQUERYDRAWINGMODE"

LONG APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset,
                          LONG lFormat, LONG lLength, PBYTE pbData)
{ PMGPILOG("GpiGetData"); (void)hps;(void)lSegid;(void)plOffset;(void)lFormat;
  (void)lLength;(void)pbData; return GPI_ERROR; }
#pragma aux GpiGetData "GPIGETDATA"

LONG APIENTRY GpiPutData(HPS hps, LONG lFormat,
                          PLONG plCount, PBYTE pbData)
{ PMGPILOG("GpiPutData"); (void)hps;(void)lFormat;(void)plCount;(void)pbData; return GPI_ERROR; }
#pragma aux GpiPutData "GPIPUTDATA"

/* ================================================================== */
/* Pick aperture                                                       */
/* ================================================================== */

BOOL APIENTRY GpiSetPickApertureSize(HPS hps, LONG lOptions, PSIZEL psizl)
{ PMGPILOG("GpiSetPickApertureSize"); (void)hps;(void)lOptions;(void)psizl; return FALSE; }
#pragma aux GpiSetPickApertureSize "GPISETPICKAPERTURESIZE"

BOOL APIENTRY GpiQueryPickApertureSize(HPS hps, PSIZEL psizlSize)
{ PMGPILOG("GpiQueryPickApertureSize"); (void)hps;(void)psizlSize; return FALSE; }
#pragma aux GpiQueryPickApertureSize "GPIQUERYPICKAPERTURESIZE"

BOOL APIENTRY GpiSetPickAperturePosition(HPS hps, PPOINTL pptlPick)
{ PMGPILOG("GpiSetPickAperturePosition"); (void)hps;(void)pptlPick; return FALSE; }
#pragma aux GpiSetPickAperturePosition "GPISETPICKAPERTUREPOSITION"

BOOL APIENTRY GpiQueryPickAperturePosition(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiQueryPickAperturePosition"); (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiQueryPickAperturePosition "GPIQUERYPICKAPERTUREPOSITION"

/* ================================================================== */
/* Tags and correlation                                                */
/* ================================================================== */

BOOL APIENTRY GpiSetTag(HPS hps, LONG lTag)
{ PMGPILOG("GpiSetTag"); (void)hps;(void)lTag; return FALSE; }
#pragma aux GpiSetTag "GPISETTAG"

ULONG APIENTRY GpiQueryTag(HPS hps, PLONG plTag)
{ PMGPILOG("GpiQueryTag"); (void)hps;(void)plTag; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryTag "GPIQUERYTAG"

LONG APIENTRY GpiCorrelateChain(HPS hps, LONG lType, PPOINTL pptlPick,
                                  LONG lMaxHits, LONG lMaxDepth,
                                  PLONG alBuffer)
{ PMGPILOG("GpiCorrelateChain"); (void)hps;(void)lType;(void)pptlPick;(void)lMaxHits;
  (void)lMaxDepth;(void)alBuffer; return GPI_ERROR; }
#pragma aux GpiCorrelateChain "GPICORRELATECHAIN"

LONG APIENTRY GpiCorrelateFrom(HPS hps, LONG lFirstSeg, LONG lLastSeg,
                                 LONG lType, PPOINTL pptlPick,
                                 LONG lMaxHits, LONG lMaxDepth,
                                 PLONG alBuffer)
{ PMGPILOG("GpiCorrelateFrom"); (void)hps;(void)lFirstSeg;(void)lLastSeg;(void)lType;(void)pptlPick;
  (void)lMaxHits;(void)lMaxDepth;(void)alBuffer; return GPI_ERROR; }
#pragma aux GpiCorrelateFrom "GPICORRELATEFROM"

LONG APIENTRY GpiCorrelateSegment(HPS hps, LONG lSegment, LONG lType,
                                    PPOINTL pptlPick, LONG lMaxDepth,
                                    LONG lMaxHits, PLONG alBuffer)
{ PMGPILOG("GpiCorrelateSegment"); (void)hps;(void)lSegment;(void)lType;(void)pptlPick;
  (void)lMaxDepth;(void)lMaxHits;(void)alBuffer; return GPI_ERROR; }
#pragma aux GpiCorrelateSegment "GPICORRELATESEGMENT"

/* ================================================================== */
/* Boundary data                                                       */
/* ================================================================== */

BOOL APIENTRY GpiResetBoundaryData(HPS hps)
{ PMGPILOG("GpiResetBoundaryData"); (void)hps; return FALSE; }
#pragma aux GpiResetBoundaryData "GPIRESETBOUNDARYDATA"

BOOL APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary)
{ PMGPILOG("GpiQueryBoundaryData"); (void)hps;(void)prclBoundary; return FALSE; }
#pragma aux GpiQueryBoundaryData "GPIQUERYBOUNDARYDATA"

/* ================================================================== */
/* Segments                                                            */
/* ================================================================== */

BOOL APIENTRY GpiOpenSegment(HPS hps, LONG lSegment)
{ PMGPILOG("GpiOpenSegment"); (void)hps;(void)lSegment; return FALSE; }
#pragma aux GpiOpenSegment "GPIOPENSEGMENT"

BOOL APIENTRY GpiCloseSegment(HPS hps)
{ PMGPILOG("GpiCloseSegment"); (void)hps; return FALSE; }
#pragma aux GpiCloseSegment "GPICLOSESEGMENT"

BOOL APIENTRY GpiDeleteSegment(HPS hps, LONG lSegid)
{ PMGPILOG("GpiDeleteSegment"); (void)hps;(void)lSegid; return FALSE; }
#pragma aux GpiDeleteSegment "GPIDELETESEGMENT"

BOOL APIENTRY GpiDeleteSegments(HPS hps, LONG lFirstSegment,
                                  LONG lLastSegment)
{ PMGPILOG("GpiDeleteSegments"); (void)hps;(void)lFirstSegment;(void)lLastSegment; return FALSE; }
#pragma aux GpiDeleteSegments "GPIDELETESEGMENTS"

LONG APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid,
                                     LONG lLastSegid, LONG lMax,
                                     PLONG alSegids)
{ PMGPILOG("GpiQuerySegmentNames"); (void)hps;(void)lFirstSegid;(void)lLastSegid;(void)lMax;
  (void)alSegids; return GPI_ERROR; }
#pragma aux GpiQuerySegmentNames "GPIQUERYSEGMENTNAMES"

BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttribute, LONG lValue)
{ PMGPILOG("GpiSetInitialSegmentAttrs"); (void)hps;(void)lAttribute;(void)lValue; return FALSE; }
#pragma aux GpiSetInitialSegmentAttrs "GPISETINITIALSEGMENTATTRS"

LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttribute)
{ PMGPILOG("GpiQueryInitialSegmentAttrs"); (void)hps;(void)lAttribute; return GPI_ALTERROR; }
#pragma aux GpiQueryInitialSegmentAttrs "GPIQUERYINITIALSEGMENTATTRS"

BOOL APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegid,
                                   LONG lAttribute, LONG lValue)
{ PMGPILOG("GpiSetSegmentAttrs"); (void)hps;(void)lSegid;(void)lAttribute;(void)lValue; return FALSE; }
#pragma aux GpiSetSegmentAttrs "GPISETSEGMENTATTRS"

LONG APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute)
{ PMGPILOG("GpiQuerySegmentAttrs"); (void)hps;(void)lSegid;(void)lAttribute; return GPI_ALTERROR; }
#pragma aux GpiQuerySegmentAttrs "GPIQUERYSEGMENTATTRS"

BOOL APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegid,
                                      LONG lRefSegid, LONG lOrder)
{ PMGPILOG("GpiSetSegmentPriority"); (void)hps;(void)lSegid;(void)lRefSegid;(void)lOrder; return FALSE; }
#pragma aux GpiSetSegmentPriority "GPISETSEGMENTPRIORITY"

LONG APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegid, LONG lOrder)
{ PMGPILOG("GpiQuerySegmentPriority"); (void)hps;(void)lRefSegid;(void)lOrder; return GPI_ALTERROR; }
#pragma aux GpiQuerySegmentPriority "GPIQUERYSEGMENTPRIORITY"

/* ================================================================== */
/* Segment editing                                                     */
/* ================================================================== */

BOOL APIENTRY GpiSetEditMode(HPS hps, LONG lMode)
{ PMGPILOG("GpiSetEditMode"); (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetEditMode "GPISETEDITMODE"

LONG APIENTRY GpiQueryEditMode(HPS hps)
{ PMGPILOG("GpiQueryEditMode"); (void)hps; return GPI_ERROR; }
#pragma aux GpiQueryEditMode "GPIQUERYEDITMODE"

BOOL APIENTRY GpiSetElementPointer(HPS hps, LONG lElement)
{ PMGPILOG("GpiSetElementPointer"); (void)hps;(void)lElement; return FALSE; }
#pragma aux GpiSetElementPointer "GPISETELEMENTPOINTER"

LONG APIENTRY GpiQueryElementPointer(HPS hps)
{ PMGPILOG("GpiQueryElementPointer"); (void)hps; return GPI_ALTERROR; }
#pragma aux GpiQueryElementPointer "GPIQUERYELEMENTPOINTER"

BOOL APIENTRY GpiOffsetElementPointer(HPS hps, LONG lOffset)
{ PMGPILOG("GpiOffsetElementPointer"); (void)hps;(void)lOffset; return FALSE; }
#pragma aux GpiOffsetElementPointer "GPIOFFSETELEMENTPOINTER"

BOOL APIENTRY GpiDeleteElement(HPS hps)
{ PMGPILOG("GpiDeleteElement"); (void)hps; return FALSE; }
#pragma aux GpiDeleteElement "GPIDELETEELEMENT"

BOOL APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirstElement,
                                      LONG lLastElement)
{ PMGPILOG("GpiDeleteElementRange"); (void)hps;(void)lFirstElement;(void)lLastElement; return FALSE; }
#pragma aux GpiDeleteElementRange "GPIDELETEELEMENTRANGE"

BOOL APIENTRY GpiLabel(HPS hps, LONG lLabel)
{ PMGPILOG("GpiLabel"); (void)hps;(void)lLabel; return FALSE; }
#pragma aux GpiLabel "GPILABEL"

BOOL APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel)
{ PMGPILOG("GpiSetElementPointerAtLabel"); (void)hps;(void)lLabel; return FALSE; }
#pragma aux GpiSetElementPointerAtLabel "GPISETELEMENTPOINTERATLABEL"

BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS hps, LONG lFirstLabel,
                                               LONG lLastLabel)
{ PMGPILOG("GpiDeleteElementsBetweenLabels"); (void)hps;(void)lFirstLabel;(void)lLastLabel; return FALSE; }
#pragma aux GpiDeleteElementsBetweenLabels "GPIDELETEELEMENTSBETWEENLABELS"

LONG APIENTRY GpiQueryElementType(HPS hps, PLONG plType,
                                    LONG lLength, PBYTE pbData)
{ PMGPILOG("GpiQueryElementType"); (void)hps;(void)plType;(void)lLength;(void)pbData; return GPI_ERROR; }
#pragma aux GpiQueryElementType "GPIQUERYELEMENTTYPE"

LONG APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength,
                                PBYTE pbData)
{ PMGPILOG("GpiQueryElement"); (void)hps;(void)lOff;(void)lMaxLength;(void)pbData; return GPI_ERROR; }
#pragma aux GpiQueryElement "GPIQUERYELEMENT"

BOOL APIENTRY GpiElement(HPS hps, LONG lType, PCSZ pszDesc,
                           LONG lLength, PBYTE pbData)
{ PMGPILOG("GpiElement"); (void)hps;(void)lType;(void)pszDesc;(void)lLength;(void)pbData;
  return FALSE; }
#pragma aux GpiElement "GPIELEMENT"

BOOL APIENTRY GpiBeginElement(HPS hps, LONG lType, PCSZ pszDesc)
{ PMGPILOG("GpiBeginElement"); (void)hps;(void)lType;(void)pszDesc; return FALSE; }
#pragma aux GpiBeginElement "GPIBEGINELEMENT"

BOOL APIENTRY GpiEndElement(HPS hps)
{ PMGPILOG("GpiEndElement"); (void)hps; return FALSE; }
#pragma aux GpiEndElement "GPIENDELEMENT"

/* ================================================================== */
/* Transforms                                                          */
/* ================================================================== */

BOOL APIENTRY GpiSetSegmentTransformMatrix(HPS hps, LONG lSegid,
                                             LONG lCount, PMATRIXLF pmatlfArray,
                                             LONG lOptions)
{ PMGPILOG("GpiSetSegmentTransformMatrix"); (void)hps;(void)lSegid;(void)lCount;(void)pmatlfArray;(void)lOptions;
  return FALSE; }
#pragma aux GpiSetSegmentTransformMatrix "GPISETSEGMENTTRANSFORMMATRIX"

BOOL APIENTRY GpiQuerySegmentTransformMatrix(HPS hps, LONG lSegid,
                                               LONG lCount,
                                               PMATRIXLF pmatlfArray)
{ PMGPILOG("GpiQuerySegmentTransformMatrix"); (void)hps;(void)lSegid;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQuerySegmentTransformMatrix "GPIQUERYSEGMENTTRANSFORMMATRIX"

BOOL APIENTRY GpiSetModelTransformMatrix(HPS hps, LONG lCount,
                                           PMATRIXLF pmatlfArray, LONG lOptions)
{ PMGPILOG("GpiSetModelTransformMatrix"); (void)hps;(void)lCount;(void)pmatlfArray;(void)lOptions; return FALSE; }
#pragma aux GpiSetModelTransformMatrix "GPISETMODELTRANSFORMMATRIX"

BOOL APIENTRY GpiQueryModelTransformMatrix(HPS hps, LONG lCount,
                                             PMATRIXLF pmatlfArray)
{ PMGPILOG("GpiQueryModelTransformMatrix"); (void)hps;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQueryModelTransformMatrix "GPIQUERYMODELTRANSFORMMATRIX"

LONG APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegment, LONG lCount,
                                     PMATRIXLF pmatlfArray, LONG lOptions)
{ PMGPILOG("GpiCallSegmentMatrix"); (void)hps;(void)lSegment;(void)lCount;(void)pmatlfArray;(void)lOptions;
  return GPI_ERROR; }
#pragma aux GpiCallSegmentMatrix "GPICALLSEGMENTMATRIX"

BOOL APIENTRY GpiSetDefaultViewMatrix(HPS hps, LONG lCount,
                                        PMATRIXLF pmatlfArray, LONG lOptions)
{ PMGPILOG("GpiSetDefaultViewMatrix"); (void)hps;(void)lCount;(void)pmatlfArray;(void)lOptions; return FALSE; }
#pragma aux GpiSetDefaultViewMatrix "GPISETDEFAULTVIEWMATRIX"

BOOL APIENTRY GpiQueryDefaultViewMatrix(HPS hps, LONG lCount,
                                          PMATRIXLF pmatlfArray)
{ PMGPILOG("GpiQueryDefaultViewMatrix"); (void)hps;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQueryDefaultViewMatrix "GPIQUERYDEFAULTVIEWMATRIX"

BOOL APIENTRY GpiSetPageViewport(HPS hps, PRECTL prclViewport)
{ PMGPILOG("GpiSetPageViewport"); (void)hps;(void)prclViewport; return FALSE; }
#pragma aux GpiSetPageViewport "GPISETPAGEVIEWPORT"

BOOL APIENTRY GpiQueryPageViewport(HPS hps, PRECTL prclViewport)
{ PMGPILOG("GpiQueryPageViewport"); (void)hps;(void)prclViewport; return FALSE; }
#pragma aux GpiQueryPageViewport "GPIQUERYPAGEVIEWPORT"

BOOL APIENTRY GpiSetViewingTransformMatrix(HPS hps, LONG lCount,
                                             PMATRIXLF pmatlfArray,
                                             LONG lOptions)
{ PMGPILOG("GpiSetViewingTransformMatrix"); (void)hps;(void)lCount;(void)pmatlfArray;(void)lOptions; return FALSE; }
#pragma aux GpiSetViewingTransformMatrix "GPISETVIEWINGTRANSFORMMATRIX"

BOOL APIENTRY GpiQueryViewingTransformMatrix(HPS hps, LONG lCount,
                                               PMATRIXLF pmatlfArray)
{ PMGPILOG("GpiQueryViewingTransformMatrix"); (void)hps;(void)lCount;(void)pmatlfArray; return FALSE; }
#pragma aux GpiQueryViewingTransformMatrix "GPIQUERYVIEWINGTRANSFORMMATRIX"

BOOL APIENTRY GpiSetGraphicsField(HPS hps, PRECTL prclField)
{ PMGPILOG("GpiSetGraphicsField"); (void)hps;(void)prclField; return FALSE; }
#pragma aux GpiSetGraphicsField "GPISETGRAPHICSFIELD"

BOOL APIENTRY GpiQueryGraphicsField(HPS hps, PRECTL prclField)
{ PMGPILOG("GpiQueryGraphicsField"); (void)hps;(void)prclField; return FALSE; }
#pragma aux GpiQueryGraphicsField "GPIQUERYGRAPHICSFIELD"

BOOL APIENTRY GpiSetViewingLimits(HPS hps, PRECTL prclLimits)
{ PMGPILOG("GpiSetViewingLimits"); (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiSetViewingLimits "GPISETVIEWINGLIMITS"

BOOL APIENTRY GpiQueryViewingLimits(HPS hps, PRECTL prclLimits)
{ PMGPILOG("GpiQueryViewingLimits"); (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiQueryViewingLimits "GPIQUERYVIEWINGLIMITS"

BOOL APIENTRY GpiConvert(HPS hps, LONG lSrc, LONG lTarg,
                           LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiConvert"); (void)hps;(void)lSrc;(void)lTarg;(void)lCount;(void)aptlPoints;
  return FALSE; }
#pragma aux GpiConvert "GPICONVERT"

BOOL APIENTRY GpiConvertWithMatrix(HPS hps, LONG lCount,
                                     PPOINTL aptlPoints, LONG lMcount,
                                     PMATRIXLF pmatlfArray)
{ PMGPILOG("GpiConvertWithMatrix"); (void)hps;(void)lCount;(void)aptlPoints;(void)lMcount;(void)pmatlfArray;
  return FALSE; }
#pragma aux GpiConvertWithMatrix "GPICONVERTWITHMATRIX"

BOOL APIENTRY GpiTranslate(HPS hps, PMATRIXLF pmatlfArray,
                              LONG lOptions, PPOINTL pptlTranslate)
{ PMGPILOG("GpiTranslate"); (void)hps;(void)pmatlfArray;(void)lOptions;(void)pptlTranslate;
  return FALSE; }
#pragma aux GpiTranslate "GPITRANSLATE"

BOOL APIENTRY GpiScale(HPS hps, PMATRIXLF pmatlfArray, LONG lOptions,
                         PFIXED afxScale, PPOINTL pptlCenter)
{ PMGPILOG("GpiScale"); (void)hps;(void)pmatlfArray;(void)lOptions;(void)afxScale;(void)pptlCenter;
  return FALSE; }
#pragma aux GpiScale "GPISCALE"

BOOL APIENTRY GpiRotate(HPS hps, PMATRIXLF pmatlfArray, LONG lOptions,
                          FIXED fxAngle, PPOINTL pptlCenter)
{ PMGPILOG("GpiRotate"); (void)hps;(void)pmatlfArray;(void)lOptions;(void)fxAngle;(void)pptlCenter;
  return FALSE; }
#pragma aux GpiRotate "GPIROTATE"

/* ================================================================== */
/* Attributes                                                          */
/* ================================================================== */

BOOL APIENTRY GpiSetAttrMode(HPS hps, LONG lMode)
{ PMGPILOG("GpiSetAttrMode"); (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetAttrMode "GPISETATTRMODE"

LONG APIENTRY GpiQueryAttrMode(HPS hps)
{ PMGPILOG("GpiQueryAttrMode"); (void)hps; return GPI_ALTERROR; }
#pragma aux GpiQueryAttrMode "GPIQUERYATTRMODE"

BOOL APIENTRY GpiPop(HPS hps, LONG lCount)
{ PMGPILOG("GpiPop"); (void)hps;(void)lCount; return FALSE; }
#pragma aux GpiPop "GPIPOP"

BOOL APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask,
                            ULONG flDefMask, PBUNDLE ppbunAttrs)
{ PMGPILOG("GpiSetAttrs"); (void)hps;(void)lPrimType;(void)flAttrMask;(void)flDefMask;
  (void)ppbunAttrs; return FALSE; }
#pragma aux GpiSetAttrs "GPISETATTRS"

LONG APIENTRY GpiQueryAttrs(HPS hps, LONG lPrimType,
                              ULONG flAttrMask, PBUNDLE ppbunAttrs)
{ PMGPILOG("GpiQueryAttrs"); (void)hps;(void)lPrimType;(void)flAttrMask;(void)ppbunAttrs; return GPI_ERROR; }
#pragma aux GpiQueryAttrs "GPIQUERYATTRS"

/* ================================================================== */
/* Color tables                                                        */
/* ================================================================== */

BOOL APIENTRY GpiCreateLogColorTable(HPS hps, ULONG flOptions,
                                       LONG lFormat, LONG lStart,
                                       LONG lCount, PLONG alTable)
{ PMGPILOG("GpiCreateLogColorTable"); (void)hps;(void)flOptions;(void)lFormat;(void)lStart;
  (void)lCount;(void)alTable; return FALSE; }
#pragma aux GpiCreateLogColorTable "GPICREATELOGCOLORTABLE"

BOOL APIENTRY GpiRealizeColorTable(HPS hps)
{ PMGPILOG("GpiRealizeColorTable"); (void)hps; return FALSE; }
#pragma aux GpiRealizeColorTable "GPIREALIZECOLORTABLE"

BOOL APIENTRY GpiUnrealizeColorTable(HPS hps)
{ PMGPILOG("GpiUnrealizeColorTable"); (void)hps; return FALSE; }
#pragma aux GpiUnrealizeColorTable "GPIUNREALIZECOLORTABLE"

BOOL APIENTRY GpiQueryColorData(HPS hps, LONG lCount, PLONG alArray)
{ PMGPILOG("GpiQueryColorData"); (void)hps;(void)lCount;(void)alArray; return FALSE; }
#pragma aux GpiQueryColorData "GPIQUERYCOLORDATA"

LONG APIENTRY GpiQueryLogColorTable(HPS hps, ULONG flOptions, LONG lStart,
                                      LONG lCount, PLONG alArray)
{ PMGPILOG("GpiQueryLogColorTable"); (void)hps;(void)flOptions;(void)lStart;(void)lCount;(void)alArray;
  return GPI_ERROR; }
#pragma aux GpiQueryLogColorTable "GPIQUERYLOGCOLORTABLE"

LONG APIENTRY GpiQueryRealColors(HPS hps, ULONG flOptions, LONG lStart,
                                   LONG lCount, PLONG alColors)
{ PMGPILOG("GpiQueryRealColors"); (void)hps;(void)flOptions;(void)lStart;(void)lCount;(void)alColors;
  return GPI_ERROR; }
#pragma aux GpiQueryRealColors "GPIQUERYREALCOLORS"

LONG APIENTRY GpiQueryNearestColor(HPS hps, ULONG flOptions, LONG lRgbIn)
{ PMGPILOG("GpiQueryNearestColor"); (void)hps;(void)flOptions;(void)lRgbIn; return GPI_ALTERROR; }
#pragma aux GpiQueryNearestColor "GPIQUERYNEARESTCOLOR"

LONG APIENTRY GpiQueryColorIndex(HPS hps, ULONG flOptions, LONG lRgbColor)
{ PMGPILOG("GpiQueryColorIndex"); (void)hps;(void)flOptions;(void)lRgbColor; return GPI_ALTERROR; }
#pragma aux GpiQueryColorIndex "GPIQUERYCOLORINDEX"

LONG APIENTRY GpiQueryRGBColor(HPS hps, ULONG flOptions, LONG lColorIndex)
{ PMGPILOG("GpiQueryRGBColor"); (void)hps;(void)flOptions;(void)lColorIndex; return GPI_ALTERROR; }
#pragma aux GpiQueryRGBColor "GPIQUERYRGBCOLOR"

/* ================================================================== */
/* Color/mix primitives                                                */
/* ================================================================== */

BOOL APIENTRY GpiSetColor(HPS hps, LONG lColor)
{ PMGPILOG("GpiSetColor"); (void)hps;(void)lColor; return FALSE; }
#pragma aux GpiSetColor "GPISETCOLOR"

LONG APIENTRY GpiQueryColor(HPS hps)
{ PMGPILOG("GpiQueryColor"); (void)hps; return CLR_NEUTRAL; }
#pragma aux GpiQueryColor "GPIQUERYCOLOR"

BOOL APIENTRY GpiSetBackColor(HPS hps, LONG lColor)
{ PMGPILOG("GpiSetBackColor"); (void)hps;(void)lColor; return FALSE; }
#pragma aux GpiSetBackColor "GPISETBACKCOLOR"

LONG APIENTRY GpiQueryBackColor(HPS hps)
{ PMGPILOG("GpiQueryBackColor"); (void)hps; return CLR_BACKGROUND; }
#pragma aux GpiQueryBackColor "GPIQUERYBACKCOLOR"

BOOL APIENTRY GpiSetMix(HPS hps, LONG lMixMode)
{ PMGPILOG("GpiSetMix"); (void)hps;(void)lMixMode; return FALSE; }
#pragma aux GpiSetMix "GPISETMIX"

LONG APIENTRY GpiQueryMix(HPS hps)
{ PMGPILOG("GpiQueryMix"); (void)hps; return FM_OVERPAINT; }
#pragma aux GpiQueryMix "GPIQUERYMIX"

BOOL APIENTRY GpiSetBackMix(HPS hps, LONG lMixMode)
{ PMGPILOG("GpiSetBackMix"); (void)hps;(void)lMixMode; return FALSE; }
#pragma aux GpiSetBackMix "GPISETBACKMIX"

LONG APIENTRY GpiQueryBackMix(HPS hps)
{ PMGPILOG("GpiQueryBackMix"); (void)hps; return BM_LEAVEALONE; }
#pragma aux GpiQueryBackMix "GPIQUERYBACKMIX"

/* ================================================================== */
/* Line attributes                                                     */
/* ================================================================== */

BOOL APIENTRY GpiSetLineType(HPS hps, LONG lLineType)
{ PMGPILOG("GpiSetLineType"); (void)hps;(void)lLineType; return FALSE; }
#pragma aux GpiSetLineType "GPISETLINETYPE"

LONG APIENTRY GpiQueryLineType(HPS hps)
{ PMGPILOG("GpiQueryLineType"); (void)hps; return LINETYPE_DEFAULT; }
#pragma aux GpiQueryLineType "GPIQUERYLINETYPE"

BOOL APIENTRY GpiSetLineWidth(HPS hps, FIXED fxLineWidth)
{ PMGPILOG("GpiSetLineWidth"); (void)hps;(void)fxLineWidth; return FALSE; }
#pragma aux GpiSetLineWidth "GPISETLINEWIDTH"

FIXED APIENTRY GpiQueryLineWidth(HPS hps)
{ PMGPILOG("GpiQueryLineWidth"); (void)hps; return (FIXED)GPI_ERROR; }
#pragma aux GpiQueryLineWidth "GPIQUERYLINEWIDTH"

BOOL APIENTRY GpiSetLineWidthGeom(HPS hps, LONG lLineWidth)
{ PMGPILOG("GpiSetLineWidthGeom"); (void)hps;(void)lLineWidth; return FALSE; }
#pragma aux GpiSetLineWidthGeom "GPISETLINEWIDTHGEOM"

LONG APIENTRY GpiQueryLineWidthGeom(HPS hps)
{ PMGPILOG("GpiQueryLineWidthGeom"); (void)hps; return 0L; }
#pragma aux GpiQueryLineWidthGeom "GPIQUERYLINEWIDTHGEOM"

BOOL APIENTRY GpiSetLineEnd(HPS hps, LONG lLineEnd)
{ PMGPILOG("GpiSetLineEnd"); (void)hps;(void)lLineEnd; return FALSE; }
#pragma aux GpiSetLineEnd "GPISETLINEEND"

LONG APIENTRY GpiQueryLineEnd(HPS hps)
{ PMGPILOG("GpiQueryLineEnd"); (void)hps; return LINEEND_DEFAULT; }
#pragma aux GpiQueryLineEnd "GPIQUERYLINEEND"

BOOL APIENTRY GpiSetLineJoin(HPS hps, LONG lLineJoin)
{ PMGPILOG("GpiSetLineJoin"); (void)hps;(void)lLineJoin; return FALSE; }
#pragma aux GpiSetLineJoin "GPISETLINEJOIN"

LONG APIENTRY GpiQueryLineJoin(HPS hps)
{ PMGPILOG("GpiQueryLineJoin"); (void)hps; return LINEJOIN_DEFAULT; }
#pragma aux GpiQueryLineJoin "GPIQUERYLINEJOIN"

/* ================================================================== */
/* Current position and line drawing                                   */
/* ================================================================== */

BOOL APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiSetCurrentPosition"); (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiSetCurrentPosition "GPISETCURRENTPOSITION"

BOOL APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiQueryCurrentPosition"); (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiQueryCurrentPosition "GPIQUERYCURRENTPOSITION"

BOOL APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiMove"); (void)hps;(void)pptlPoint; return FALSE; }
#pragma aux GpiMove "GPIMOVE"

LONG APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint)
{ PMGPILOG("GpiLine"); (void)hps;(void)pptlEndPoint; return GPI_ERROR; }
#pragma aux GpiLine "GPILINE"

LONG APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiPolyLine"); (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyLine "GPIPOLYLINE"

LONG APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiPolyLineDisjoint"); (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyLineDisjoint "GPIPOLYLINEDISJOINT"

LONG APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlCorner,
                      LONG lHRound, LONG lVRound)
{ PMGPILOG("GpiBox"); (void)hps;(void)lControl;(void)pptlCorner;(void)lHRound;(void)lVRound;
  return GPI_ERROR; }
#pragma aux GpiBox "GPIBOX"

LONG APIENTRY GpiPtVisible(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiPtVisible"); (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiPtVisible "GPIPTVISIBLE"

LONG APIENTRY GpiRectVisible(HPS hps, PRECTL prclRect)
{ PMGPILOG("GpiRectVisible"); (void)hps;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiRectVisible "GPIRECTVISIBLE"

/* ================================================================== */
/* Arcs and curves                                                     */
/* ================================================================== */

BOOL APIENTRY GpiSetArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ PMGPILOG("GpiSetArcParams"); (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiSetArcParams "GPISETARCPARAMS"

BOOL APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ PMGPILOG("GpiQueryArcParams"); (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiQueryArcParams "GPIQUERYARCPARAMS"

LONG APIENTRY GpiPointArc(HPS hps, PPOINTL pptl)
{ PMGPILOG("GpiPointArc"); (void)hps;(void)pptl; return GPI_ERROR; }
#pragma aux GpiPointArc "GPIPOINTARC"

LONG APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMult)
{ PMGPILOG("GpiFullArc"); (void)hps;(void)lControl;(void)fxMult; return GPI_ERROR; }
#pragma aux GpiFullArc "GPIFULLARC"

LONG APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter, FIXED fxMult,
                               FIXED fxStartAngle, FIXED fxSweepAngle)
{ PMGPILOG("GpiPartialArc"); (void)hps;(void)pptlCenter;(void)fxMult;(void)fxStartAngle;
  (void)fxSweepAngle; return GPI_ERROR; }
#pragma aux GpiPartialArc "GPIPARTIALARC"

LONG APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiPolyFillet"); (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyFillet "GPIPOLYFILLET"

LONG APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount,
                                    PPOINTL aptlPoints, PFIXED afxSharpness)
{ PMGPILOG("GpiPolyFilletSharp"); (void)hps;(void)lCount;(void)aptlPoints;(void)afxSharpness;
  return GPI_ERROR; }
#pragma aux GpiPolyFilletSharp "GPIPOLYFILLETSHARP"

LONG APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiPolySpline"); (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolySpline "GPIPOLYSPLINE"

/* ================================================================== */
/* Pattern/fill                                                        */
/* ================================================================== */

BOOL APIENTRY GpiSetBitmapId(HPS hps, HBITMAP hbm, LONG lLcid)
{ PMGPILOG("GpiSetBitmapId"); (void)hps;(void)hbm;(void)lLcid; return FALSE; }
#pragma aux GpiSetBitmapId "GPISETBITMAPID"

HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps, LONG lLcid)
{ PMGPILOG("GpiQueryBitmapHandle"); (void)hps;(void)lLcid; return (HBITMAP)GPI_ERROR; }
#pragma aux GpiQueryBitmapHandle "GPIQUERYBITMAPHANDLE"

BOOL APIENTRY GpiSetPatternSet(HPS hps, LONG lSet)
{ PMGPILOG("GpiSetPatternSet"); (void)hps;(void)lSet; return FALSE; }
#pragma aux GpiSetPatternSet "GPISETPATTERNSET"

LONG APIENTRY GpiQueryPatternSet(HPS hps)
{ PMGPILOG("GpiQueryPatternSet"); (void)hps; return LCID_DEFAULT; }
#pragma aux GpiQueryPatternSet "GPIQUERYPATTERNSET"

BOOL APIENTRY GpiSetPattern(HPS hps, LONG lPattern)
{ PMGPILOG("GpiSetPattern"); (void)hps;(void)lPattern; return FALSE; }
#pragma aux GpiSetPattern "GPISETPATTERN"

LONG APIENTRY GpiQueryPattern(HPS hps)
{ PMGPILOG("GpiQueryPattern"); (void)hps; return PATSYM_DEFAULT; }
#pragma aux GpiQueryPattern "GPIQUERYPATTERN"

BOOL APIENTRY GpiSetPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{ PMGPILOG("GpiSetPatternRefPoint"); (void)hps;(void)pptlRefPoint; return FALSE; }
#pragma aux GpiSetPatternRefPoint "GPISETPATTERNREFPOINT"

BOOL APIENTRY GpiQueryPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{ PMGPILOG("GpiQueryPatternRefPoint"); (void)hps;(void)pptlRefPoint; return FALSE; }
#pragma aux GpiQueryPatternRefPoint "GPIQUERYPATTERNREFPOINT"

BOOL APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{ PMGPILOG("GpiBeginArea"); (void)hps;(void)flOptions; return FALSE; }
#pragma aux GpiBeginArea "GPIBEGINAREA"

LONG APIENTRY GpiEndArea(HPS hps)
{ PMGPILOG("GpiEndArea"); (void)hps; return GPI_ERROR; }
#pragma aux GpiEndArea "GPIENDAREA"

/* ================================================================== */
/* Fonts                                                               */
/* ================================================================== */

BOOL APIENTRY GpiLoadFonts(HAB hab, PCSZ pszFilename)
{ PMGPILOG("GpiLoadFonts"); (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiLoadFonts "GPILOADFONTS"

BOOL APIENTRY GpiUnloadFonts(HAB hab, PCSZ pszFilename)
{ PMGPILOG("GpiUnloadFonts"); (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiUnloadFonts "GPIUNLOADFONTS"

LONG APIENTRY GpiCreateLogFont(HPS hps, PSTR8 pName, LONG lLcid,
                                  PFATTRS pfattrs)
{ PMGPILOG("GpiCreateLogFont"); (void)hps;(void)pName;(void)lLcid;(void)pfattrs; return GPI_ERROR; }
#pragma aux GpiCreateLogFont "GPICREATELOGFONT"

BOOL APIENTRY GpiDeleteSetId(HPS hps, LONG lLcid)
{ PMGPILOG("GpiDeleteSetId"); (void)hps;(void)lLcid; return FALSE; }
#pragma aux GpiDeleteSetId "GPIDELETESETID"

LONG APIENTRY GpiQueryNumberSetIds(HPS hps)
{ PMGPILOG("GpiQueryNumberSetIds"); (void)hps; return GPI_ALTERROR; }
#pragma aux GpiQueryNumberSetIds "GPIQUERYNUMBERSETIDS"

BOOL APIENTRY GpiQuerySetIds(HPS hps, LONG lCount, PLONG alTypes,
                               PSTR8 aNames, PLONG allcids)
{ PMGPILOG("GpiQuerySetIds"); (void)hps;(void)lCount;(void)alTypes;(void)aNames;(void)allcids;
  return FALSE; }
#pragma aux GpiQuerySetIds "GPIQUERYSETIDS"

LONG APIENTRY GpiQueryFonts(HPS hps, ULONG flOptions, PCSZ pszFacename,
                              PLONG plReqFonts, LONG lMetricsLength,
                              PFONTMETRICS afmMetrics)
{ PMGPILOG("GpiQueryFonts"); (void)hps;(void)flOptions;(void)pszFacename;(void)plReqFonts;
  (void)lMetricsLength;(void)afmMetrics; return GPI_ERROR; }
#pragma aux GpiQueryFonts "GPIQUERYFONTS"

BOOL APIENTRY GpiQueryFontMetrics(HPS hps, LONG lMetricsLength,
                                    PFONTMETRICS pfmMetrics)
{ PMGPILOG("GpiQueryFontMetrics"); (void)hps;(void)lMetricsLength;(void)pfmMetrics; return FALSE; }
#pragma aux GpiQueryFontMetrics "GPIQUERYFONTMETRICS"

LONG APIENTRY GpiQueryKerningPairs(HPS hps, LONG lCount,
                                     PKERNINGPAIRS akrnprData)
{ PMGPILOG("GpiQueryKerningPairs"); (void)hps;(void)lCount;(void)akrnprData; return GPI_ERROR; }
#pragma aux GpiQueryKerningPairs "GPIQUERYKERNINGPAIRS"

BOOL APIENTRY GpiQueryWidthTable(HPS hps, LONG lFirstChar,
                                   LONG lCount, PLONG alData)
{ PMGPILOG("GpiQueryWidthTable"); (void)hps;(void)lFirstChar;(void)lCount;(void)alData; return FALSE; }
#pragma aux GpiQueryWidthTable "GPIQUERYWIDTHTABLE"

BOOL APIENTRY GpiSetCp(HPS hps, ULONG ulCodePage)
{ PMGPILOG("GpiSetCp"); (void)hps;(void)ulCodePage; return FALSE; }
#pragma aux GpiSetCp "GPISETCP"

ULONG APIENTRY GpiQueryCp(HPS hps)
{ PMGPILOG("GpiQueryCp"); (void)hps; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryCp "GPIQUERYCP"

BOOL APIENTRY GpiQueryTextBox(HPS hps, LONG lCount1, PCH pchString,
                                LONG lCount2, PPOINTL aptlPoints)
{ PMGPILOG("GpiQueryTextBox"); (void)hps;(void)lCount1;(void)pchString;(void)lCount2;(void)aptlPoints;
  return FALSE; }
#pragma aux GpiQueryTextBox "GPIQUERYTEXTBOX"

BOOL APIENTRY GpiQueryDefCharBox(HPS hps, PSIZEL psizlSize)
{ PMGPILOG("GpiQueryDefCharBox"); (void)hps;(void)psizlSize; return FALSE; }
#pragma aux GpiQueryDefCharBox "GPIQUERYDEFCHARBOX"

LONG APIENTRY GpiQueryFontFileDescriptions(HAB hab, PCSZ pszFilename,
                                              PLONG plCount,
                                              PFFDESCS affdescsNames)
{ PMGPILOG("GpiQueryFontFileDescriptions"); (void)hab;(void)pszFilename;(void)plCount;(void)affdescsNames; return GPI_ERROR; }
#pragma aux GpiQueryFontFileDescriptions "GPIQUERYFONTFILEDESCRIPTIONS"

/* ================================================================== */
/* Character attributes                                                */
/* ================================================================== */

BOOL APIENTRY GpiSetCharSet(HPS hps, LONG lSet)
{ PMGPILOG("GpiSetCharSet"); (void)hps;(void)lSet; return FALSE; }
#pragma aux GpiSetCharSet "GPISETCHARSET"

LONG APIENTRY GpiQueryCharSet(HPS hps)
{ PMGPILOG("GpiQueryCharSet"); (void)hps; return LCID_DEFAULT; }
#pragma aux GpiQueryCharSet "GPIQUERYCHARSET"

BOOL APIENTRY GpiSetCharBox(HPS hps, PSIZEF psizfxBox)
{ PMGPILOG("GpiSetCharBox"); (void)hps;(void)psizfxBox; return FALSE; }
#pragma aux GpiSetCharBox "GPISETCHARBOX"

BOOL APIENTRY GpiQueryCharBox(HPS hps, PSIZEF psizfxSize)
{ PMGPILOG("GpiQueryCharBox"); (void)hps;(void)psizfxSize; return FALSE; }
#pragma aux GpiQueryCharBox "GPIQUERYCHARBOX"

BOOL APIENTRY GpiSetCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{ PMGPILOG("GpiSetCharAngle"); (void)hps;(void)pgradlAngle; return FALSE; }
#pragma aux GpiSetCharAngle "GPISETCHARANGLE"

BOOL APIENTRY GpiQueryCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{ PMGPILOG("GpiQueryCharAngle"); (void)hps;(void)pgradlAngle; return FALSE; }
#pragma aux GpiQueryCharAngle "GPIQUERYCHARANGLE"

BOOL APIENTRY GpiSetCharShear(HPS hps, PPOINTL pptlAngle)
{ PMGPILOG("GpiSetCharShear"); (void)hps;(void)pptlAngle; return FALSE; }
#pragma aux GpiSetCharShear "GPISETCHARSHEAR"

BOOL APIENTRY GpiQueryCharShear(HPS hps, PPOINTL pptlShear)
{ PMGPILOG("GpiQueryCharShear"); (void)hps;(void)pptlShear; return FALSE; }
#pragma aux GpiQueryCharShear "GPIQUERYCHARSHEAR"

BOOL APIENTRY GpiSetCharDirection(HPS hps, LONG lDirection)
{ PMGPILOG("GpiSetCharDirection"); (void)hps;(void)lDirection; return FALSE; }
#pragma aux GpiSetCharDirection "GPISETCHARDIRECTION"

LONG APIENTRY GpiQueryCharDirection(HPS hps)
{ PMGPILOG("GpiQueryCharDirection"); (void)hps; return CHDIRN_DEFAULT; }
#pragma aux GpiQueryCharDirection "GPIQUERYCHARDIRECTION"

BOOL APIENTRY GpiSetCharMode(HPS hps, LONG lMode)
{ PMGPILOG("GpiSetCharMode"); (void)hps;(void)lMode; return FALSE; }
#pragma aux GpiSetCharMode "GPISETCHARMODE"

LONG APIENTRY GpiQueryCharMode(HPS hps)
{ PMGPILOG("GpiQueryCharMode"); (void)hps; return CM_DEFAULT; }
#pragma aux GpiQueryCharMode "GPIQUERYCHARMODE"

BOOL APIENTRY GpiSetCharExtra(HPS hps, LONG lExtra)
{ PMGPILOG("GpiSetCharExtra"); (void)hps;(void)lExtra; return FALSE; }
#pragma aux GpiSetCharExtra "GPISETCHAREXTRA"

ULONG APIENTRY GpiQueryCharExtra(HPS hps, PLONG plExtra)
{ PMGPILOG("GpiQueryCharExtra"); (void)hps;(void)plExtra; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryCharExtra "GPIQUERYCHAREXTRA"

BOOL APIENTRY GpiSetCharBreakExtra(HPS hps, LONG lBreakExtra)
{ PMGPILOG("GpiSetCharBreakExtra"); (void)hps;(void)lBreakExtra; return FALSE; }
#pragma aux GpiSetCharBreakExtra "GPISETCHARBREAKEXTRA"

ULONG APIENTRY GpiQueryCharBreakExtra(HPS hps, PLONG plBreakExtra)
{ PMGPILOG("GpiQueryCharBreakExtra"); (void)hps;(void)plBreakExtra; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryCharBreakExtra "GPIQUERYCHARBREAKEXTRA"

/* ================================================================== */
/* Character output                                                    */
/* ================================================================== */

LONG APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString)
{ PMGPILOG("GpiCharString"); (void)hps;(void)lCount;(void)pchString; return GPI_ERROR; }
#pragma aux GpiCharString "GPICHARSTRING"

LONG APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint,
                                LONG lCount, PCH pchString)
{ PMGPILOG("GpiCharStringAt"); (void)hps;(void)pptlPoint;(void)lCount;(void)pchString; return GPI_ERROR; }
#pragma aux GpiCharStringAt "GPICHARSTRINGAT"

LONG APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect, ULONG flOptions,
                                  LONG lCount, PCH pchString, PLONG alAdx)
{ PMGPILOG("GpiCharStringPos"); (void)hps;(void)prclRect;(void)flOptions;(void)lCount;
  (void)pchString;(void)alAdx; return GPI_ERROR; }
#pragma aux GpiCharStringPos "GPICHARSTRINGPOS"

LONG APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlStart,
                                    PRECTL prclRect, ULONG flOptions,
                                    LONG lCount, PCH pchString, PLONG alAdx)
{ PMGPILOG("GpiCharStringPosAt"); (void)hps;(void)pptlStart;(void)prclRect;(void)flOptions;
  (void)lCount;(void)pchString;(void)alAdx; return GPI_ERROR; }
#pragma aux GpiCharStringPosAt "GPICHARSTRINGPOSAT"

BOOL APIENTRY GpiQueryCharStringPos(HPS hps, ULONG flOptions, LONG lCount,
                                      PCH pchString, PLONG alAdx,
                                      PPOINTL aptlPos)
{ PMGPILOG("GpiQueryCharStringPos"); (void)hps;(void)flOptions;(void)lCount;(void)pchString;
  (void)alAdx;(void)aptlPos; return FALSE; }
#pragma aux GpiQueryCharStringPos "GPIQUERYCHARSTRINGPOS"

BOOL APIENTRY GpiQueryCharStringPosAt(HPS hps, PPOINTL pptlStart,
                                         ULONG flOptions, LONG lCount,
                                         PCH pchString, PLONG alAdx,
                                         PPOINTL aptlPos)
{ PMGPILOG("GpiQueryCharStringPosAt"); (void)hps;(void)pptlStart;(void)flOptions;(void)lCount;
  (void)pchString;(void)alAdx;(void)aptlPos; return FALSE; }
#pragma aux GpiQueryCharStringPosAt "GPIQUERYCHARSTRINGPOSAT"

/* ================================================================== */
/* Markers                                                             */
/* ================================================================== */

BOOL APIENTRY GpiSetMarkerSet(HPS hps, LONG lSet)
{ PMGPILOG("GpiSetMarkerSet"); (void)hps;(void)lSet; return FALSE; }
#pragma aux GpiSetMarkerSet "GPISETMARKERSET"

LONG APIENTRY GpiQueryMarkerSet(HPS hps)
{ PMGPILOG("GpiQueryMarkerSet"); (void)hps; return LCID_DEFAULT; }
#pragma aux GpiQueryMarkerSet "GPIQUERYMARKERSET"

BOOL APIENTRY GpiSetMarker(HPS hps, LONG lSymbol)
{ PMGPILOG("GpiSetMarker"); (void)hps;(void)lSymbol; return FALSE; }
#pragma aux GpiSetMarker "GPISETMARKER"

LONG APIENTRY GpiQueryMarker(HPS hps)
{ PMGPILOG("GpiQueryMarker"); (void)hps; return 0L; }   /* MARKSYM_DEFAULT (pmgpi.h guards it behind the marker INCL) */
#pragma aux GpiQueryMarker "GPIQUERYMARKER"

BOOL APIENTRY GpiSetMarkerBox(HPS hps, PSIZEF psizfxSize)
{ PMGPILOG("GpiSetMarkerBox"); (void)hps;(void)psizfxSize; return FALSE; }
#pragma aux GpiSetMarkerBox "GPISETMARKERBOX"

BOOL APIENTRY GpiQueryMarkerBox(HPS hps, PSIZEF psizfxSize)
{ PMGPILOG("GpiQueryMarkerBox"); (void)hps;(void)psizfxSize; return FALSE; }
#pragma aux GpiQueryMarkerBox "GPIQUERYMARKERBOX"

LONG APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiMarker"); (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiMarker "GPIMARKER"

LONG APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiPolyMarker"); (void)hps;(void)lCount;(void)aptlPoints; return GPI_ERROR; }
#pragma aux GpiPolyMarker "GPIPOLYMARKER"

/* ================================================================== */
/* Image                                                               */
/* ================================================================== */

LONG APIENTRY GpiImage(HPS hps, LONG lFormat, PSIZEL psizlImageSize,
                         LONG lLength, PBYTE pbData)
{ PMGPILOG("GpiImage"); (void)hps;(void)lFormat;(void)psizlImageSize;(void)lLength;(void)pbData;
  return GPI_ERROR; }
#pragma aux GpiImage "GPIIMAGE"

/* ================================================================== */
/* Bitmaps                                                             */
/* ================================================================== */

HBITMAP APIENTRY GpiCreateBitmap(HPS hps, PBITMAPINFOHEADER2 pbmpData,
                                    ULONG fl, PBYTE pbInitData,
                                    PBITMAPINFO2 pbmiInfoTable)
{ PMGPILOG("GpiCreateBitmap"); (void)hps;(void)pbmpData;(void)fl;(void)pbInitData;(void)pbmiInfoTable;
  return (HBITMAP)GPI_ERROR; }
#pragma aux GpiCreateBitmap "GPICREATEBITMAP"

BOOL APIENTRY GpiDeleteBitmap(HBITMAP hbm)
{ PMGPILOG("GpiDeleteBitmap"); (void)hbm; return FALSE; }
#pragma aux GpiDeleteBitmap "GPIDELETEBITMAP"

HBITMAP APIENTRY GpiSetBitmap(HPS hps, HBITMAP hbm)
{ PMGPILOG("GpiSetBitmap"); (void)hps;(void)hbm; return (HBITMAP)GPI_ERROR; }
#pragma aux GpiSetBitmap "GPISETBITMAP"

BOOL APIENTRY GpiSetBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{ PMGPILOG("GpiSetBitmapDimension"); (void)hbm;(void)psizlBitmapDimension; return FALSE; }
#pragma aux GpiSetBitmapDimension "GPISETBITMAPDIMENSION"

BOOL APIENTRY GpiQueryBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{ PMGPILOG("GpiQueryBitmapDimension"); (void)hbm;(void)psizlBitmapDimension; return FALSE; }
#pragma aux GpiQueryBitmapDimension "GPIQUERYBITMAPDIMENSION"

BOOL APIENTRY GpiQueryDeviceBitmapFormats(HPS hps, LONG lCount,
                                            PLONG alArray)
{ PMGPILOG("GpiQueryDeviceBitmapFormats"); (void)hps;(void)lCount;(void)alArray; return FALSE; }
#pragma aux GpiQueryDeviceBitmapFormats "GPIQUERYDEVICEBITMAPFORMATS"

BOOL APIENTRY GpiQueryBitmapParameters(HBITMAP hbm,
                                          PBITMAPINFOHEADER pbmpData)
{ PMGPILOG("GpiQueryBitmapParameters"); (void)hbm;(void)pbmpData; return FALSE; }
#pragma aux GpiQueryBitmapParameters "GPIQUERYBITMAPPARAMETERS"

BOOL APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm,
                                          PBITMAPINFOHEADER2 pbmpData)
{ PMGPILOG("GpiQueryBitmapInfoHeader"); (void)hbm;(void)pbmpData; return FALSE; }
#pragma aux GpiQueryBitmapInfoHeader "GPIQUERYBITMAPINFOHEADER"

LONG APIENTRY GpiSetBitmapBits(HPS hps, LONG lScanStart, LONG lScans,
                                  PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{ PMGPILOG("GpiSetBitmapBits"); (void)hps;(void)lScanStart;(void)lScans;(void)pbBuffer;(void)pbmiInfoTable;
  return GPI_ERROR; }
#pragma aux GpiSetBitmapBits "GPISETBITMAPBITS"

LONG APIENTRY GpiQueryBitmapBits(HPS hps, LONG lScanStart, LONG lScans,
                                    PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{ PMGPILOG("GpiQueryBitmapBits"); (void)hps;(void)lScanStart;(void)lScans;(void)pbBuffer;(void)pbmiInfoTable;
  return GPI_ERROR; }
#pragma aux GpiQueryBitmapBits "GPIQUERYBITMAPBITS"

LONG APIENTRY GpiBitBlt(HPS hpsTarget, HPS hpsSource, LONG lCount,
                          PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{ PMGPILOG("GpiBitBlt"); (void)hpsTarget;(void)hpsSource;(void)lCount;(void)aptlPoints;
  (void)lRop;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiBitBlt "GPIBITBLT"

LONG APIENTRY GpiWCBitBlt(HPS hpsTarget, HBITMAP hbmSource, LONG lCount,
                             PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{ PMGPILOG("GpiWCBitBlt"); (void)hpsTarget;(void)hbmSource;(void)lCount;(void)aptlPoints;
  (void)lRop;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiWCBitBlt "GPIWCBITBLT"

LONG APIENTRY GpiDrawBits(HPS hps, PVOID pBits,
                            PBITMAPINFO2 pbmiInfoTable, LONG lCount,
                            PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{ PMGPILOG("GpiDrawBits"); (void)hps;(void)pBits;(void)pbmiInfoTable;(void)lCount;
  (void)aptlPoints;(void)lRop;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiDrawBits "GPIDRAWBITS"

LONG APIENTRY GpiSetPel(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiSetPel"); (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiSetPel "GPISETPEL"

LONG APIENTRY GpiQueryPel(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiQueryPel"); (void)hps;(void)pptlPoint; return GPI_ALTERROR; }
#pragma aux GpiQueryPel "GPIQUERYPEL"

LONG APIENTRY GpiFloodFill(HPS hps, LONG lOptions, LONG lColor)
{ PMGPILOG("GpiFloodFill"); (void)hps;(void)lOptions;(void)lColor; return GPI_ERROR; }
#pragma aux GpiFloodFill "GPIFLOODFILL"

/* ================================================================== */
/* Regions                                                             */
/* ================================================================== */

HRGN APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles)
{ PMGPILOG("GpiCreateRegion"); (void)hps;(void)lCount;(void)arclRectangles; return (HRGN)GPI_ERROR; }
#pragma aux GpiCreateRegion "GPICREATEREGION"

BOOL APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lCount,
                              PRECTL arclRectangles)
{ PMGPILOG("GpiSetRegion"); (void)hps;(void)hrgn;(void)lCount;(void)arclRectangles; return FALSE; }
#pragma aux GpiSetRegion "GPISETREGION"

BOOL APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn)
{ PMGPILOG("GpiDestroyRegion"); (void)hps;(void)hrgn; return FALSE; }
#pragma aux GpiDestroyRegion "GPIDESTROYREGION"

LONG APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest,
                                  HRGN hrgnSrc1, HRGN hrgnSrc2, LONG lMode)
{ PMGPILOG("GpiCombineRegion"); (void)hps;(void)hrgnDest;(void)hrgnSrc1;(void)hrgnSrc2;(void)lMode;
  return GPI_ERROR; }
#pragma aux GpiCombineRegion "GPICOMBINEREGION"

LONG APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2)
{ PMGPILOG("GpiEqualRegion"); (void)hps;(void)hrgnSrc1;(void)hrgnSrc2; return GPI_ERROR; }
#pragma aux GpiEqualRegion "GPIEQUALREGION"

BOOL APIENTRY GpiOffsetRegion(HPS hps, HRGN hrgn, PPOINTL pptlOffset)
{ PMGPILOG("GpiOffsetRegion"); (void)hps;(void)hrgn;(void)pptlOffset; return FALSE; }
#pragma aux GpiOffsetRegion "GPIOFFSETREGION"

LONG APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint)
{ PMGPILOG("GpiPtInRegion"); (void)hps;(void)hrgn;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiPtInRegion "GPIPTINREGION"

LONG APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect)
{ PMGPILOG("GpiRectInRegion"); (void)hps;(void)hrgn;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiRectInRegion "GPIRECTINREGION"

LONG APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound)
{ PMGPILOG("GpiQueryRegionBox"); (void)hps;(void)hrgn;(void)prclBound; return GPI_ERROR; }
#pragma aux GpiQueryRegionBox "GPIQUERYREGIONBOX"

BOOL APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound,
                                     PRGNRECT prgnrcControl, PRECTL arclRects)
{ PMGPILOG("GpiQueryRegionRects"); (void)hps;(void)hrgn;(void)prclBound;(void)prgnrcControl;(void)arclRects;
  return FALSE; }
#pragma aux GpiQueryRegionRects "GPIQUERYREGIONRECTS"

LONG APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld)
{ PMGPILOG("GpiSetClipRegion"); (void)hps;(void)hrgn;(void)phrgnOld; return GPI_ERROR; }
#pragma aux GpiSetClipRegion "GPISETCLIPREGION"

HRGN APIENTRY GpiQueryClipRegion(HPS hps)
{ PMGPILOG("GpiQueryClipRegion"); (void)hps; return (HRGN)GPI_ERROR; }
#pragma aux GpiQueryClipRegion "GPIQUERYCLIPREGION"

LONG APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound)
{ PMGPILOG("GpiQueryClipBox"); (void)hps;(void)prclBound; return GPI_ERROR; }
#pragma aux GpiQueryClipBox "GPIQUERYCLIPBOX"

LONG APIENTRY GpiIntersectClipRectangle(HPS hps, PRECTL prclRect)
{ PMGPILOG("GpiIntersectClipRectangle"); (void)hps;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiIntersectClipRectangle "GPIINTERSECTCLIPRECTANGLE"

LONG APIENTRY GpiExcludeClipRectangle(HPS hps, PRECTL prclRect)
{ PMGPILOG("GpiExcludeClipRectangle"); (void)hps;(void)prclRect; return GPI_ERROR; }
#pragma aux GpiExcludeClipRectangle "GPIEXCLUDECLIPRECTANGLE"

LONG APIENTRY GpiOffsetClipRegion(HPS hps, PPOINTL pptlPoint)
{ PMGPILOG("GpiOffsetClipRegion"); (void)hps;(void)pptlPoint; return GPI_ERROR; }
#pragma aux GpiOffsetClipRegion "GPIOFFSETCLIPREGION"

LONG APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn)
{ PMGPILOG("GpiPaintRegion"); (void)hps;(void)hrgn; return GPI_ERROR; }
#pragma aux GpiPaintRegion "GPIPAINTREGION"

LONG APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness)
{ PMGPILOG("GpiFrameRegion"); (void)hps;(void)hrgn;(void)thickness; return GPI_ERROR; }
#pragma aux GpiFrameRegion "GPIFRAMEREGION"

/* ================================================================== */
/* Metafiles                                                           */
/* ================================================================== */

HMF APIENTRY GpiLoadMetaFile(HAB hab, PCSZ pszFilename)
{ PMGPILOG("GpiLoadMetaFile"); (void)hab;(void)pszFilename; return (HMF)GPI_ERROR; }
#pragma aux GpiLoadMetaFile "GPILOADMETAFILE"

HMF APIENTRY GpiCopyMetaFile(HMF hmf)
{ PMGPILOG("GpiCopyMetaFile"); (void)hmf; return (HMF)GPI_ERROR; }
#pragma aux GpiCopyMetaFile "GPICOPYMETAFILE"

LONG APIENTRY GpiPlayMetaFile(HPS hps, HMF hmf, LONG lCount1,
                                PLONG alOptarray, PLONG plSegCount,
                                LONG lCount2, PCSZ pszDesc)
{ PMGPILOG("GpiPlayMetaFile"); (void)hps;(void)hmf;(void)lCount1;(void)alOptarray;(void)plSegCount;
  (void)lCount2;(void)pszDesc; return GPI_ERROR; }
#pragma aux GpiPlayMetaFile "GPIPLAYMETAFILE"

BOOL APIENTRY GpiSaveMetaFile(HMF hmf, PCSZ pszFilename)
{ PMGPILOG("GpiSaveMetaFile"); (void)hmf;(void)pszFilename; return FALSE; }
#pragma aux GpiSaveMetaFile "GPISAVEMETAFILE"

BOOL APIENTRY GpiDeleteMetaFile(HMF hmf)
{ PMGPILOG("GpiDeleteMetaFile"); (void)hmf; return FALSE; }
#pragma aux GpiDeleteMetaFile "GPIDELETEMETAFILE"

ULONG APIENTRY GpiQueryMetaFileBits(HMF hmf, LONG lOffset,
                                      LONG lLength, PBYTE pbData)
{ PMGPILOG("GpiQueryMetaFileBits"); (void)hmf;(void)lOffset;(void)lLength;(void)pbData; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryMetaFileBits "GPIQUERYMETAFILEBITS"

BOOL APIENTRY GpiSetMetaFileBits(HMF hmf, LONG lOffset,
                                    LONG lLength, PBYTE pbBuffer)
{ PMGPILOG("GpiSetMetaFileBits"); (void)hmf;(void)lOffset;(void)lLength;(void)pbBuffer; return FALSE; }
#pragma aux GpiSetMetaFileBits "GPISETMETAFILEBITS"

LONG APIENTRY GpiQueryMetaFileLength(HMF hmf)
{ PMGPILOG("GpiQueryMetaFileLength"); (void)hmf; return GPI_ALTERROR; }
#pragma aux GpiQueryMetaFileLength "GPIQUERYMETAFILELENGTH"

HMF APIENTRY GpiOpenMetaFile(HAB hab)
{ PMGPILOG("GpiOpenMetaFile"); (void)hab; return (HMF)GPI_ERROR; }
#pragma aux GpiOpenMetaFile "GPIOPENMETAFILE"

BOOL APIENTRY GpiCloseMetaFile(HMF hmf)
{ PMGPILOG("GpiCloseMetaFile"); (void)hmf; return FALSE; }
#pragma aux GpiCloseMetaFile "GPICLOSEMETAFILE"

BOOL APIENTRY GpiAccessMetaFile(HMF hmf)
{ PMGPILOG("GpiAccessMetaFile"); (void)hmf; return FALSE; }
#pragma aux GpiAccessMetaFile "GPIACCESSMETAFILE"

BOOL APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData)
{ PMGPILOG("GpiComment"); (void)hps;(void)lLength;(void)pbData; return FALSE; }
#pragma aux GpiComment "GPICOMMENT"

/* ================================================================== */
/* Paths and clipping                                                  */
/* ================================================================== */

BOOL APIENTRY GpiSetClipPath(HPS hps, LONG lPath, LONG lOptions)
{ PMGPILOG("GpiSetClipPath"); (void)hps;(void)lPath;(void)lOptions; return FALSE; }
#pragma aux GpiSetClipPath "GPISETCLIPPATH"

BOOL APIENTRY GpiBeginPath(HPS hps, LONG lPath)
{ PMGPILOG("GpiBeginPath"); (void)hps;(void)lPath; return FALSE; }
#pragma aux GpiBeginPath "GPIBEGINPATH"

BOOL APIENTRY GpiEndPath(HPS hps)
{ PMGPILOG("GpiEndPath"); (void)hps; return FALSE; }
#pragma aux GpiEndPath "GPIENDPATH"

BOOL APIENTRY GpiCloseFigure(HPS hps)
{ PMGPILOG("GpiCloseFigure"); (void)hps; return FALSE; }
#pragma aux GpiCloseFigure "GPICLOSEFIGURE"

BOOL APIENTRY GpiModifyPath(HPS hps, LONG lPath, LONG lMode)
{ PMGPILOG("GpiModifyPath"); (void)hps;(void)lPath;(void)lMode; return FALSE; }
#pragma aux GpiModifyPath "GPIMODIFYPATH"

LONG APIENTRY GpiFillPath(HPS hps, LONG lPath, LONG lOptions)
{ PMGPILOG("GpiFillPath"); (void)hps;(void)lPath;(void)lOptions; return GPI_ERROR; }
#pragma aux GpiFillPath "GPIFILLPATH"

LONG APIENTRY GpiStrokePath(HPS hps, LONG lPath, ULONG flOptions)
{ PMGPILOG("GpiStrokePath"); (void)hps;(void)lPath;(void)flOptions; return GPI_ERROR; }
#pragma aux GpiStrokePath "GPISTROKEPATH"

LONG APIENTRY GpiOutlinePath(HPS hps, LONG lPath, LONG lOptions)
{ PMGPILOG("GpiOutlinePath"); (void)hps;(void)lPath;(void)lOptions; return GPI_ERROR; }
#pragma aux GpiOutlinePath "GPIOUTLINEPATH"

HRGN APIENTRY GpiPathToRegion(HPS hps, LONG lPath, LONG lOptions)
{ PMGPILOG("GpiPathToRegion"); (void)hps;(void)lPath;(void)lOptions; return (HRGN)GPI_ERROR; }
#pragma aux GpiPathToRegion "GPIPATHTOREGION"

/* ================================================================== */
/* Miscellaneous GPI                                                   */
/* ================================================================== */

HDC APIENTRY GpiQueryDevice(HPS hps)
{ PMGPILOG("GpiQueryDevice"); (void)hps; return (HDC)GPI_ERROR; }
#pragma aux GpiQueryDevice "GPIQUERYDEVICE"

BOOL APIENTRY GpiVectorSymbol(HPS hps, LONG lCount, PPOINTL aptlPoints)
{ PMGPILOG("GpiVectorSymbol"); (void)hps;(void)lCount;(void)aptlPoints; return FALSE; }
#pragma aux GpiVectorSymbol "GPIVECTORSYMBOL"

BOOL APIENTRY GpiConvPSH(HPS hps, LONG lAction, PVOID pvConvData)
{ PMGPILOG("GpiConvPSH"); (void)hps;(void)lAction;(void)pvConvData; return FALSE; }
#pragma aux GpiConvPSH "GPICONVPSH"

BOOL APIENTRY GpiSetPS(HPS hps, PSIZEL psizl, ULONG fl)
{ PMGPILOG("GpiSetPS"); (void)hps;(void)psizl;(void)fl; return FALSE; }
#pragma aux GpiSetPS "GPISETPS"

BOOL APIENTRY GpiSetDCOwner(HDC hdc, ULONG idProcess)
{ PMGPILOG("GpiSetDCOwner"); (void)hdc;(void)idProcess; return FALSE; }
#pragma aux GpiSetDCOwner "GPISETDCOWNER"

BOOL APIENTRY GpiSetBitmapOwner(HBITMAP hbm, ULONG idProcess)
{ PMGPILOG("GpiSetBitmapOwner"); (void)hbm;(void)idProcess; return FALSE; }
#pragma aux GpiSetBitmapOwner "GPISETBITMAPOWNER"

BOOL APIENTRY GpiSetRegionOwner(HPS hps, HRGN hrgn, ULONG idProcess)
{ PMGPILOG("GpiSetRegionOwner"); (void)hps;(void)hrgn;(void)idProcess; return FALSE; }
#pragma aux GpiSetRegionOwner "GPISETREGIONOWNER"

BOOL APIENTRY GpiInstallIFIFont(HAB hab, PCSZ pszFileName)
{ PMGPILOG("GpiInstallIFIFont"); (void)hab;(void)pszFileName; return FALSE; }
#pragma aux GpiInstallIFIFont "GPIINSTALLIFIFONT"

/* ================================================================== */
/* Default view / def attrs                                            */
/* ================================================================== */

BOOL APIENTRY GpiSetDefTag(HPS hps, LONG lTag)
{ PMGPILOG("GpiSetDefTag"); (void)hps;(void)lTag; return FALSE; }
#pragma aux GpiSetDefTag "GPISETDEFTAG"

ULONG APIENTRY GpiQueryDefTag(HPS hps, PLONG plTag)
{ PMGPILOG("GpiQueryDefTag"); (void)hps;(void)plTag; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryDefTag "GPIQUERYDEFTAG"

BOOL APIENTRY GpiSetDefAttrs(HPS hps, LONG lPrimType,
                               ULONG flAttrMask, PBUNDLE ppbunAttrs)
{ PMGPILOG("GpiSetDefAttrs"); (void)hps;(void)lPrimType;(void)flAttrMask;(void)ppbunAttrs; return FALSE; }
#pragma aux GpiSetDefAttrs "GPISETDEFATTRS"

BOOL APIENTRY GpiQueryDefAttrs(HPS hps, LONG lPrimType,
                                  ULONG flAttrMask, PBUNDLE ppbunAttrs)
{ PMGPILOG("GpiQueryDefAttrs"); (void)hps;(void)lPrimType;(void)flAttrMask;(void)ppbunAttrs; return FALSE; }
#pragma aux GpiQueryDefAttrs "GPIQUERYDEFATTRS"

BOOL APIENTRY GpiSetDefViewingLimits(HPS hps, PRECTL prclLimits)
{ PMGPILOG("GpiSetDefViewingLimits"); (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiSetDefViewingLimits "GPISETDEFVIEWINGLIMITS"

BOOL APIENTRY GpiQueryDefViewingLimits(HPS hps, PRECTL prclLimits)
{ PMGPILOG("GpiQueryDefViewingLimits"); (void)hps;(void)prclLimits; return FALSE; }
#pragma aux GpiQueryDefViewingLimits "GPIQUERYDEFVIEWINGLIMITS"

BOOL APIENTRY GpiSetDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ PMGPILOG("GpiSetDefArcParams"); (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiSetDefArcParams "GPISETDEFARCPARAMS"

BOOL APIENTRY GpiQueryDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{ PMGPILOG("GpiQueryDefArcParams"); (void)hps;(void)parcpArcParams; return FALSE; }
#pragma aux GpiQueryDefArcParams "GPIQUERYDEFARCPARAMS"

/* ================================================================== */
/* Palette                                                             */
/* ================================================================== */

HPAL APIENTRY GpiCreatePalette(HAB hab, ULONG flOptions, ULONG ulFormat,
                                  ULONG ulCount, PULONG aulTable)
{ PMGPILOG("GpiCreatePalette"); (void)hab;(void)flOptions;(void)ulFormat;(void)ulCount;(void)aulTable;
  return (HPAL)GPI_ERROR; }
#pragma aux GpiCreatePalette "GPICREATEPALETTE"

BOOL APIENTRY GpiDeletePalette(HPAL hpal)
{ PMGPILOG("GpiDeletePalette"); (void)hpal; return FALSE; }
#pragma aux GpiDeletePalette "GPIDELETEPALETTE"

HPAL APIENTRY GpiSelectPalette(HPS hps, HPAL hpal)
{ PMGPILOG("GpiSelectPalette"); (void)hps;(void)hpal; return (HPAL)GPI_ERROR; }
#pragma aux GpiSelectPalette "GPISELECTPALETTE"

LONG APIENTRY GpiAnimatePalette(HPS hps, HPAL hpal, ULONG ulFormat,
                                   ULONG ulStart, PULONG aulTable)
{ PMGPILOG("GpiAnimatePalette"); (void)hps;(void)hpal;(void)ulFormat;(void)ulStart;
  (void)aulTable; return GPI_ERROR; }
#pragma aux GpiAnimatePalette "GPIANIMATEPALETTE"

BOOL APIENTRY GpiSetPaletteEntries(HPAL hpal, ULONG ulFormat,
                                      ULONG ulStart, ULONG ulCount,
                                      PULONG aulTable)
{ PMGPILOG("GpiSetPaletteEntries"); (void)hpal;(void)ulFormat;(void)ulStart;(void)ulCount;(void)aulTable;
  return FALSE; }
#pragma aux GpiSetPaletteEntries "GPISETPALETTEENTRIES"

HPAL APIENTRY GpiQueryPalette(HPS hps)
{ PMGPILOG("GpiQueryPalette"); (void)hps; return (HPAL)GPI_ERROR; }
#pragma aux GpiQueryPalette "GPIQUERYPALETTE"

LONG APIENTRY GpiQueryPaletteInfo(HPAL hpal, HPS hps, ULONG flOptions,
                                     ULONG ulStart, ULONG ulCount,
                                     PULONG aulArray)
{ PMGPILOG("GpiQueryPaletteInfo"); (void)hpal;(void)hps;(void)flOptions;(void)ulStart;
  (void)ulCount;(void)aulArray; return GPI_ERROR; }
#pragma aux GpiQueryPaletteInfo "GPIQUERYPALETTEINFO"

/* ================================================================== */
/* Public fonts                                                        */
/* ================================================================== */

BOOL APIENTRY GpiLoadPublicFonts(HAB hab, PCSZ pszFilename)
{ PMGPILOG("GpiLoadPublicFonts"); (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiLoadPublicFonts "GPILOADPUBLICFONTS"

BOOL APIENTRY GpiUnloadPublicFonts(HAB hab, PCSZ pszFilename)
{ PMGPILOG("GpiUnloadPublicFonts"); (void)hab;(void)pszFilename; return FALSE; }
#pragma aux GpiUnloadPublicFonts "GPIUNLOADPUBLICFONTS"

BOOL APIENTRY GpiQueryLogicalFont(HPS hps, LONG lLcid, PSTR8 pName,
                                     PFATTRS pfAttrs, LONG lLength)
{ PMGPILOG("GpiQueryLogicalFont"); (void)hps;(void)lLcid;(void)pName;(void)pfAttrs;(void)lLength; return FALSE;}
#pragma aux GpiQueryLogicalFont "GPIQUERYLOGICALFONT"

BOOL APIENTRY GpiQueryFaceString(HPS hps, PCSZ pszFamilyname,
                                    PFACENAMEDESC pfnd, LONG lLength,
                                    PCSZ pszFacename)
{ PMGPILOG("GpiQueryFaceString"); (void)hps;(void)pszFamilyname;(void)pfnd;(void)lLength;(void)pszFacename;
  return FALSE; }
#pragma aux GpiQueryFaceString "GPIQUERYFACESTRING"

ULONG APIENTRY GpiQueryFontAction(HAB hab, ULONG ulOptions)
{ PMGPILOG("GpiQueryFontAction"); (void)hab;(void)ulOptions; return (ULONG)GPI_ERROR; }
#pragma aux GpiQueryFontAction "GPIQUERYFONTACTION"

/* ================================================================== */
/* Play/suspend                                                        */
/* ================================================================== */

BOOL APIENTRY GpiSuspendPlay(HPS hps)
{ PMGPILOG("GpiSuspendPlay"); (void)hps; return FALSE; }
#pragma aux GpiSuspendPlay "GPISUSPENDPLAY"

BOOL APIENTRY GpiResumePlay(HPS hps)
{ PMGPILOG("GpiResumePlay"); (void)hps; return FALSE; }
#pragma aux GpiResumePlay "GPIRESUMEPLAY"

/* ================================================================== */
/* MT (metafile/graphics streaming) functions                          */
/* ================================================================== */

BOOL APIENTRY MtStartReadRequest(PVOID pvMtCtl)
{ PMGPILOG("MtStartReadRequest"); (void)pvMtCtl; return FALSE; }
#pragma aux MtStartReadRequest "MTSTARTREADREQUEST"

BOOL APIENTRY MtEndReadRequest(PVOID pvMtCtl)
{ PMGPILOG("MtEndReadRequest"); (void)pvMtCtl; return FALSE; }
#pragma aux MtEndReadRequest "MTENDREADREQUEST"

BOOL APIENTRY MtGetDescription(PVOID pvMtCtl, PVOID pvDesc)
{ PMGPILOG("MtGetDescription"); (void)pvMtCtl;(void)pvDesc; return FALSE; }
#pragma aux MtGetDescription "MTGETDESCRIPTION"

BOOL APIENTRY MtGetCodePage(PVOID pvMtCtl, PULONG pulCP)
{ PMGPILOG("MtGetCodePage"); (void)pvMtCtl;(void)pulCP; return FALSE; }
#pragma aux MtGetCodePage "MTGETCODEPAGE"

BOOL APIENTRY MtGetLCT(PVOID pvMtCtl, PVOID pvLCT)
{ PMGPILOG("MtGetLCT"); (void)pvMtCtl;(void)pvLCT; return FALSE; }
#pragma aux MtGetLCT "MTGETLCT"

BOOL APIENTRY MtGetGDDInfo(PVOID pvMtCtl, PVOID pvGDDInfo)
{ PMGPILOG("MtGetGDDInfo"); (void)pvMtCtl;(void)pvGDDInfo; return FALSE; }
#pragma aux MtGetGDDInfo "MTGETGDDINFO"

BOOL APIENTRY MtGetFirstFont(PVOID pvMtCtl, PVOID pvFontData)
{ PMGPILOG("MtGetFirstFont"); (void)pvMtCtl;(void)pvFontData; return FALSE; }
#pragma aux MtGetFirstFont "MTGETFIRSTFONT"

BOOL APIENTRY MtGetNextFont(PVOID pvMtCtl, PVOID pvFontData)
{ PMGPILOG("MtGetNextFont"); (void)pvMtCtl;(void)pvFontData; return FALSE; }
#pragma aux MtGetNextFont "MTGETNEXTFONT"

BOOL APIENTRY MtGetFirstGraphicsData(PVOID pvMtCtl, PVOID pvGfxData)
{ PMGPILOG("MtGetFirstGraphicsData"); (void)pvMtCtl;(void)pvGfxData; return FALSE; }
#pragma aux MtGetFirstGraphicsData "MTGETFIRSTGRAPHICSDATA"

BOOL APIENTRY MtGetNextGraphicsData(PVOID pvMtCtl, PVOID pvGfxData)
{ PMGPILOG("MtGetNextGraphicsData"); (void)pvMtCtl;(void)pvGfxData; return FALSE; }
#pragma aux MtGetNextGraphicsData "MTGETNEXTGRAPHICSDATA"

/* ================================================================== */
/* SEGSW segment window operations                                     */
/* ================================================================== */

BOOL APIENTRY SegSGWOpenSegmentWindow(PVOID pvSGWCtl, PVOID pvWinData)
{ PMGPILOG("SegSGWOpenSegmentWindow"); (void)pvSGWCtl;(void)pvWinData; return FALSE; }
#pragma aux SegSGWOpenSegmentWindow "SEGSGWOPENSEGMENTWINDOW"

BOOL APIENTRY SegSGWNewPartData(PVOID pvSGWCtl, PVOID pvPartData)
{ PMGPILOG("SegSGWNewPartData"); (void)pvSGWCtl;(void)pvPartData; return FALSE; }
#pragma aux SegSGWNewPartData "SEGSGWNEWPARTDATA"

BOOL APIENTRY SegSGWNextOrderF(PVOID pvSGWCtl, PVOID pvOrderData)
{ PMGPILOG("SegSGWNextOrderF"); (void)pvSGWCtl;(void)pvOrderData; return FALSE; }
#pragma aux SegSGWNextOrderF "SEGSGWNEXTORDERF"

/* ================================================================== */
/* MT association                                                      */
/* ================================================================== */

BOOL APIENTRY GpiMtAssociate(HPS hps, PVOID pvMtCtl)
{ PMGPILOG("GpiMtAssociate"); (void)hps;(void)pvMtCtl; return FALSE; }
#pragma aux GpiMtAssociate "GPIMTASSOCIATE"

BOOL APIENTRY GpiMtDisassociate(HPS hps, PVOID pvMtCtl)
{ PMGPILOG("GpiMtDisassociate"); (void)hps;(void)pvMtCtl; return FALSE; }
#pragma aux GpiMtDisassociate "GPIMTDISASSOCIATE"

/* ================================================================== */
/* Format conversion                                                   */
/* ================================================================== */

BOOL APIENTRY FmtConvertGOCAPolyLine(PVOID pvIn, PVOID pvOut, PULONG pulLen)
{ PMGPILOG("FmtConvertGOCAPolyLine"); (void)pvIn;(void)pvOut;(void)pulLen; return FALSE; }
#pragma aux FmtConvertGOCAPolyLine "FMTCONVERTGOCAPOLY"

/* ================================================================== */
/* Ink path                                                            */
/* ================================================================== */

BOOL APIENTRY GpiBeginInkPath(HPS hps, LONG lPath, ULONG flOptions)
{ PMGPILOG("GpiBeginInkPath"); (void)hps;(void)lPath;(void)flOptions; return FALSE; }
#pragma aux GpiBeginInkPath "GPIBEGININKPATH"

BOOL APIENTRY GpiEndInkPath(HPS hps)
{ PMGPILOG("GpiEndInkPath"); (void)hps; return FALSE; }
#pragma aux GpiEndInkPath "GPIENDINKPATH"

BOOL APIENTRY GpiStrokeInkPath(HPS hps, LONG lPath, ULONG flOptions)
{ PMGPILOG("GpiStrokeInkPath"); (void)hps;(void)lPath;(void)flOptions; return FALSE; }
#pragma aux GpiStrokeInkPath "GPISTROKEINKPATH"

/* ================================================================== */
/* DSP                                                                 */
/* ================================================================== */

BOOL APIENTRY DspInitSystemDriverName(PCSZ pszDriverName)
{ PMGPILOG("DspInitSystemDriverName"); (void)pszDriverName; return FALSE; }
#pragma aux DspInitSystemDriverName "DSPINITSYSTEMDRIVERNAME"

/* ================================================================== */
/* By-ordinal exports of the original PMGPI.DLL that have no name in its    */
/* name table (339 x 32-bit + 1 forwarder). Required for ORDINAL PARITY --  */
/* other system DLLs import these by number (e.g. PMSPL->PMGPI.604) and a   */
/* single missing ordinal is a hard boot stop. APIENTRY (_System) is        */
/* caller-cleanup, so a zero-arg stub is stack-safe for any real arg count. */
/* Each logs OrdNNN so the boot log reveals which mystery ordinals are hot. */
/* ================================================================== */
LONG APIENTRY PmgpiOrd351(void) { PMGPILOG("Ord351"); return 0L; }
#pragma aux PmgpiOrd351 "PMGPIORD351"
LONG APIENTRY PmgpiOrd352(void) { PMGPILOG("Ord352"); return 0L; }
#pragma aux PmgpiOrd352 "PMGPIORD352"
LONG APIENTRY PmgpiOrd353(void) { PMGPILOG("Ord353"); return 0L; }
#pragma aux PmgpiOrd353 "PMGPIORD353"
LONG APIENTRY PmgpiOrd354(void) { PMGPILOG("Ord354"); return 0L; }
#pragma aux PmgpiOrd354 "PMGPIORD354"
LONG APIENTRY PmgpiOrd355(void) { PMGPILOG("Ord355"); return 0L; }
#pragma aux PmgpiOrd355 "PMGPIORD355"
LONG APIENTRY PmgpiOrd356(void) { PMGPILOG("Ord356"); return 0L; }
#pragma aux PmgpiOrd356 "PMGPIORD356"
LONG APIENTRY PmgpiOrd357(void) { PMGPILOG("Ord357"); return 0L; }
#pragma aux PmgpiOrd357 "PMGPIORD357"
LONG APIENTRY PmgpiOrd358(void) { PMGPILOG("Ord358"); return 0L; }
#pragma aux PmgpiOrd358 "PMGPIORD358"
LONG APIENTRY PmgpiOrd359(void) { PMGPILOG("Ord359"); return 0L; }
#pragma aux PmgpiOrd359 "PMGPIORD359"
LONG APIENTRY PmgpiOrd360(void) { PMGPILOG("Ord360"); return 0L; }
#pragma aux PmgpiOrd360 "PMGPIORD360"
LONG APIENTRY PmgpiOrd361(void) { PMGPILOG("Ord361"); return 0L; }
#pragma aux PmgpiOrd361 "PMGPIORD361"
LONG APIENTRY PmgpiOrd362(void) { PMGPILOG("Ord362"); return 0L; }
#pragma aux PmgpiOrd362 "PMGPIORD362"
LONG APIENTRY PmgpiOrd363(void) { PMGPILOG("Ord363"); return 0L; }
#pragma aux PmgpiOrd363 "PMGPIORD363"
LONG APIENTRY PmgpiOrd364(void) { PMGPILOG("Ord364"); return 0L; }
#pragma aux PmgpiOrd364 "PMGPIORD364"
LONG APIENTRY PmgpiOrd365(void) { PMGPILOG("Ord365"); return 0L; }
#pragma aux PmgpiOrd365 "PMGPIORD365"
LONG APIENTRY PmgpiOrd366(void) { PMGPILOG("Ord366"); return 0L; }
#pragma aux PmgpiOrd366 "PMGPIORD366"
LONG APIENTRY PmgpiOrd367(void) { PMGPILOG("Ord367"); return 0L; }
#pragma aux PmgpiOrd367 "PMGPIORD367"
LONG APIENTRY PmgpiOrd368(void) { PMGPILOG("Ord368"); return 0L; }
#pragma aux PmgpiOrd368 "PMGPIORD368"
LONG APIENTRY PmgpiOrd369(void) { PMGPILOG("Ord369"); return 0L; }
#pragma aux PmgpiOrd369 "PMGPIORD369"
LONG APIENTRY PmgpiOrd370(void) { PMGPILOG("Ord370"); return 0L; }
#pragma aux PmgpiOrd370 "PMGPIORD370"
LONG APIENTRY PmgpiOrd371(void) { PMGPILOG("Ord371"); return 0L; }
#pragma aux PmgpiOrd371 "PMGPIORD371"
LONG APIENTRY PmgpiOrd372(void) { PMGPILOG("Ord372"); return 0L; }
#pragma aux PmgpiOrd372 "PMGPIORD372"
LONG APIENTRY PmgpiOrd373(void) { PMGPILOG("Ord373"); return 0L; }
#pragma aux PmgpiOrd373 "PMGPIORD373"
LONG APIENTRY PmgpiOrd374(void) { PMGPILOG("Ord374"); return 0L; }
#pragma aux PmgpiOrd374 "PMGPIORD374"
LONG APIENTRY PmgpiOrd375(void) { PMGPILOG("Ord375"); return 0L; }
#pragma aux PmgpiOrd375 "PMGPIORD375"
LONG APIENTRY PmgpiOrd376(void) { PMGPILOG("Ord376"); return 0L; }
#pragma aux PmgpiOrd376 "PMGPIORD376"
LONG APIENTRY PmgpiOrd377(void) { PMGPILOG("Ord377"); return 0L; }
#pragma aux PmgpiOrd377 "PMGPIORD377"
LONG APIENTRY PmgpiOrd378(void) { PMGPILOG("Ord378"); return 0L; }
#pragma aux PmgpiOrd378 "PMGPIORD378"
LONG APIENTRY PmgpiOrd379(void) { PMGPILOG("Ord379"); return 0L; }
#pragma aux PmgpiOrd379 "PMGPIORD379"
LONG APIENTRY PmgpiOrd380(void) { PMGPILOG("Ord380"); return 0L; }
#pragma aux PmgpiOrd380 "PMGPIORD380"
LONG APIENTRY PmgpiOrd381(void) { PMGPILOG("Ord381"); return 0L; }
#pragma aux PmgpiOrd381 "PMGPIORD381"
LONG APIENTRY PmgpiOrd382(void) { PMGPILOG("Ord382"); return 0L; }
#pragma aux PmgpiOrd382 "PMGPIORD382"
LONG APIENTRY PmgpiOrd383(void) { PMGPILOG("Ord383"); return 0L; }
#pragma aux PmgpiOrd383 "PMGPIORD383"
LONG APIENTRY PmgpiOrd384(void) { PMGPILOG("Ord384"); return 0L; }
#pragma aux PmgpiOrd384 "PMGPIORD384"
LONG APIENTRY PmgpiOrd385(void) { PMGPILOG("Ord385"); return 0L; }
#pragma aux PmgpiOrd385 "PMGPIORD385"
LONG APIENTRY PmgpiOrd386(void) { PMGPILOG("Ord386"); return 0L; }
#pragma aux PmgpiOrd386 "PMGPIORD386"
LONG APIENTRY PmgpiOrd387(void) { PMGPILOG("Ord387"); return 0L; }
#pragma aux PmgpiOrd387 "PMGPIORD387"
LONG APIENTRY PmgpiOrd388(void) { PMGPILOG("Ord388"); return 0L; }
#pragma aux PmgpiOrd388 "PMGPIORD388"
LONG APIENTRY PmgpiOrd389(void) { PMGPILOG("Ord389"); return 0L; }
#pragma aux PmgpiOrd389 "PMGPIORD389"
LONG APIENTRY PmgpiOrd390(void) { PMGPILOG("Ord390"); return 0L; }
#pragma aux PmgpiOrd390 "PMGPIORD390"
LONG APIENTRY PmgpiOrd391(void) { PMGPILOG("Ord391"); return 0L; }
#pragma aux PmgpiOrd391 "PMGPIORD391"
LONG APIENTRY PmgpiOrd392(void) { PMGPILOG("Ord392"); return 0L; }
#pragma aux PmgpiOrd392 "PMGPIORD392"
LONG APIENTRY PmgpiOrd393(void) { PMGPILOG("Ord393"); return 0L; }
#pragma aux PmgpiOrd393 "PMGPIORD393"
LONG APIENTRY PmgpiOrd394(void) { PMGPILOG("Ord394"); return 0L; }
#pragma aux PmgpiOrd394 "PMGPIORD394"
LONG APIENTRY PmgpiOrd395(void) { PMGPILOG("Ord395"); return 0L; }
#pragma aux PmgpiOrd395 "PMGPIORD395"
LONG APIENTRY PmgpiOrd396(void) { PMGPILOG("Ord396"); return 0L; }
#pragma aux PmgpiOrd396 "PMGPIORD396"
LONG APIENTRY PmgpiOrd397(void) { PMGPILOG("Ord397"); return 0L; }
#pragma aux PmgpiOrd397 "PMGPIORD397"
LONG APIENTRY PmgpiOrd398(void) { PMGPILOG("Ord398"); return 0L; }
#pragma aux PmgpiOrd398 "PMGPIORD398"
LONG APIENTRY PmgpiOrd400(void) { PMGPILOG("Ord400"); return 0L; }
#pragma aux PmgpiOrd400 "PMGPIORD400"
LONG APIENTRY PmgpiOrd401(void) { PMGPILOG("Ord401"); return 0L; }
#pragma aux PmgpiOrd401 "PMGPIORD401"
LONG APIENTRY PmgpiOrd402(void) { PMGPILOG("Ord402"); return 0L; }
#pragma aux PmgpiOrd402 "PMGPIORD402"
LONG APIENTRY PmgpiOrd403(void) { PMGPILOG("Ord403"); return 0L; }
#pragma aux PmgpiOrd403 "PMGPIORD403"
LONG APIENTRY PmgpiOrd404(void) { PMGPILOG("Ord404"); return 0L; }
#pragma aux PmgpiOrd404 "PMGPIORD404"
LONG APIENTRY PmgpiOrd405(void) { PMGPILOG("Ord405"); return 0L; }
#pragma aux PmgpiOrd405 "PMGPIORD405"
LONG APIENTRY PmgpiOrd406(void) { PMGPILOG("Ord406"); return 0L; }
#pragma aux PmgpiOrd406 "PMGPIORD406"
LONG APIENTRY PmgpiOrd407(void) { PMGPILOG("Ord407"); return 0L; }
#pragma aux PmgpiOrd407 "PMGPIORD407"
LONG APIENTRY PmgpiOrd408(void) { PMGPILOG("Ord408"); return 0L; }
#pragma aux PmgpiOrd408 "PMGPIORD408"
LONG APIENTRY PmgpiOrd409(void) { PMGPILOG("Ord409"); return 0L; }
#pragma aux PmgpiOrd409 "PMGPIORD409"
LONG APIENTRY PmgpiOrd411(void) { PMGPILOG("Ord411"); return 0L; }
#pragma aux PmgpiOrd411 "PMGPIORD411"
LONG APIENTRY PmgpiOrd412(void) { PMGPILOG("Ord412"); return 0L; }
#pragma aux PmgpiOrd412 "PMGPIORD412"
LONG APIENTRY PmgpiOrd413(void) { PMGPILOG("Ord413"); return 0L; }
#pragma aux PmgpiOrd413 "PMGPIORD413"
LONG APIENTRY PmgpiOrd414(void) { PMGPILOG("Ord414"); return 0L; }
#pragma aux PmgpiOrd414 "PMGPIORD414"
LONG APIENTRY PmgpiOrd415(void) { PMGPILOG("Ord415"); return 0L; }
#pragma aux PmgpiOrd415 "PMGPIORD415"
LONG APIENTRY PmgpiOrd416(void) { PMGPILOG("Ord416"); return 0L; }
#pragma aux PmgpiOrd416 "PMGPIORD416"
LONG APIENTRY PmgpiOrd417(void) { PMGPILOG("Ord417"); return 0L; }
#pragma aux PmgpiOrd417 "PMGPIORD417"
LONG APIENTRY PmgpiOrd418(void) { PMGPILOG("Ord418"); return 0L; }
#pragma aux PmgpiOrd418 "PMGPIORD418"
LONG APIENTRY PmgpiOrd419(void) { PMGPILOG("Ord419"); return 0L; }
#pragma aux PmgpiOrd419 "PMGPIORD419"
LONG APIENTRY PmgpiOrd420(void) { PMGPILOG("Ord420"); return 0L; }
#pragma aux PmgpiOrd420 "PMGPIORD420"
LONG APIENTRY PmgpiOrd421(void) { PMGPILOG("Ord421"); return 0L; }
#pragma aux PmgpiOrd421 "PMGPIORD421"
LONG APIENTRY PmgpiOrd422(void) { PMGPILOG("Ord422"); return 0L; }
#pragma aux PmgpiOrd422 "PMGPIORD422"
LONG APIENTRY PmgpiOrd423(void) { PMGPILOG("Ord423"); return 0L; }
#pragma aux PmgpiOrd423 "PMGPIORD423"
LONG APIENTRY PmgpiOrd424(void) { PMGPILOG("Ord424"); return 0L; }
#pragma aux PmgpiOrd424 "PMGPIORD424"
LONG APIENTRY PmgpiOrd425(void) { PMGPILOG("Ord425"); return 0L; }
#pragma aux PmgpiOrd425 "PMGPIORD425"
LONG APIENTRY PmgpiOrd426(void) { PMGPILOG("Ord426"); return 0L; }
#pragma aux PmgpiOrd426 "PMGPIORD426"
LONG APIENTRY PmgpiOrd427(void) { PMGPILOG("Ord427"); return 0L; }
#pragma aux PmgpiOrd427 "PMGPIORD427"
LONG APIENTRY PmgpiOrd428(void) { PMGPILOG("Ord428"); return 0L; }
#pragma aux PmgpiOrd428 "PMGPIORD428"
LONG APIENTRY PmgpiOrd429(void) { PMGPILOG("Ord429"); return 0L; }
#pragma aux PmgpiOrd429 "PMGPIORD429"
LONG APIENTRY PmgpiOrd430(void) { PMGPILOG("Ord430"); return 0L; }
#pragma aux PmgpiOrd430 "PMGPIORD430"
LONG APIENTRY PmgpiOrd431(void) { PMGPILOG("Ord431"); return 0L; }
#pragma aux PmgpiOrd431 "PMGPIORD431"
LONG APIENTRY PmgpiOrd432(void) { PMGPILOG("Ord432"); return 0L; }
#pragma aux PmgpiOrd432 "PMGPIORD432"
LONG APIENTRY PmgpiOrd433(void) { PMGPILOG("Ord433"); return 0L; }
#pragma aux PmgpiOrd433 "PMGPIORD433"
LONG APIENTRY PmgpiOrd434(void) { PMGPILOG("Ord434"); return 0L; }
#pragma aux PmgpiOrd434 "PMGPIORD434"
LONG APIENTRY PmgpiOrd435(void) { PMGPILOG("Ord435"); return 0L; }
#pragma aux PmgpiOrd435 "PMGPIORD435"
LONG APIENTRY PmgpiOrd436(void) { PMGPILOG("Ord436"); return 0L; }
#pragma aux PmgpiOrd436 "PMGPIORD436"
LONG APIENTRY PmgpiOrd437(void) { PMGPILOG("Ord437"); return 0L; }
#pragma aux PmgpiOrd437 "PMGPIORD437"
LONG APIENTRY PmgpiOrd438(void) { PMGPILOG("Ord438"); return 0L; }
#pragma aux PmgpiOrd438 "PMGPIORD438"
LONG APIENTRY PmgpiOrd439(void) { PMGPILOG("Ord439"); return 0L; }
#pragma aux PmgpiOrd439 "PMGPIORD439"
LONG APIENTRY PmgpiOrd440(void) { PMGPILOG("Ord440"); return 0L; }
#pragma aux PmgpiOrd440 "PMGPIORD440"
LONG APIENTRY PmgpiOrd441(void) { PMGPILOG("Ord441"); return 0L; }
#pragma aux PmgpiOrd441 "PMGPIORD441"
LONG APIENTRY PmgpiOrd442(void) { PMGPILOG("Ord442"); return 0L; }
#pragma aux PmgpiOrd442 "PMGPIORD442"
LONG APIENTRY PmgpiOrd443(void) { PMGPILOG("Ord443"); return 0L; }
#pragma aux PmgpiOrd443 "PMGPIORD443"
LONG APIENTRY PmgpiOrd444(void) { PMGPILOG("Ord444"); return 0L; }
#pragma aux PmgpiOrd444 "PMGPIORD444"
LONG APIENTRY PmgpiOrd445(void) { PMGPILOG("Ord445"); return 0L; }
#pragma aux PmgpiOrd445 "PMGPIORD445"
LONG APIENTRY PmgpiOrd446(void) { PMGPILOG("Ord446"); return 0L; }
#pragma aux PmgpiOrd446 "PMGPIORD446"
LONG APIENTRY PmgpiOrd447(void) { PMGPILOG("Ord447"); return 0L; }
#pragma aux PmgpiOrd447 "PMGPIORD447"
LONG APIENTRY PmgpiOrd448(void) { PMGPILOG("Ord448"); return 0L; }
#pragma aux PmgpiOrd448 "PMGPIORD448"
LONG APIENTRY PmgpiOrd449(void) { PMGPILOG("Ord449"); return 0L; }
#pragma aux PmgpiOrd449 "PMGPIORD449"
LONG APIENTRY PmgpiOrd450(void) { PMGPILOG("Ord450"); return 0L; }
#pragma aux PmgpiOrd450 "PMGPIORD450"
LONG APIENTRY PmgpiOrd451(void) { PMGPILOG("Ord451"); return 0L; }
#pragma aux PmgpiOrd451 "PMGPIORD451"
LONG APIENTRY PmgpiOrd452(void) { PMGPILOG("Ord452"); return 0L; }
#pragma aux PmgpiOrd452 "PMGPIORD452"
LONG APIENTRY PmgpiOrd453(void) { PMGPILOG("Ord453"); return 0L; }
#pragma aux PmgpiOrd453 "PMGPIORD453"
LONG APIENTRY PmgpiOrd454(void) { PMGPILOG("Ord454"); return 0L; }
#pragma aux PmgpiOrd454 "PMGPIORD454"
LONG APIENTRY PmgpiOrd455(void) { PMGPILOG("Ord455"); return 0L; }
#pragma aux PmgpiOrd455 "PMGPIORD455"
LONG APIENTRY PmgpiOrd456(void) { PMGPILOG("Ord456"); return 0L; }
#pragma aux PmgpiOrd456 "PMGPIORD456"
LONG APIENTRY PmgpiOrd457(void) { PMGPILOG("Ord457"); return 0L; }
#pragma aux PmgpiOrd457 "PMGPIORD457"
LONG APIENTRY PmgpiOrd458(void) { PMGPILOG("Ord458"); return 0L; }
#pragma aux PmgpiOrd458 "PMGPIORD458"
LONG APIENTRY PmgpiOrd459(void) { PMGPILOG("Ord459"); return 0L; }
#pragma aux PmgpiOrd459 "PMGPIORD459"
LONG APIENTRY PmgpiOrd460(void) { PMGPILOG("Ord460"); return 0L; }
#pragma aux PmgpiOrd460 "PMGPIORD460"
LONG APIENTRY PmgpiOrd461(void) { PMGPILOG("Ord461"); return 0L; }
#pragma aux PmgpiOrd461 "PMGPIORD461"
LONG APIENTRY PmgpiOrd462(void) { PMGPILOG("Ord462"); return 0L; }
#pragma aux PmgpiOrd462 "PMGPIORD462"
LONG APIENTRY PmgpiOrd463(void) { PMGPILOG("Ord463"); return 0L; }
#pragma aux PmgpiOrd463 "PMGPIORD463"
LONG APIENTRY PmgpiOrd464(void) { PMGPILOG("Ord464"); return 0L; }
#pragma aux PmgpiOrd464 "PMGPIORD464"
LONG APIENTRY PmgpiOrd465(void) { PMGPILOG("Ord465"); return 0L; }
#pragma aux PmgpiOrd465 "PMGPIORD465"
LONG APIENTRY PmgpiOrd466(void) { PMGPILOG("Ord466"); return 0L; }
#pragma aux PmgpiOrd466 "PMGPIORD466"
LONG APIENTRY PmgpiOrd467(void) { PMGPILOG("Ord467"); return 0L; }
#pragma aux PmgpiOrd467 "PMGPIORD467"
LONG APIENTRY PmgpiOrd468(void) { PMGPILOG("Ord468"); return 0L; }
#pragma aux PmgpiOrd468 "PMGPIORD468"
LONG APIENTRY PmgpiOrd469(void) { PMGPILOG("Ord469"); return 0L; }
#pragma aux PmgpiOrd469 "PMGPIORD469"
LONG APIENTRY PmgpiOrd470(void) { PMGPILOG("Ord470"); return 0L; }
#pragma aux PmgpiOrd470 "PMGPIORD470"
LONG APIENTRY PmgpiOrd471(void) { PMGPILOG("Ord471"); return 0L; }
#pragma aux PmgpiOrd471 "PMGPIORD471"
LONG APIENTRY PmgpiOrd472(void) { PMGPILOG("Ord472"); return 0L; }
#pragma aux PmgpiOrd472 "PMGPIORD472"
LONG APIENTRY PmgpiOrd473(void) { PMGPILOG("Ord473"); return 0L; }
#pragma aux PmgpiOrd473 "PMGPIORD473"
LONG APIENTRY PmgpiOrd474(void) { PMGPILOG("Ord474"); return 0L; }
#pragma aux PmgpiOrd474 "PMGPIORD474"
LONG APIENTRY PmgpiOrd475(void) { PMGPILOG("Ord475"); return 0L; }
#pragma aux PmgpiOrd475 "PMGPIORD475"
LONG APIENTRY PmgpiOrd476(void) { PMGPILOG("Ord476"); return 0L; }
#pragma aux PmgpiOrd476 "PMGPIORD476"
LONG APIENTRY PmgpiOrd477(void) { PMGPILOG("Ord477"); return 0L; }
#pragma aux PmgpiOrd477 "PMGPIORD477"
LONG APIENTRY PmgpiOrd478(void) { PMGPILOG("Ord478"); return 0L; }
#pragma aux PmgpiOrd478 "PMGPIORD478"
LONG APIENTRY PmgpiOrd479(void) { PMGPILOG("Ord479"); return 0L; }
#pragma aux PmgpiOrd479 "PMGPIORD479"
LONG APIENTRY PmgpiOrd480(void) { PMGPILOG("Ord480"); return 0L; }
#pragma aux PmgpiOrd480 "PMGPIORD480"
LONG APIENTRY PmgpiOrd481(void) { PMGPILOG("Ord481"); return 0L; }
#pragma aux PmgpiOrd481 "PMGPIORD481"
LONG APIENTRY PmgpiOrd482(void) { PMGPILOG("Ord482"); return 0L; }
#pragma aux PmgpiOrd482 "PMGPIORD482"
LONG APIENTRY PmgpiOrd483(void) { PMGPILOG("Ord483"); return 0L; }
#pragma aux PmgpiOrd483 "PMGPIORD483"
LONG APIENTRY PmgpiOrd484(void) { PMGPILOG("Ord484"); return 0L; }
#pragma aux PmgpiOrd484 "PMGPIORD484"
LONG APIENTRY PmgpiOrd485(void) { PMGPILOG("Ord485"); return 0L; }
#pragma aux PmgpiOrd485 "PMGPIORD485"
LONG APIENTRY PmgpiOrd486(void) { PMGPILOG("Ord486"); return 0L; }
#pragma aux PmgpiOrd486 "PMGPIORD486"
LONG APIENTRY PmgpiOrd487(void) { PMGPILOG("Ord487"); return 0L; }
#pragma aux PmgpiOrd487 "PMGPIORD487"
LONG APIENTRY PmgpiOrd488(void) { PMGPILOG("Ord488"); return 0L; }
#pragma aux PmgpiOrd488 "PMGPIORD488"
LONG APIENTRY PmgpiOrd489(void) { PMGPILOG("Ord489"); return 0L; }
#pragma aux PmgpiOrd489 "PMGPIORD489"
LONG APIENTRY PmgpiOrd490(void) { PMGPILOG("Ord490"); return 0L; }
#pragma aux PmgpiOrd490 "PMGPIORD490"
LONG APIENTRY PmgpiOrd491(void) { PMGPILOG("Ord491"); return 0L; }
#pragma aux PmgpiOrd491 "PMGPIORD491"
LONG APIENTRY PmgpiOrd492(void) { PMGPILOG("Ord492"); return 0L; }
#pragma aux PmgpiOrd492 "PMGPIORD492"
LONG APIENTRY PmgpiOrd494(void) { PMGPILOG("Ord494"); return 0L; }
#pragma aux PmgpiOrd494 "PMGPIORD494"
LONG APIENTRY PmgpiOrd495(void) { PMGPILOG("Ord495"); return 0L; }
#pragma aux PmgpiOrd495 "PMGPIORD495"
LONG APIENTRY PmgpiOrd496(void) { PMGPILOG("Ord496"); return 0L; }
#pragma aux PmgpiOrd496 "PMGPIORD496"
LONG APIENTRY PmgpiOrd497(void) { PMGPILOG("Ord497"); return 0L; }
#pragma aux PmgpiOrd497 "PMGPIORD497"
LONG APIENTRY PmgpiOrd498(void) { PMGPILOG("Ord498"); return 0L; }
#pragma aux PmgpiOrd498 "PMGPIORD498"
LONG APIENTRY PmgpiOrd499(void) { PMGPILOG("Ord499"); return 0L; }
#pragma aux PmgpiOrd499 "PMGPIORD499"
LONG APIENTRY PmgpiOrd500(void) { PMGPILOG("Ord500"); return 0L; }
#pragma aux PmgpiOrd500 "PMGPIORD500"
LONG APIENTRY PmgpiOrd501(void) { PMGPILOG("Ord501"); return 0L; }
#pragma aux PmgpiOrd501 "PMGPIORD501"
LONG APIENTRY PmgpiOrd502(void) { PMGPILOG("Ord502"); return 0L; }
#pragma aux PmgpiOrd502 "PMGPIORD502"
LONG APIENTRY PmgpiOrd503(void) { PMGPILOG("Ord503"); return 0L; }
#pragma aux PmgpiOrd503 "PMGPIORD503"
LONG APIENTRY PmgpiOrd504(void) { PMGPILOG("Ord504"); return 0L; }
#pragma aux PmgpiOrd504 "PMGPIORD504"
LONG APIENTRY PmgpiOrd505(void) { PMGPILOG("Ord505"); return 0L; }
#pragma aux PmgpiOrd505 "PMGPIORD505"
LONG APIENTRY PmgpiOrd506(void) { PMGPILOG("Ord506"); return 0L; }
#pragma aux PmgpiOrd506 "PMGPIORD506"
LONG APIENTRY PmgpiOrd507(void) { PMGPILOG("Ord507"); return 0L; }
#pragma aux PmgpiOrd507 "PMGPIORD507"
LONG APIENTRY PmgpiOrd508(void) { PMGPILOG("Ord508"); return 0L; }
#pragma aux PmgpiOrd508 "PMGPIORD508"
LONG APIENTRY PmgpiOrd509(void) { PMGPILOG("Ord509"); return 0L; }
#pragma aux PmgpiOrd509 "PMGPIORD509"
LONG APIENTRY PmgpiOrd510(void) { PMGPILOG("Ord510"); return 0L; }
#pragma aux PmgpiOrd510 "PMGPIORD510"
LONG APIENTRY PmgpiOrd511(void) { PMGPILOG("Ord511"); return 0L; }
#pragma aux PmgpiOrd511 "PMGPIORD511"
LONG APIENTRY PmgpiOrd512(void) { PMGPILOG("Ord512"); return 0L; }
#pragma aux PmgpiOrd512 "PMGPIORD512"
LONG APIENTRY PmgpiOrd513(void) { PMGPILOG("Ord513"); return 0L; }
#pragma aux PmgpiOrd513 "PMGPIORD513"
LONG APIENTRY PmgpiOrd514(void) { PMGPILOG("Ord514"); return 0L; }
#pragma aux PmgpiOrd514 "PMGPIORD514"
LONG APIENTRY PmgpiOrd515(void) { PMGPILOG("Ord515"); return 0L; }
#pragma aux PmgpiOrd515 "PMGPIORD515"
LONG APIENTRY PmgpiOrd516(void) { PMGPILOG("Ord516"); return 0L; }
#pragma aux PmgpiOrd516 "PMGPIORD516"
LONG APIENTRY PmgpiOrd517(void) { PMGPILOG("Ord517"); return 0L; }
#pragma aux PmgpiOrd517 "PMGPIORD517"
LONG APIENTRY PmgpiOrd518(void) { PMGPILOG("Ord518"); return 0L; }
#pragma aux PmgpiOrd518 "PMGPIORD518"
LONG APIENTRY PmgpiOrd519(void) { PMGPILOG("Ord519"); return 0L; }
#pragma aux PmgpiOrd519 "PMGPIORD519"
LONG APIENTRY PmgpiOrd520(void) { PMGPILOG("Ord520"); return 0L; }
#pragma aux PmgpiOrd520 "PMGPIORD520"
LONG APIENTRY PmgpiOrd521(void) { PMGPILOG("Ord521"); return 0L; }
#pragma aux PmgpiOrd521 "PMGPIORD521"
LONG APIENTRY PmgpiOrd522(void) { PMGPILOG("Ord522"); return 0L; }
#pragma aux PmgpiOrd522 "PMGPIORD522"
LONG APIENTRY PmgpiOrd523(void) { PMGPILOG("Ord523"); return 0L; }
#pragma aux PmgpiOrd523 "PMGPIORD523"
LONG APIENTRY PmgpiOrd524(void) { PMGPILOG("Ord524"); return 0L; }
#pragma aux PmgpiOrd524 "PMGPIORD524"
LONG APIENTRY PmgpiOrd525(void) { PMGPILOG("Ord525"); return 0L; }
#pragma aux PmgpiOrd525 "PMGPIORD525"
LONG APIENTRY PmgpiOrd526(void) { PMGPILOG("Ord526"); return 0L; }
#pragma aux PmgpiOrd526 "PMGPIORD526"
LONG APIENTRY PmgpiOrd527(void) { PMGPILOG("Ord527"); return 0L; }
#pragma aux PmgpiOrd527 "PMGPIORD527"
LONG APIENTRY PmgpiOrd528(void) { PMGPILOG("Ord528"); return 0L; }
#pragma aux PmgpiOrd528 "PMGPIORD528"
LONG APIENTRY PmgpiOrd529(void) { PMGPILOG("Ord529"); return 0L; }
#pragma aux PmgpiOrd529 "PMGPIORD529"
LONG APIENTRY PmgpiOrd530(void) { PMGPILOG("Ord530"); return 0L; }
#pragma aux PmgpiOrd530 "PMGPIORD530"
LONG APIENTRY PmgpiOrd531(void) { PMGPILOG("Ord531"); return 0L; }
#pragma aux PmgpiOrd531 "PMGPIORD531"
LONG APIENTRY PmgpiOrd532(void) { PMGPILOG("Ord532"); return 0L; }
#pragma aux PmgpiOrd532 "PMGPIORD532"
LONG APIENTRY PmgpiOrd533(void) { PMGPILOG("Ord533"); return 0L; }
#pragma aux PmgpiOrd533 "PMGPIORD533"
LONG APIENTRY PmgpiOrd534(void) { PMGPILOG("Ord534"); return 0L; }
#pragma aux PmgpiOrd534 "PMGPIORD534"
LONG APIENTRY PmgpiOrd535(void) { PMGPILOG("Ord535"); return 0L; }
#pragma aux PmgpiOrd535 "PMGPIORD535"
LONG APIENTRY PmgpiOrd536(void) { PMGPILOG("Ord536"); return 0L; }
#pragma aux PmgpiOrd536 "PMGPIORD536"
LONG APIENTRY PmgpiOrd537(void) { PMGPILOG("Ord537"); return 0L; }
#pragma aux PmgpiOrd537 "PMGPIORD537"
LONG APIENTRY PmgpiOrd538(void) { PMGPILOG("Ord538"); return 0L; }
#pragma aux PmgpiOrd538 "PMGPIORD538"
LONG APIENTRY PmgpiOrd539(void) { PMGPILOG("Ord539"); return 0L; }
#pragma aux PmgpiOrd539 "PMGPIORD539"
LONG APIENTRY PmgpiOrd540(void) { PMGPILOG("Ord540"); return 0L; }
#pragma aux PmgpiOrd540 "PMGPIORD540"
LONG APIENTRY PmgpiOrd541(void) { PMGPILOG("Ord541"); return 0L; }
#pragma aux PmgpiOrd541 "PMGPIORD541"
LONG APIENTRY PmgpiOrd542(void) { PMGPILOG("Ord542"); return 0L; }
#pragma aux PmgpiOrd542 "PMGPIORD542"
LONG APIENTRY PmgpiOrd543(void) { PMGPILOG("Ord543"); return 0L; }
#pragma aux PmgpiOrd543 "PMGPIORD543"
LONG APIENTRY PmgpiOrd544(void) { PMGPILOG("Ord544"); return 0L; }
#pragma aux PmgpiOrd544 "PMGPIORD544"
LONG APIENTRY PmgpiOrd545(void) { PMGPILOG("Ord545"); return 0L; }
#pragma aux PmgpiOrd545 "PMGPIORD545"
LONG APIENTRY PmgpiOrd546(void) { PMGPILOG("Ord546"); return 0L; }
#pragma aux PmgpiOrd546 "PMGPIORD546"
LONG APIENTRY PmgpiOrd547(void) { PMGPILOG("Ord547"); return 0L; }
#pragma aux PmgpiOrd547 "PMGPIORD547"
LONG APIENTRY PmgpiOrd548(void) { PMGPILOG("Ord548"); return 0L; }
#pragma aux PmgpiOrd548 "PMGPIORD548"
LONG APIENTRY PmgpiOrd549(void) { PMGPILOG("Ord549"); return 0L; }
#pragma aux PmgpiOrd549 "PMGPIORD549"
LONG APIENTRY PmgpiOrd550(void) { PMGPILOG("Ord550"); return 0L; }
#pragma aux PmgpiOrd550 "PMGPIORD550"
LONG APIENTRY PmgpiOrd551(void) { PMGPILOG("Ord551"); return 0L; }
#pragma aux PmgpiOrd551 "PMGPIORD551"
LONG APIENTRY PmgpiOrd552(void) { PMGPILOG("Ord552"); return 0L; }
#pragma aux PmgpiOrd552 "PMGPIORD552"
LONG APIENTRY PmgpiOrd553(void) { PMGPILOG("Ord553"); return 0L; }
#pragma aux PmgpiOrd553 "PMGPIORD553"
LONG APIENTRY PmgpiOrd554(void) { PMGPILOG("Ord554"); return 0L; }
#pragma aux PmgpiOrd554 "PMGPIORD554"
LONG APIENTRY PmgpiOrd555(void) { PMGPILOG("Ord555"); return 0L; }
#pragma aux PmgpiOrd555 "PMGPIORD555"
LONG APIENTRY PmgpiOrd557(void) { PMGPILOG("Ord557"); return 0L; }
#pragma aux PmgpiOrd557 "PMGPIORD557"
LONG APIENTRY PmgpiOrd558(void) { PMGPILOG("Ord558"); return 0L; }
#pragma aux PmgpiOrd558 "PMGPIORD558"
LONG APIENTRY PmgpiOrd559(void) { PMGPILOG("Ord559"); return 0L; }
#pragma aux PmgpiOrd559 "PMGPIORD559"
LONG APIENTRY PmgpiOrd560(void) { PMGPILOG("Ord560"); return 0L; }
#pragma aux PmgpiOrd560 "PMGPIORD560"
LONG APIENTRY PmgpiOrd561(void) { PMGPILOG("Ord561"); return 0L; }
#pragma aux PmgpiOrd561 "PMGPIORD561"
LONG APIENTRY PmgpiOrd562(void) { PMGPILOG("Ord562"); return 0L; }
#pragma aux PmgpiOrd562 "PMGPIORD562"
LONG APIENTRY PmgpiOrd563(void) { PMGPILOG("Ord563"); return 0L; }
#pragma aux PmgpiOrd563 "PMGPIORD563"
LONG APIENTRY PmgpiOrd564(void) { PMGPILOG("Ord564"); return 0L; }
#pragma aux PmgpiOrd564 "PMGPIORD564"
LONG APIENTRY PmgpiOrd565(void) { PMGPILOG("Ord565"); return 0L; }
#pragma aux PmgpiOrd565 "PMGPIORD565"
LONG APIENTRY PmgpiOrd566(void) { PMGPILOG("Ord566"); return 0L; }
#pragma aux PmgpiOrd566 "PMGPIORD566"
LONG APIENTRY PmgpiOrd567(void) { PMGPILOG("Ord567"); return 0L; }
#pragma aux PmgpiOrd567 "PMGPIORD567"
LONG APIENTRY PmgpiOrd568(void) { PMGPILOG("Ord568"); return 0L; }
#pragma aux PmgpiOrd568 "PMGPIORD568"
LONG APIENTRY PmgpiOrd569(void) { PMGPILOG("Ord569"); return 0L; }
#pragma aux PmgpiOrd569 "PMGPIORD569"
LONG APIENTRY PmgpiOrd570(void) { PMGPILOG("Ord570"); return 0L; }
#pragma aux PmgpiOrd570 "PMGPIORD570"
LONG APIENTRY PmgpiOrd571(void) { PMGPILOG("Ord571"); return 0L; }
#pragma aux PmgpiOrd571 "PMGPIORD571"
LONG APIENTRY PmgpiOrd572(void) { PMGPILOG("Ord572"); return 0L; }
#pragma aux PmgpiOrd572 "PMGPIORD572"
LONG APIENTRY PmgpiOrd573(void) { PMGPILOG("Ord573"); return 0L; }
#pragma aux PmgpiOrd573 "PMGPIORD573"
LONG APIENTRY PmgpiOrd574(void) { PMGPILOG("Ord574"); return 0L; }
#pragma aux PmgpiOrd574 "PMGPIORD574"
LONG APIENTRY PmgpiOrd575(void) { PMGPILOG("Ord575"); return 0L; }
#pragma aux PmgpiOrd575 "PMGPIORD575"
LONG APIENTRY PmgpiOrd576(void) { PMGPILOG("Ord576"); return 0L; }
#pragma aux PmgpiOrd576 "PMGPIORD576"
LONG APIENTRY PmgpiOrd577(void) { PMGPILOG("Ord577"); return 0L; }
#pragma aux PmgpiOrd577 "PMGPIORD577"
LONG APIENTRY PmgpiOrd578(void) { PMGPILOG("Ord578"); return 0L; }
#pragma aux PmgpiOrd578 "PMGPIORD578"
LONG APIENTRY PmgpiOrd579(void) { PMGPILOG("Ord579"); return 0L; }
#pragma aux PmgpiOrd579 "PMGPIORD579"
LONG APIENTRY PmgpiOrd580(void) { PMGPILOG("Ord580"); return 0L; }
#pragma aux PmgpiOrd580 "PMGPIORD580"
LONG APIENTRY PmgpiOrd581(void) { PMGPILOG("Ord581"); return 0L; }
#pragma aux PmgpiOrd581 "PMGPIORD581"
LONG APIENTRY PmgpiOrd582(void) { PMGPILOG("Ord582"); return 0L; }
#pragma aux PmgpiOrd582 "PMGPIORD582"
LONG APIENTRY PmgpiOrd583(void) { PMGPILOG("Ord583"); return 0L; }
#pragma aux PmgpiOrd583 "PMGPIORD583"
LONG APIENTRY PmgpiOrd584(void) { PMGPILOG("Ord584"); return 0L; }
#pragma aux PmgpiOrd584 "PMGPIORD584"
LONG APIENTRY PmgpiOrd585(void) { PMGPILOG("Ord585"); return 0L; }
#pragma aux PmgpiOrd585 "PMGPIORD585"
LONG APIENTRY PmgpiOrd586(void) { PMGPILOG("Ord586"); return 0L; }
#pragma aux PmgpiOrd586 "PMGPIORD586"
LONG APIENTRY PmgpiOrd587(void) { PMGPILOG("Ord587"); return 0L; }
#pragma aux PmgpiOrd587 "PMGPIORD587"
LONG APIENTRY PmgpiOrd588(void) { PMGPILOG("Ord588"); return 0L; }
#pragma aux PmgpiOrd588 "PMGPIORD588"
LONG APIENTRY PmgpiOrd589(void) { PMGPILOG("Ord589"); return 0L; }
#pragma aux PmgpiOrd589 "PMGPIORD589"
LONG APIENTRY PmgpiOrd590(void) { PMGPILOG("Ord590"); return 0L; }
#pragma aux PmgpiOrd590 "PMGPIORD590"
LONG APIENTRY PmgpiOrd591(void) { PMGPILOG("Ord591"); return 0L; }
#pragma aux PmgpiOrd591 "PMGPIORD591"
LONG APIENTRY PmgpiOrd592(void) { PMGPILOG("Ord592"); return 0L; }
#pragma aux PmgpiOrd592 "PMGPIORD592"
LONG APIENTRY PmgpiOrd593(void) { PMGPILOG("Ord593"); return 0L; }
#pragma aux PmgpiOrd593 "PMGPIORD593"
LONG APIENTRY PmgpiOrd594(void) { PMGPILOG("Ord594"); return 0L; }
#pragma aux PmgpiOrd594 "PMGPIORD594"
LONG APIENTRY PmgpiOrd595(void) { PMGPILOG("Ord595"); return 0L; }
#pragma aux PmgpiOrd595 "PMGPIORD595"
LONG APIENTRY PmgpiOrd596(void) { PMGPILOG("Ord596"); return 0L; }
#pragma aux PmgpiOrd596 "PMGPIORD596"
LONG APIENTRY PmgpiOrd597(void) { PMGPILOG("Ord597"); return 0L; }
#pragma aux PmgpiOrd597 "PMGPIORD597"
LONG APIENTRY PmgpiOrd598(void) { PMGPILOG("Ord598"); return 0L; }
#pragma aux PmgpiOrd598 "PMGPIORD598"
LONG APIENTRY PmgpiOrd599(void) { PMGPILOG("Ord599"); return 0L; }
#pragma aux PmgpiOrd599 "PMGPIORD599"
LONG APIENTRY PmgpiOrd601(void) { PMGPILOG("Ord601"); return 0L; }
#pragma aux PmgpiOrd601 "PMGPIORD601"
LONG APIENTRY PmgpiOrd602(void) { PMGPILOG("Ord602"); return 0L; }
#pragma aux PmgpiOrd602 "PMGPIORD602"
LONG APIENTRY PmgpiOrd603(void) { PMGPILOG("Ord603"); return 0L; }
#pragma aux PmgpiOrd603 "PMGPIORD603"
LONG APIENTRY PmgpiOrd604(void) { PMGPILOG("Ord604"); return 0L; }
#pragma aux PmgpiOrd604 "PMGPIORD604"
LONG APIENTRY PmgpiOrd605(void) { PMGPILOG("Ord605"); return 0L; }
#pragma aux PmgpiOrd605 "PMGPIORD605"
LONG APIENTRY PmgpiOrd606(void) { PMGPILOG("Ord606"); return 0L; }
#pragma aux PmgpiOrd606 "PMGPIORD606"
LONG APIENTRY PmgpiOrd607(void) { PMGPILOG("Ord607"); return 0L; }
#pragma aux PmgpiOrd607 "PMGPIORD607"
LONG APIENTRY PmgpiOrd608(void) { PMGPILOG("Ord608"); return 0L; }
#pragma aux PmgpiOrd608 "PMGPIORD608"
LONG APIENTRY PmgpiOrd609(void) { PMGPILOG("Ord609"); return 0L; }
#pragma aux PmgpiOrd609 "PMGPIORD609"
LONG APIENTRY PmgpiOrd610(void) { PMGPILOG("Ord610"); return 0L; }
#pragma aux PmgpiOrd610 "PMGPIORD610"
LONG APIENTRY PmgpiOrd611(void) { PMGPILOG("Ord611"); return 0L; }
#pragma aux PmgpiOrd611 "PMGPIORD611"
LONG APIENTRY PmgpiOrd612(void) { PMGPILOG("Ord612"); return 0L; }
#pragma aux PmgpiOrd612 "PMGPIORD612"
LONG APIENTRY PmgpiOrd613(void) { PMGPILOG("Ord613"); return 0L; }
#pragma aux PmgpiOrd613 "PMGPIORD613"
LONG APIENTRY PmgpiOrd614(void) { PMGPILOG("Ord614"); return 0L; }
#pragma aux PmgpiOrd614 "PMGPIORD614"
LONG APIENTRY PmgpiOrd615(void) { PMGPILOG("Ord615"); return 0L; }
#pragma aux PmgpiOrd615 "PMGPIORD615"
LONG APIENTRY PmgpiOrd616(void) { PMGPILOG("Ord616"); return 0L; }
#pragma aux PmgpiOrd616 "PMGPIORD616"
LONG APIENTRY PmgpiOrd617(void) { PMGPILOG("Ord617"); return 0L; }
#pragma aux PmgpiOrd617 "PMGPIORD617"
LONG APIENTRY PmgpiOrd618(void) { PMGPILOG("Ord618"); return 0L; }
#pragma aux PmgpiOrd618 "PMGPIORD618"
LONG APIENTRY PmgpiOrd619(void) { PMGPILOG("Ord619"); return 0L; }
#pragma aux PmgpiOrd619 "PMGPIORD619"
LONG APIENTRY PmgpiOrd620(void) { PMGPILOG("Ord620"); return 0L; }
#pragma aux PmgpiOrd620 "PMGPIORD620"
LONG APIENTRY PmgpiOrd621(void) { PMGPILOG("Ord621"); return 0L; }
#pragma aux PmgpiOrd621 "PMGPIORD621"
LONG APIENTRY PmgpiOrd622(void) { PMGPILOG("Ord622"); return 0L; }
#pragma aux PmgpiOrd622 "PMGPIORD622"
LONG APIENTRY PmgpiOrd623(void) { PMGPILOG("Ord623"); return 0L; }
#pragma aux PmgpiOrd623 "PMGPIORD623"
LONG APIENTRY PmgpiOrd624(void) { PMGPILOG("Ord624"); return 0L; }
#pragma aux PmgpiOrd624 "PMGPIORD624"
LONG APIENTRY PmgpiOrd625(void) { PMGPILOG("Ord625"); return 0L; }
#pragma aux PmgpiOrd625 "PMGPIORD625"
LONG APIENTRY PmgpiOrd626(void) { PMGPILOG("Ord626"); return 0L; }
#pragma aux PmgpiOrd626 "PMGPIORD626"
LONG APIENTRY PmgpiOrd627(void) { PMGPILOG("Ord627"); return 0L; }
#pragma aux PmgpiOrd627 "PMGPIORD627"
LONG APIENTRY PmgpiOrd628(void) { PMGPILOG("Ord628"); return 0L; }
#pragma aux PmgpiOrd628 "PMGPIORD628"
LONG APIENTRY PmgpiOrd629(void) { PMGPILOG("Ord629"); return 0L; }
#pragma aux PmgpiOrd629 "PMGPIORD629"
LONG APIENTRY PmgpiOrd630(void) { PMGPILOG("Ord630"); return 0L; }
#pragma aux PmgpiOrd630 "PMGPIORD630"
LONG APIENTRY PmgpiOrd631(void) { PMGPILOG("Ord631"); return 0L; }
#pragma aux PmgpiOrd631 "PMGPIORD631"
LONG APIENTRY PmgpiOrd632(void) { PMGPILOG("Ord632"); return 0L; }
#pragma aux PmgpiOrd632 "PMGPIORD632"
LONG APIENTRY PmgpiOrd633(void) { PMGPILOG("Ord633"); return 0L; }
#pragma aux PmgpiOrd633 "PMGPIORD633"
LONG APIENTRY PmgpiOrd634(void) { PMGPILOG("Ord634"); return 0L; }
#pragma aux PmgpiOrd634 "PMGPIORD634"
LONG APIENTRY PmgpiOrd635(void) { PMGPILOG("Ord635"); return 0L; }
#pragma aux PmgpiOrd635 "PMGPIORD635"
LONG APIENTRY PmgpiOrd636(void) { PMGPILOG("Ord636"); return 0L; }
#pragma aux PmgpiOrd636 "PMGPIORD636"
LONG APIENTRY PmgpiOrd637(void) { PMGPILOG("Ord637"); return 0L; }
#pragma aux PmgpiOrd637 "PMGPIORD637"
LONG APIENTRY PmgpiOrd638(void) { PMGPILOG("Ord638"); return 0L; }
#pragma aux PmgpiOrd638 "PMGPIORD638"
LONG APIENTRY PmgpiOrd639(void) { PMGPILOG("Ord639"); return 0L; }
#pragma aux PmgpiOrd639 "PMGPIORD639"
LONG APIENTRY PmgpiOrd640(void) { PMGPILOG("Ord640"); return 0L; }
#pragma aux PmgpiOrd640 "PMGPIORD640"
LONG APIENTRY PmgpiOrd641(void) { PMGPILOG("Ord641"); return 0L; }
#pragma aux PmgpiOrd641 "PMGPIORD641"
LONG APIENTRY PmgpiOrd642(void) { PMGPILOG("Ord642"); return 0L; }
#pragma aux PmgpiOrd642 "PMGPIORD642"
LONG APIENTRY PmgpiOrd643(void) { PMGPILOG("Ord643"); return 0L; }
#pragma aux PmgpiOrd643 "PMGPIORD643"
LONG APIENTRY PmgpiOrd644(void) { PMGPILOG("Ord644"); return 0L; }
#pragma aux PmgpiOrd644 "PMGPIORD644"
LONG APIENTRY PmgpiOrd645(void) { PMGPILOG("Ord645"); return 0L; }
#pragma aux PmgpiOrd645 "PMGPIORD645"
LONG APIENTRY PmgpiOrd646(void) { PMGPILOG("Ord646"); return 0L; }
#pragma aux PmgpiOrd646 "PMGPIORD646"
LONG APIENTRY PmgpiOrd648(void) { PMGPILOG("Ord648"); return 0L; }
#pragma aux PmgpiOrd648 "PMGPIORD648"
LONG APIENTRY PmgpiOrd649(void) { PMGPILOG("Ord649"); return 0L; }
#pragma aux PmgpiOrd649 "PMGPIORD649"
LONG APIENTRY PmgpiOrd650(void) { PMGPILOG("Ord650"); return 0L; }
#pragma aux PmgpiOrd650 "PMGPIORD650"
LONG APIENTRY PmgpiOrd651(void) { PMGPILOG("Ord651"); return 0L; }
#pragma aux PmgpiOrd651 "PMGPIORD651"
LONG APIENTRY PmgpiOrd652(void) { PMGPILOG("Ord652"); return 0L; }
#pragma aux PmgpiOrd652 "PMGPIORD652"
LONG APIENTRY PmgpiOrd653(void) { PMGPILOG("Ord653"); return 0L; }
#pragma aux PmgpiOrd653 "PMGPIORD653"
LONG APIENTRY PmgpiOrd654(void) { PMGPILOG("Ord654"); return 0L; }
#pragma aux PmgpiOrd654 "PMGPIORD654"
LONG APIENTRY PmgpiOrd655(void) { PMGPILOG("Ord655"); return 0L; }
#pragma aux PmgpiOrd655 "PMGPIORD655"
/* EXPERIMENT (2026-07-03): boot log shows Ord656 is the first/only PMGPI call,
 * invoked 4x, then PMDD traps with EAX=0, CR2=0x340 -- i.e. our 0 return was used
 * as a pointer and dereferenced at +0x340. So ord 656 must return a valid pointer
 * to a structure. Return a per-call zeroed scratch buffer (4 slots, since it is
 * called 4x) to get past the trap and reveal the next blocker. NOT the real
 * contract -- ord 656 needs proper reverse-engineering (Ghidra on OrigFiles). */
static char  g_ord656buf[4][2048];
static ULONG g_ord656idx = 0;
LONG APIENTRY PmgpiOrd656(void)
{
    char *p;
    PMGPILOG("Ord656");
    p = g_ord656buf[g_ord656idx & 3];
    g_ord656idx++;
    return (LONG)p;
}
#pragma aux PmgpiOrd656 "PMGPIORD656"
LONG APIENTRY PmgpiOrd657(void) { PMGPILOG("Ord657"); return 0L; }
#pragma aux PmgpiOrd657 "PMGPIORD657"
LONG APIENTRY PmgpiOrd658(void) { PMGPILOG("Ord658"); return 0L; }
#pragma aux PmgpiOrd658 "PMGPIORD658"
LONG APIENTRY PmgpiOrd659(void) { PMGPILOG("Ord659"); return 0L; }
#pragma aux PmgpiOrd659 "PMGPIORD659"
LONG APIENTRY PmgpiOrd660(void) { PMGPILOG("Ord660"); return 0L; }
#pragma aux PmgpiOrd660 "PMGPIORD660"
LONG APIENTRY PmgpiOrd661(void) { PMGPILOG("Ord661"); return 0L; }
#pragma aux PmgpiOrd661 "PMGPIORD661"
LONG APIENTRY PmgpiOrd662(void) { PMGPILOG("Ord662"); return 0L; }
#pragma aux PmgpiOrd662 "PMGPIORD662"
LONG APIENTRY PmgpiOrd663(void) { PMGPILOG("Ord663"); return 0L; }
#pragma aux PmgpiOrd663 "PMGPIORD663"
LONG APIENTRY PmgpiOrd664(void) { PMGPILOG("Ord664"); return 0L; }
#pragma aux PmgpiOrd664 "PMGPIORD664"
LONG APIENTRY PmgpiOrd665(void) { PMGPILOG("Ord665"); return 0L; }
#pragma aux PmgpiOrd665 "PMGPIORD665"
LONG APIENTRY PmgpiOrd666(void) { PMGPILOG("Ord666"); return 0L; }
#pragma aux PmgpiOrd666 "PMGPIORD666"
LONG APIENTRY PmgpiOrd667(void) { PMGPILOG("Ord667"); return 0L; }
#pragma aux PmgpiOrd667 "PMGPIORD667"
LONG APIENTRY PmgpiOrd668(void) { PMGPILOG("Ord668"); return 0L; }
#pragma aux PmgpiOrd668 "PMGPIORD668"
LONG APIENTRY PmgpiOrd669(void) { PMGPILOG("Ord669"); return 0L; }
#pragma aux PmgpiOrd669 "PMGPIORD669"
LONG APIENTRY PmgpiOrd700(void) { PMGPILOG("Ord700"); return 0L; }
#pragma aux PmgpiOrd700 "PMGPIORD700"
LONG APIENTRY PmgpiOrd701(void) { PMGPILOG("Ord701"); return 0L; }
#pragma aux PmgpiOrd701 "PMGPIORD701"
LONG APIENTRY PmgpiOrd703(void) { PMGPILOG("Ord703"); return 0L; }
#pragma aux PmgpiOrd703 "PMGPIORD703"
LONG APIENTRY PmgpiOrd704(void) { PMGPILOG("Ord704"); return 0L; }
#pragma aux PmgpiOrd704 "PMGPIORD704"
LONG APIENTRY PmgpiOrd705(void) { PMGPILOG("Ord705"); return 0L; }
#pragma aux PmgpiOrd705 "PMGPIORD705"
LONG APIENTRY PmgpiOrd706(void) { PMGPILOG("Ord706"); return 0L; }
#pragma aux PmgpiOrd706 "PMGPIORD706"
LONG APIENTRY PmgpiOrd707(void) { PMGPILOG("Ord707"); return 0L; }
#pragma aux PmgpiOrd707 "PMGPIORD707"
LONG APIENTRY PmgpiOrd710(void) { PMGPILOG("Ord710"); return 0L; }
#pragma aux PmgpiOrd710 "PMGPIORD710"
LONG APIENTRY PmgpiOrd712(void) { PMGPILOG("Ord712"); return 0L; }
#pragma aux PmgpiOrd712 "PMGPIORD712"
LONG APIENTRY PmgpiOrd713(void) { PMGPILOG("Ord713"); return 0L; }
#pragma aux PmgpiOrd713 "PMGPIORD713"
LONG APIENTRY PmgpiOrd714(void) { PMGPILOG("Ord714"); return 0L; }
#pragma aux PmgpiOrd714 "PMGPIORD714"
LONG APIENTRY PmgpiOrd715(void) { PMGPILOG("Ord715"); return 0L; }
#pragma aux PmgpiOrd715 "PMGPIORD715"
LONG APIENTRY PmgpiOrd716(void) { PMGPILOG("Ord716"); return 0L; }
#pragma aux PmgpiOrd716 "PMGPIORD716"
LONG APIENTRY PmgpiOrd717(void) { PMGPILOG("Ord717"); return 0L; }
#pragma aux PmgpiOrd717 "PMGPIORD717"
LONG APIENTRY PmgpiOrd718(void) { PMGPILOG("Ord718"); return 0L; }
#pragma aux PmgpiOrd718 "PMGPIORD718"
LONG APIENTRY PmgpiOrd719(void) { PMGPILOG("Ord719"); return 0L; }
#pragma aux PmgpiOrd719 "PMGPIORD719"
LONG APIENTRY PmgpiOrd720(void) { PMGPILOG("Ord720"); return 0L; }
#pragma aux PmgpiOrd720 "PMGPIORD720"
LONG APIENTRY PmgpiOrd721(void) { PMGPILOG("Ord721"); return 0L; }
#pragma aux PmgpiOrd721 "PMGPIORD721"
LONG APIENTRY PmgpiOrd722(void) { PMGPILOG("Ord722"); return 0L; }
#pragma aux PmgpiOrd722 "PMGPIORD722"
LONG APIENTRY PmgpiOrd723(void) { PMGPILOG("Ord723"); return 0L; }
#pragma aux PmgpiOrd723 "PMGPIORD723"
LONG APIENTRY PmgpiOrd724(void) { PMGPILOG("Ord724"); return 0L; }
#pragma aux PmgpiOrd724 "PMGPIORD724"
LONG APIENTRY PmgpiOrd725(void) { PMGPILOG("Ord725"); return 0L; }
#pragma aux PmgpiOrd725 "PMGPIORD725"
LONG APIENTRY PmgpiOrd726(void) { PMGPILOG("Ord726"); return 0L; }
#pragma aux PmgpiOrd726 "PMGPIORD726"
LONG APIENTRY PmgpiOrd727(void) { PMGPILOG("Ord727"); return 0L; }
#pragma aux PmgpiOrd727 "PMGPIORD727"
LONG APIENTRY PmgpiOrd728(void) { PMGPILOG("Ord728"); return 0L; }
#pragma aux PmgpiOrd728 "PMGPIORD728"
LONG APIENTRY PmgpiOrd730(void) { PMGPILOG("Ord730"); return 0L; }
#pragma aux PmgpiOrd730 "PMGPIORD730"
LONG APIENTRY PmgpiOrd399(void) { PMGPILOG("Ord399"); return 0L; }
#pragma aux PmgpiOrd399 "PMGPIORD399"
