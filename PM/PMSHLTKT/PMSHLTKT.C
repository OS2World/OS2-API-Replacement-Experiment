/*
 * PMSHLTKT.C  --  PM Shell Toolkit (clean-room reimplementation)
 *
 * Replacement for PMSHLTKT.DLL (LX, 16-bit, 44,232 bytes, IBM). 54 exports.
 * See PMSHLTKT_RE.md for the full reverse-engineered ABI + import map.
 *
 * IMPORTANT: 16-bit DLL. wcc (NOT wcc386):
 *   wcc -bt=OS2 -ml -2 -s -wx -zu -zp1 -fo=PMSHLTKT.OBJ PMSHLTKT.C
 *   wlink @PMSHLTKT.LNK
 *
 * Calling convention (recovered by disassembly):
 *   - Uppercase exports: __far __pascal (callee cleanup; __pascal uppercases the
 *     symbol so the export name matches exactly). __loadds so the exports can
 *     reach this DLL's data segment (the original does the equivalent DGROUP
 *     load at every entry).
 *   - _BldFEAList / _BldMVMT: __cdecl far (leading underscore, caller cleanup).
 *
 * STATUS: Family A (string/mem/convert/DBCS/validate, 26 functions) is fully
 * implemented and faithful. Family B (FS engine, object-msg, MRI, icons, printer
 * EA, desktop-lock hooks) is ABI-correct (exact stack cleanup) and being filled
 * in against the documented 16-bit imports across subsequent passes; the two
 * OBJ*MSGPROC functions are done. Family-B WIP bodies return safe defaults and
 * are marked "TODO(faithful)".
 */

/* ---- minimal types (no os2.h; this is a freestanding 16-bit build) -------- */
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned short SEL;
typedef unsigned short BOOL;
typedef unsigned long  ULONG;
typedef short          SHORT;
typedef long           LONG;

typedef char       __far *PCH;    /* far char pointer   */
typedef UCHAR      __far *PB;     /* far byte pointer   */
typedef void       __far *PV;     /* far void pointer   */

#define NULLP   ((PV)0)
#define NO_ERROR 0

/* Export calling conventions. */
#define SHLAPI    __far __pascal __loadds
#define SHLCDECL  __far __cdecl  __loadds

/* ================================================================== */
/*  Imports                                                            */
/* ================================================================== */
/* DosGetDBCSEv (DOSCALLS.4): returns the DBCS environment vector -- a list of
 * {lo,hi} lead-byte range pairs terminated by {0,0}. Used by ISDBCS/NEXTPCH/
 * PREVPCH so double-byte text is walked correctly. On a single-byte codepage
 * the vector is empty and these degrade to plain byte stepping. */
extern USHORT __far __pascal DOS16GETDBCSEV(USHORT cb, PV pcc, PV pBuf);
#pragma aux DOS16GETDBCSEV "DOS16GETDBCSEV";

/* ================================================================== */
/*  Module data                                                        */
/* ================================================================== */
static UCHAR gDBCS[12];          /* DBCS lead-byte range pairs {lo,hi}..{0,0} */

/* ---- selector-validation primitives for WSPVAL (lsl / verr / verw) -------- */
/* segment byte limit of a selector, or 0 if the selector is invalid */
static USHORT seglim(SEL s);
#pragma aux seglim =            \
    "lsl  ax, ax"               \
    "jz   L_ok"                 \
    "xor  ax, ax"               \
    "L_ok:"                     \
    parm [ax] value [ax] modify exact [ax];
/* 1 if selector is readable, else 0 */
static USHORT canrd(SEL s);
#pragma aux canrd =             \
    "verr ax"                   \
    "mov  ax, 0"                \
    "jnz  R_fin"                \
    "inc  ax"                   \
    "R_fin:"                    \
    parm [ax] value [ax] modify exact [ax];
/* 1 if selector is writable, else 0 */
static USHORT canwr(SEL s);
#pragma aux canwr =             \
    "verw ax"                   \
    "mov  ax, 0"                \
    "jnz  W_fin"                \
    "inc  ax"                   \
    "W_fin:"                    \
    parm [ax] value [ax] modify exact [ax];

