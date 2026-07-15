/*
 * BVHWNDW.C  --  Base Video Handler for WiNDoWed VIO sessions (OS/2)
 *
 * Replacement for BVHWNDW.DLL (LX, 16-bit, 16797 bytes, IBM).
 *
 * IMPORTANT: 16-bit DLL. wcc (NOT wcc386):
 *   wcc -bt=OS2 -ml -2 -s -wx -zu -zp1 -fo=BVHWNDW.OBJ BVHWNDW.C
 *   wlink @BVHWNDW.LNK
 *
 * This implementation is a TRANSCRIPTION of the original's contract, recovered
 * by disassembly (see BVHWNDW_RE.md). BVHWNDW is NOT a DDK vector-table BVH:
 * DEVENABLE installs a 26-entry handler table (Fn 256..281) into the device
 * block at (*(deviceblock+4) + 0x400), and registers its Global/Local info-seg
 * selectors into the shared OS2CHAR.148 structure. All exports use the far
 * pascal 3-parameter form: fn(param1 far*, param2 far*, ULONG Function).
 */
#include <string.h>

#define FAR      __far
#define PASCAL   __pascal
#define EXPENTRY __far __pascal
#define APIENTRY __far __pascal

typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef unsigned short USHORT;
typedef unsigned short SEL;
typedef unsigned short BOOL;
typedef unsigned long  ULONG;
typedef short          SHORT;
typedef void FAR *     PVOID;

#define NO_ERROR                 0
#define ERROR_VIO_INVALID_PARMS  0x161

/* Watcom 16-bit far-pointer helpers */
#define MK_FP(s,o)   (((void FAR *)(((ULONG)(USHORT)(s) << 16) | (USHORT)(o))))
#define FP_SEG(p)    ((USHORT)((ULONG)(void FAR *)(p) >> 16))
#define FP_OFF(p)    ((USHORT)(ULONG)(void FAR *)(p))

/* A vector-table / handler entry: far pascal, 3 params, returns USHORT. */
typedef USHORT (EXPENTRY *PFN)(PVOID, PVOID, ULONG);

/* ---- Fn codes -------------------------------------------------- */
#define FnFillLogicalDevBlock   1
#define BVH_FIRST_FN            256
#define BVH_SLOTS               26          /* Fn 256..281 */

/* Diagnostic toggle: 0 = install the handler table only (skip the OS2CHAR.148
 * registration) to isolate the boot hang; 1 = full DEVENABLE. */
#ifndef BVHWNDW_REGISTER
#define BVHWNDW_REGISTER 0
#endif

/* ================================================================== */
/*  Imports                                                            */
/* ================================================================== */
/* DosGetInfoSeg -- fills the Global and Local info-segment selectors.
 * 16-bit DOSCALLS ordinal 8. */
extern void APIENTRY DOS16GETINFOSEG(SEL FAR *pGlobal, SEL FAR *pLocal);
#pragma aux DOS16GETINFOSEG "DOS16GETINFOSEG";

/* DosAllocSeg (DOSCALLS.34) -- allocate the logical video buffer. */
extern USHORT APIENTRY DOS16ALLOCSEG(USHORT cbSeg, SEL FAR *psel, USHORT fsSel);
#pragma aux DOS16ALLOCSEG "DOS16ALLOCSEG";

/* Diagnostic: create a marker FILE per handler. VirtualBox has no PC speaker, so
 * DosBeep is inaudible; instead each checkpoint creates C:\BVHxx.MRK. After a
 * windowed cmd.exe attempt, 'dir C:\BVH*.MRK' shows which handlers ran (delete
 * them first to distinguish boot-time from session-time). Set BVHWNDW_MARK 0 off. */
#ifndef BVHWNDW_MARK
#define BVHWNDW_MARK 1
#endif
#if BVHWNDW_MARK
extern USHORT APIENTRY DOS16OPEN(char FAR *, USHORT FAR *, USHORT FAR *, ULONG,
                                 USHORT, USHORT, USHORT, ULONG);
#pragma aux DOS16OPEN "DOS16OPEN";
extern USHORT APIENTRY DOS16CLOSE(USHORT);
#pragma aux DOS16CLOSE "DOS16CLOSE";
static void mark(char FAR *name)
{
    USHORT hf, act;
    /* create-if-new | replace-if-exists ; write-only, deny-none */
    if (DOS16OPEN(name, &hf, &act, 0L, 0, 0x12, 0x0041, 0L) == 0)
        DOS16CLOSE(hf);
}
#define MARK(s) mark(s)
#else
#define MARK(s) ((void)0)
#endif

/* PMMERGE.2036 -- called during device enable when the info seg advanced. */
extern USHORT APIENTRY PMMERGE_2036(USHORT a, USHORT b);
#pragma aux PMMERGE_2036 "PMMERGE_2036";

/* OS2CHAR.148 -- the shared data structure BVHWNDW registers itself into.
 * Imported as data; &Os2Char148 is its far address. */
extern UCHAR FAR Os2Char148;

/* PMVIOP windowed presentation-space lock/unlock. */
extern USHORT APIENTRY LockVioPS(PVOID);
extern USHORT APIENTRY UnlockVioPS(PVOID);

/* ================================================================== */
/*  Module globals (were data-seg offsets 2/4/6/0x16 in the original)  */
/* ================================================================== */
static SEL    gGIS = 0;      /* Global info-seg selector  (orig g[4]) */
static SEL    gLIS = 0;      /* Local  info-seg selector  (orig g[2]) */
static USHORT gVal6 = 0;     /* GIS[0x19] - 1             (orig g[6]) */
static int    gInited = 0;

/* forward decls of the 26 handlers */
static USHORT EXPENTRY BufferUpdate(PVOID p1, PVOID p2, ULONG fn);
static USHORT EXPENTRY InitEnv(PVOID p1, PVOID p2, ULONG fn);
static USHORT EXPENTRY GetConfig(PVOID p1, PVOID p2, ULONG fn);
static USHORT EXPENTRY GetMode(PVOID p1, PVOID p2, ULONG fn);
static USHORT EXPENTRY SetMode(PVOID p1, PVOID p2, ULONG fn);
static USHORT EXPENTRY RetOK(PVOID p1, PVOID p2, ULONG fn);      /* 0x04ec: return 0    */
static USHORT EXPENTRY RetNotInst(PVOID p1, PVOID p2, ULONG fn); /* 0x04e6: return 0x1ee */
static USHORT EXPENTRY Ret1A5(PVOID p1, PVOID p2, ULONG fn);     /* 0x052e: return 0x1a5 */
static USHORT EXPENTRY GenericOK(PVOID p1, PVOID p2, ULONG fn);  /* undecompiled -> 0    */

/* The 26-entry handler table, Fn 256..281 (from BVHWNDW_RE.md). Slots 21,22
 * (Fn 277,278) are null in the original and are skipped on install. The still-
 * undecompiled substantive handlers (261,264,265,274,275,276,279) use GenericOK
 * for now. */
static const PFN HandlerTable[BVH_SLOTS] = {
    BufferUpdate,   /* 256 BufferUpdate */
    InitEnv,        /* 257 InitEnv      */
    RetOK,          /* 258 SaveEnv      */
    RetOK,          /* 259 RestoreEnv   */
    GetConfig,      /* 260 RetConfigInfo*/
    GenericOK,      /* 261              */
    RetNotInst,     /* 262             */
    RetNotInst,     /* 263             */
    GenericOK,      /* 264             */
    GenericOK,      /* 265             */
    RetNotInst,     /* 266             */
    RetNotInst,     /* 267             */
    GetMode,        /* 268 GetMode     */
    SetMode,        /* 269 SetMode     */
    RetNotInst,     /* 270             */
    RetNotInst,     /* 271             */
    RetNotInst,     /* 272             */
    RetNotInst,     /* 273             */
    GenericOK,      /* 274             */
    GenericOK,      /* 275             */
    GenericOK,      /* 276             */
    0,              /* 277 (null)      */
    0,              /* 278 (null)      */
    GenericOK,      /* 279 GetLVBInfo  */
    Ret1A5,         /* 280             */
    Ret1A5          /* 281             */
};

/* ================================================================== */
/*  ord 2: DEVENABLE                                                    */
/* ================================================================== */
USHORT EXPENTRY DEVENABLE(PVOID pDev, PVOID pParm, ULONG Function)
{
    UCHAR FAR *pShared;
    PFN  FAR  *pVec;
    UCHAR FAR *pVecStruct;
    USHORT i;

    MARK("C:\\BVHDE.MRK");   /* DEVENABLE reached */
    if ((USHORT)Function != FnFillLogicalDevBlock)
        return 0xFFFF;
    if (pParm == 0 || *(USHORT FAR *)((UCHAR FAR *)pParm + 4) < 0x11a)
        return 0x1DF;

    if (!gInited) {
        /* install our 26 handlers into (*(pDev+4)) + 0x400, skipping nulls */
        pVecStruct = *(UCHAR FAR * FAR *)((UCHAR FAR *)pDev + 4);
        if (pVecStruct != 0) {
            pVec = (PFN FAR *)(pVecStruct + 0x400);
            for (i = 0; i < BVH_SLOTS; i++)
                if (HandlerTable[i] != 0)
                    pVec[i] = HandlerTable[i];
        }
        gInited = 1;
    }

#if BVHWNDW_REGISTER
    /* --- OS2CHAR.148 registration (isolated for diagnosis) --- */
    pShared = (UCHAR FAR *)&Os2Char148;
    *(USHORT FAR *)(pShared + 0x18) = FP_SEG(pDev);
    if (gGIS == 0) DOS16GETINFOSEG(&gGIS, &gLIS);
    *(SEL FAR *)(pShared + 0x126) = gGIS;
    *(SEL FAR *)(pShared + 0x16c) = gGIS;
    *(SEL FAR *)(pShared + 0x128) = gLIS;
    *(SEL FAR *)(pShared + 0x16e) = gLIS;
#else
    (void)pShared;
#endif
    return NO_ERROR;
}