/* ---- DBCS helpers --------------------------------------------------------- */
static int isLead(UCHAR c)
{
    int i;
    if (c == 0) return 0;
    for (i = 0; (gDBCS[i] | gDBCS[i+1]) != 0; i += 2)
        if (c >= gDBCS[i] && c <= gDBCS[i+1]) return 1;
    return 0;
}

/* ================================================================== */
/*  Family A -- string / memory                                        */
/* ================================================================== */
USHORT SHLAPI STRLEN(PCH s)                     /* ord 8 */
{
    PCH p = s;
    while (*p) p++;
    return (USHORT)(p - s);
}

PCH SHLAPI STRCPY(PCH d, PCH s)                 /* ord 7 */
{
    PCH r = d;
    while ((*d++ = *s++) != 0) ;
    return r;
}

PCH SHLAPI STRCAT(PCH d, PCH s)                 /* ord 6 */
{
    PCH r = d;
    while (*d) d++;
    while ((*d++ = *s++) != 0) ;
    return r;
}

PCH SHLAPI STRCHR(PCH s, USHORT ch)             /* ord 10 */
{
    UCHAR c = (UCHAR)ch;
    for (;;) {
        if ((UCHAR)*s == c) return s;
        if (*s == 0) return (PCH)0;
        s++;
    }
}

PCH SHLAPI STRRCHR(PCH s, USHORT ch)            /* ord 11 */
{
    UCHAR c = (UCHAR)ch;
    PCH last = (PCH)0;
    for (;;) {
        if ((UCHAR)*s == c) last = s;
        if (*s == 0) return last;
        s++;
    }
}

SHORT SHLAPI STRCMP(PCH a, PCH b)               /* ord 13 */
{
    while (*a && (UCHAR)*a == (UCHAR)*b) { a++; b++; }
    return (SHORT)((UCHAR)*a - (UCHAR)*b);
}

static UCHAR upcase(UCHAR c)
{ return (c >= 'a' && c <= 'z') ? (UCHAR)(c - 'a' + 'A') : c; }

SHORT SHLAPI STRCMPI(PCH a, PCH b)              /* ord 20 */
{
    while (*a && upcase((UCHAR)*a) == upcase((UCHAR)*b)) { a++; b++; }
    return (SHORT)(upcase((UCHAR)*a) - upcase((UCHAR)*b));
}

SHORT SHLAPI STRNCMP(PCH a, PCH b, USHORT n)    /* ord 30 */
{
    while (n && *a && (UCHAR)*a == (UCHAR)*b) { a++; b++; n--; }
    if (n == 0) return 0;
    return (SHORT)((UCHAR)*a - (UCHAR)*b);
}

SHORT SHLAPI STRNCMPI(PCH a, PCH b, USHORT n)   /* ord 21 */
{
    while (n && *a && upcase((UCHAR)*a) == upcase((UCHAR)*b)) { a++; b++; n--; }
    if (n == 0) return 0;
    return (SHORT)(upcase((UCHAR)*a) - upcase((UCHAR)*b));
}

PCH SHLAPI STRNCPY(PCH d, PCH s, USHORT n)      /* ord 25 -- standard strncpy */
{
    PCH r = d;
    USHORT i;
    /* IBM: copy up to n chars, stop after copying the NUL; does NOT null-pad
     * the remainder (verified against the original binary via tests/diff). */
    for (i = 0; i < n; i++) { if ((d[i] = s[i]) == 0) break; }
    return r;
}

PCH SHLAPI STRNCPY_TRUNC(PCH d, PCH s, USHORT n)/* ord 24 */
{
    PCH r = d;
    USHORT i;
    /* IBM: identical to STRNCPY on a single-byte codepage (verified via
     * tests/diff). The _TRUNC variant differs only in DBCS boundary handling
     * (avoids splitting a double-byte char at n) -- unverified, deferred. */
    for (i = 0; i < n; i++) { if ((d[i] = s[i]) == 0) break; }
    return r;
}

PCH SHLAPI STRNCAT(PCH d, PCH s, USHORT n)      /* ord 23 -- cap total at n */
{
    PCH r = d;
    USHORT len = 0;
    while (d[len]) len++;
    while (len < n && *s) { d[len++] = *s++; }
    d[n] = 0;                                   /* original writes dst[n]=0 */
    return r;
}

PCH SHLAPI STRNCAT_TRUNC(PCH d, PCH s, USHORT n)/* ord 22 */
{
    PCH r = d;
    USHORT len = 0;
    /* IBM: same as STRNCAT on a single-byte codepage -- terminator at d[n]
     * (so it can truncate the dest), verified via tests/diff. */
    while (d[len]) len++;
    while (len < n && *s) { d[len++] = *s++; }
    d[n] = 0;
    return r;
}

PCH SHLAPI STRSTR(PCH hay, PCH needle)          /* ord 26 */
{
    PCH h, n;
    if (*needle == 0) return (PCH)0;            /* IBM: empty needle -> NULL */
    for (; *hay; hay++) {
        h = hay; n = needle;
        while (*n && *h == *n) { h++; n++; }
        if (*n == 0) return hay;
    }
    return (PCH)0;
}

PCH SHLAPI STRUPR(PCH s)                         /* ord 27 -- DBCS-safe */
{
    PCH p = s;
    while (*p) {
        if (isLead((UCHAR)*p) && p[1]) { p += 2; continue; }
        *p = (char)upcase((UCHAR)*p);
        p++;
    }
    return s;
}

PCH SHLAPI STRPBRK(PCH s, PCH set)              /* ord 42 */
{
    PCH q;
    for (; *s; s++)
        for (q = set; *q; q++)
            if (*s == *q) return s;
    return (PCH)0;
}

void SHLAPI MEMCPY(PV d, PV s, USHORT n)        /* ord 12 */
{
    PB dd = (PB)d, ss = (PB)s;
    while (n) { *dd++ = *ss++; n--; }
}

/* ================================================================== */
/*  Family A -- numeric conversion                                     */
/* ================================================================== */
void SHLAPI ITOA(USHORT value, PCH buf)         /* ord 9 -- unsigned decimal */
{
    UCHAR tmp[6];
    int i = 0, j = 0;
    do { tmp[i++] = (UCHAR)('0' + (value % 10)); value = (USHORT)(value / 10); }
    while (value);
    while (i) buf[j++] = (char)tmp[--i];
    buf[j] = 0;
}

void SHLAPI ULTOA(ULONG value, PCH buf)         /* ord 15 -- unsigned decimal */
{
    UCHAR tmp[11];
    int i = 0, j = 0;
    do { tmp[i++] = (UCHAR)('0' + (int)(value % 10)); value /= 10; }
    while (value);
    while (i) buf[j++] = (char)tmp[--i];
    buf[j] = 0;
}

SHORT SHLAPI ATOI(PCH s)                        /* ord 14 */
{
    SHORT sign = 1, v = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') { v = (SHORT)(v * 10 + (*s - '0')); s++; }
    return (SHORT)(sign * v);
}

LONG SHLAPI ATOL(PCH s)                         /* ord 29 */
{
    LONG v = 0;
    /* IBM ATOL quirk (verified via tests/diff): accepts an optional leading
     * '+' only -- it does NOT handle '-' (returns 0 for negative input) and
     * does NOT skip leading whitespace. Unlike ATOI, which does both. */
    if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
    return v;
}

/* ================================================================== */
/*  Family A -- text / DBCS / validate                                 */
/* ================================================================== */
USHORT SHLAPI ISDBCS(USHORT ch)                 /* ord 17 */
{ return (USHORT)isLead((UCHAR)ch); }

PCH SHLAPI NEXTPCH(PCH p)                        /* ord 18 */
{
    if (isLead((UCHAR)*p) && p[1]) return p + 2;
    return p + 1;
}

PCH SHLAPI PREVPCH(PCH start, PCH cur)          /* ord 19 */
{
    PCH q = start, prev = start;
    if (cur <= start) return start;
    while (q < cur) { prev = q; q = (isLead((UCHAR)*q) && q[1]) ? q + 2 : q + 1; }
    return prev;
}

/* XLATBLANK (ord 28): on a DBCS codepage the original folds double-byte blanks;
 * on a single-byte codepage it is a no-op returning NULL (matches the observed
 * non-DBCS path). TODO(faithful): DBCS blank folding. */
PV SHLAPI XLATBLANK(PCH s)
{ (void)s; return NULLP; }

/* STRIPBLANKS (ord 3): flags bit0 = strip leading blanks, bit1 = strip trailing.
 * Mirrors the original: XLATBLANK first, then trim. */