/* ================================================================== */
/*  Fn 257: InitEnv -- initialise the environment for a new session.   */
/*  Transcribed from the original 0xc9 (text-mode path): copy the      */
/*  255-byte env template, set the mode fields, allocate + clear the    */
/*  logical video buffer (env[0xf7]). The advanced setup calls          */
/*  (OS2CHAR/codepage/internal helpers) are not yet transcribed.        */
/* ================================================================== */
/* Exact 255-byte env template (data 0x1e in the original). */
static const UCHAR EnvTemplate[255] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x00,0x01,0x04,0x50,0x00,0x19,0x00,
    0x80,0x02,0x90,0x01,0x00,0x01,0x00,0x80,0x0b,0x00,0xa0,0x0f,0x00,0x00,0xa0,0x0f,
    0x00,0x00,0xa0,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4f,0x00,
    0x18,0x00,0x01,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
/* PDDR Ch.14 (257): ParmBlock = {WORD Length=6; WORD Flags; WORD LVBselector}.
 * "This routine ALWAYS returns with AX = 0." We set up BVHWNDW's env buffer
 * (the template embeds a VIOMODEINFO at env[0x78]) and allocate a backing LVB. */
static USHORT EXPENTRY InitEnv(PVOID pEnv, PVOID pParm, ULONG fn)
{
    UCHAR FAR *e = (UCHAR FAR *)pEnv;
    UCHAR FAR *p = (UCHAR FAR *)pParm;
    UCHAR FAR *lvb;
    USHORT i, size;
    SEL sel;
    (void)fn;

    MARK("C:\\BVHIE.MRK");   /* InitEnv reached */
    for (i = 0; i < 255; i++)                 /* copy the env template */
        e[i] = EnvTemplate[i];

    *(USHORT FAR *)(e + 0x14) = *(USHORT FAR *)(p + 4);  /* passed LVB selector */
    *(USHORT FAR *)(e + 0x1a) = 2;
    *(ULONG  FAR *)(e + 0x1c) = 0x00500019UL; /* 25 rows / 80 cols */
    *(ULONG  FAR *)(e + 0x28) = 0x00070006UL;
    *(USHORT FAR *)(e + 0x68) = 0;
    e[0x85] = 1;
    *(USHORT FAR *)(e + 0xf9) = 0;

    /* allocate BVHWNDW's backing logical video buffer (80*25*2 = 4000 bytes) */
    size = 80 * 25 * 2;
    *(USHORT FAR *)(e + 0xf5) = size;
    *(USHORT FAR *)(e + 0xf7) = 0;
    if (DOS16ALLOCSEG(size, (SEL FAR *)(e + 0xf7), 2) == 0) {
        sel = *(SEL FAR *)(e + 0xf7);
        lvb = (UCHAR FAR *)MK_FP(sel, 0);
        for (i = 0; i < size; i++)
            lvb[i] = 0;
    }
    MARK("C:\\BVHIE2.MRK");
    return NO_ERROR;                          /* spec: ALWAYS return 0 */
}