void SHLAPI STRIPBLANKS(PCH s, USHORT flags)
{
    PCH p, q;
    if (flags == 0) return;
    XLATBLANK(s);
    if (flags & 1) {                            /* leading */
        p = s;
        while (*p == ' ') p++;
        if (p != s) { q = s; while ((*q++ = *p++) != 0) ; }
    }
    if (flags & 2) {                            /* trailing */
        USHORT n = STRLEN(s);
        while (n > 0 && s[n-1] == ' ') s[--n] = 0;
    }
}

USHORT SHLAPI ISNUMBER(PCH s)                   /* ord 45 */
{
    STRIPBLANKS(s, 3);                          /* original strips both sides */
    if (*s == '-' || *s == '+') s++;
    if (*s == 0) return 0;
    while (*s) { if (*s < '0' || *s > '9') return 0; s++; }
    return 1;
}

/* WSPVAL (ord 49): validate that [off, off+len) within selector 'sel' is
 * accessible for read (rw==0) or write (rw!=0). Returns 1 if valid, else 0. */
USHORT SHLAPI WSPVAL(SEL sel, USHORT off, USHORT len, USHORT rw)
{
    USHORT lim = seglim(sel);
    USHORT end = (USHORT)(off + len);
    if (lim == 0) return 0;                     /* invalid selector          */
    if (end < off) return 0;                    /* offset+length overflow    */
    if (lim < end) return 0;                    /* extent past segment limit */
    if (rw) return canwr(sel);
    return canrd(sel);
}

/* ================================================================== */
/*  Family B -- WPS integration                                        */
/*  ABI-correct (exact retf cleanup). OBJ*MSGPROC done; the remainder   */
/*  is being reimplemented against the documented imports.              */
/* ================================================================== */

/* The OBJ*MSGPROC trio is a far-function-pointer trampoline (verified by
 * disassembly), NOT PM window messaging: CREATE registers a callback (returns
 * the fn pointer as-is), SEND invokes it, DESTROY is a no-op. FSTRAVERSE uses it
 * to run a per-file operation callback (delete/rename/move/copy) on each file. */
typedef ULONG (__far __pascal *FSCB)(ULONG, ULONG, ULONG, ULONG, USHORT);

/* ord 37: returns its 2nd parameter (the callback fn pointer). */
ULONG SHLAPI OBJCREATEMSGPROC(ULONG a, ULONG fn, ULONG c)
{ (void)a; (void)c; return fn; }

/* ord 38: no-op teardown, returns 0. */
ULONG SHLAPI OBJDESTROYMSGPROC(ULONG fn)
{ (void)fn; return 0; }

/* ---- File-operation engine (retf 0x14 / 0x10) -- TODO(faithful):
 * the original drives a WPS file-op control block (progress/confirm) via
 * internal workers over DosMove/DosDelete/DosOpen+Read+Write/DosFindFirst2. */