/* ================================================================== */
/*  Fn 260 / ord 3: GETCONFIG (RetConfigInfo)                          */
/* ================================================================== */
/* Exact 40-byte config table (data 0x11d in the original). */
static const UCHAR ConfigTable[40] = {
    0x28,0x00, 0xff,0xff, 0xff,0xff, 0x00,0x80,0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0xff,0x00,0x00,0x00,
    0x00,0x80,0x00,0x00, 0x00,0x80,0x00,0x00, 0x20,0x00,
    0x24,0x00, 0x01,0x00, 0xff,0xff, 0x01,0x00, 0xff,0xff
};
static USHORT EXPENTRY GetConfig(PVOID pEnv, PVOID pParm, ULONG fn)
{
    UCHAR FAR *pOut;
    USHORT cb, i;
    (void)pEnv; (void)fn;
    if (pParm == 0 || *(USHORT FAR *)pParm != 8)
        return ERROR_VIO_INVALID_PARMS;
    pOut = *(UCHAR FAR * FAR *)((UCHAR FAR *)pParm + 4);
    if (pOut == 0) return ERROR_VIO_INVALID_PARMS;
    cb = *(USHORT FAR *)pOut;
    if (cb <= 1) return ERROR_VIO_INVALID_PARMS;
    if (cb > sizeof(ConfigTable)) cb = sizeof(ConfigTable);
    for (i = 0; i < cb; i++) pOut[i] = ConfigTable[i];
    return NO_ERROR;
}
/* GETCONFIG is also an export (ord 3) -> same code. */
USHORT EXPENTRY GETCONFIG(PVOID p1, PVOID p2, ULONG fn) { return GetConfig(p1, p2, fn); }

/* ================================================================== */
/*  ord 1 / Fn 256: BUFFERUPDATE -- render VIO cells into the window.  */
/*  (First cut; full cell rendering still to transcribe.)              */
/* ================================================================== */
static USHORT EXPENTRY BufferUpdate(PVOID pvps, PVOID pRect, ULONG fn)
{
    (void)pRect; (void)fn;
    MARK("C:\\BVHBU.MRK");   /* BufferUpdate reached (session is rendering) */
    if (pvps == 0) return NO_ERROR;
    if (LockVioPS(pvps) != NO_ERROR) return NO_ERROR;
    /* TODO: draw the changed LVB cells to the console DC via PMGPI. */
    UnlockVioPS(pvps);
    return NO_ERROR;
}
/* BUFFERUPDATE export (ord 1). */
USHORT EXPENTRY BUFFERUPDATE(PVOID p1, PVOID p2, ULONG fn) { return BufferUpdate(p1, p2, fn); }

/* ================================================================== */
/*  Mode / stub handlers                                               */
/* ================================================================== */
/* PDDR Ch.14 (268) Query Mode: ParmBlock = {WORD Length; WORD Flags; DWORD far
 * addr of a VioGetMode (VIOMODEINFO)}. Copy the env's VIOMODEINFO (env[0x78])
 * into the caller's buffer up to its cb. */
static USHORT EXPENTRY GetMode(PVOID pEnv, PVOID pParm, ULONG fn)
{
    UCHAR FAR *e = (UCHAR FAR *)pEnv;
    UCHAR FAR *p = (UCHAR FAR *)pParm;
    UCHAR FAR *mode;
    USHORT cb, i;
    (void)fn;
    MARK("C:\\BVHGM.MRK");
    if (p == 0) return NO_ERROR;
    mode = *(UCHAR FAR * FAR *)(p + 4);        /* DWORD far addr of mode data */
    if (mode == 0) return NO_ERROR;
    cb = *(USHORT FAR *)mode;                   /* caller's buffer size */
    if (cb > 34) cb = 34;                       /* VIOMODEINFO is 34 bytes */
    for (i = 0; i < cb; i++)
        mode[i] = e[0x78 + i];                  /* env-embedded VIOMODEINFO */
    return NO_ERROR;
}
/* PDDR Ch.14 (269) Set Mode: implicitly inits the env; returns 0 (mode ok). */
static USHORT EXPENTRY SetMode(PVOID p1, PVOID p2, ULONG fn)
{ (void)p1; (void)p2; (void)fn; MARK("C:\\BVHSM.MRK"); return NO_ERROR; }
static USHORT EXPENTRY RetOK(PVOID p1, PVOID p2, ULONG fn)
{ (void)p1; (void)p2; (void)fn; return 0; }
static USHORT EXPENTRY RetNotInst(PVOID p1, PVOID p2, ULONG fn)
{ (void)p1; (void)p2; (void)fn; return 0x1EE; }
static USHORT EXPENTRY Ret1A5(PVOID p1, PVOID p2, ULONG fn)
{ (void)p1; (void)p2; (void)fn; return 0x1A5; }
static USHORT EXPENTRY GenericOK(PVOID p1, PVOID p2, ULONG fn)
{ (void)p1; (void)p2; (void)fn; MARK("C:\\BVHGN.MRK"); return NO_ERROR; }

/* ================================================================== */
/*  16-bit DLL init entry                                              */
/* ================================================================== */
unsigned _DLL_InitTerm(unsigned hmod, unsigned fFlag);
#pragma aux _DLL_InitTerm "_DLL_InitTerm";
unsigned _DLL_InitTerm(unsigned hmod, unsigned fFlag)
{ (void)hmod; (void)fFlag; return 1; }