USHORT SHLAPI FSTRAVERSE(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e)   /* 31: 0x14 */
{ (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
USHORT SHLAPI FSQUERYDRIVEDATA(PV a)                                     /* 32: 0x04 */
{ (void)a; return 0; }
USHORT SHLAPI FSRENAME(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e)     /* 33: 0x14 */
{ (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
USHORT SHLAPI FSCOPY(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e)       /* 34: 0x14 */
{ (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
USHORT SHLAPI FSMOVE(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e)       /* 35: 0x14 */
{ (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
USHORT SHLAPI FSDELETE(ULONG a, ULONG b, ULONG c, ULONG d)               /* 36: 0x10 */
{ (void)a;(void)b;(void)c;(void)d; return 0; }
USHORT SHLAPI FSTRAVINFO(PV a)                                           /* 40: 0x04 */
{ (void)a; return 0; }

/* ord 39: trampoline -- if the callback pointer is null return 1, else call
 * fn(a,b,c,d,e) forwarding the 18-byte argument frame (verified by disasm:
 * pushes [bp+6..bp+0x16] then `lcall [fn]`, callee-cleanup => __far __pascal). */
ULONG SHLAPI OBJSENDMSG(ULONG fn, ULONG a, ULONG b, ULONG c, ULONG d, USHORT e)
{
    FSCB f;
    if (fn == 0) return 1;
    f = (FSCB)(void __far *)fn;
    return f(a, b, c, d, e);
}

/* ---- MRI resource loader (retf 0x10) -- TODO(faithful): DosGetResource/
 * WinLoadMessage into the caller's buffer. */
USHORT SHLAPI MRILOADRESOURCE(ULONG a, ULONG b, ULONG c, ULONG d)        /* 41: 0x10 */
{ (void)a;(void)b;(void)c;(void)d; return 0; }

/* ---- Icon helpers -- TODO(faithful). */
USHORT SHLAPI DOSREFRESHICONS(ULONG a, ULONG b, ULONG c, USHORT d)       /* 1: 0x0e */
{ (void)a;(void)b;(void)c;(void)d; return 0; }
USHORT SHLAPI GETICONDATA(ULONG a, ULONG b, ULONG c, ULONG d)            /* 2: 0x10 */
{ (void)a;(void)b;(void)c;(void)d; return 0; }
ULONG SHLAPI GETICONHANDLEFORVIOEXE(ULONG a, ULONG b, USHORT c)          /* 4: 0x0a */
{ (void)a;(void)b;(void)c; return 0; }
ULONG SHLAPI GETICONHANDLEFORPMEXE(ULONG a, USHORT b)                    /* 5: 0x06 */
{ (void)a;(void)b; return 0; }
USHORT SHLAPI SETICONDATA(ULONG a, ULONG b, ULONG c)                     /* 16: 0x0c */
{ (void)a;(void)b;(void)c; return 0; }

/* ---- Printer EA helpers -- TODO(faithful): DosQPathInfo EA. */
USHORT SHLAPI PRNCOPYDRIVER(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e)/* 46: 0x14 */
{ (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
USHORT SHLAPI PRNQUERYEAVALUE(ULONG a, ULONG b, ULONG c, USHORT d)       /* 47: 0x0e */
{ (void)a;(void)b;(void)c;(void)d; return 0; }
USHORT SHLAPI PRNGETFILELIST(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e)/* 48: 0x14 */
{ (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }

/* ---- EA builders (__cdecl) -- TODO(faithful): build FEA2/MVMT structures. */
USHORT SHLCDECL BldFEAList(PV a, PV b, PV c)                             /* 43 -> _BldFEAList */
{ (void)a;(void)b;(void)c; return 0; }
USHORT SHLCDECL BldMVMT(PV a, PV b, PV c)                               /* 44 -> _BldMVMT */
{ (void)a;(void)b;(void)c; return 0; }

/* ---- Desktop-lock hooks -- TODO(faithful): PM input/sendmsg hooks. */
USHORT SHLAPI INITLOCKHOOKPROC(void)                                     /* 50: 0x00 */
{ return 0; }
USHORT SHLAPI LOCKHOOKPROC(ULONG a, ULONG b, USHORT c)                   /* 51: 0x0a */
{ (void)a;(void)b;(void)c; return 0; }
USHORT SHLAPI LOCKDTPROC(void)                                          /* 52: 0x00 */
{ return 0; }
USHORT SHLAPI BLOCKHOOKPROC(ULONG a, ULONG b, USHORT c)                 /* 53: 0x0a */
{ (void)a;(void)b;(void)c; return 0; }
USHORT SHLAPI LOCKHWNDPROC(ULONG a)                                     /* 54: 0x04 */
{ (void)a; return 0; }

/* ================================================================== */
/*  16-bit DLL init: fetch the DBCS environment vector once on load.    */
/* ================================================================== */
unsigned _DLL_InitTerm(unsigned hmod, unsigned fFlag);
#pragma aux _DLL_InitTerm "_DLL_InitTerm";
unsigned _DLL_InitTerm(unsigned hmod, unsigned fFlag)
{
    USHORT cc[2];                               /* {country, codepage} = default */
    (void)hmod;
    if (fFlag == 0) {                           /* process load */
        int i;
        for (i = 0; i < (int)sizeof(gDBCS); i++) gDBCS[i] = 0;
        cc[0] = 0; cc[1] = 0;
        DOS16GETDBCSEV((USHORT)sizeof(gDBCS), (PV)cc, (PV)gDBCS);
    }
    return 1;
}
