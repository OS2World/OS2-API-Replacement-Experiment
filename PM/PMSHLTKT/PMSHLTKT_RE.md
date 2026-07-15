# PMSHLTKT.DLL — reverse-engineering notes (cWarp clean-room clone)

Original: `OrigFiles/PM/PMSHLTKT.DLL` — **16-bit** LX DLL, 44,232 bytes, EXEPACK-
packed, module name `PMSHLTKT`, **54 exports** (all 16-bit entries), single code
object (obj1) + data object (obj2). Imports: DOSCALLS, PMWIN, NLS, MSG, PMMERGE.

It is the **PM Shell Toolkit**: a helper library the Workplace Shell links
against. Two families:
* **A. pure utility** (strings/mem/convert/DBCS/validate) — clean-room-safe.
* **B. WPS integration** (file-op engine, object-msg procs, MRI loader, icon
  extraction, printer-EA helpers, desktop-lock hooks) — load-bearing; must be
  reimplemented faithfully against the documented 16-bit APIs below.

Tooling: `tools/dis16.py <dll> [ord...]` disassembles an export with LX import
fixups resolved to `MODULE.ord`; it also underpins the import map below. Built on
the EXEPACK:1/2 decoders shared with `tools/extract_res.py`.

## Calling convention

* Uppercase names → `__far __pascal` (callee cleanup, `retf N`, args pushed
  left-to-right so the **first** parameter sits at the highest `bp+` offset).
* `_BldFEAList` / `_BldMVMT` → `__cdecl` far (leading `_`, caller cleanup,
  `retf` with no operand).

Watcom mapping: declare pascal exports with an uppercase C name (the `__pascal`
keyword uppercases and omits the underscore, giving the exact export name);
declare the two cdecl exports as `BldFEAList`/`BldMVMT` (`__cdecl` prepends the
single underscore → `_BldFEAList` / `_BldMVMT`).

## Export ABI table (ord · name · retf · derived signature · family/status)

```
 1 DOSREFRESHICONS        retf ?    (frame 0x2d2, big)                 B  WIP
 2 GETICONDATA            retf 0x10 (4 dwords)                         B  WIP
 3 STRIPBLANKS            retf 6    (PCH str, USHORT flags)            A  done  bit0=lead,bit1=trail
 4 GETICONHANDLEFORVIOEXE retf 0xa                                     B  WIP
 5 GETICONHANDLEFORPMEXE  retf 6                                       B  WIP
 6 STRCAT                 retf 8    (PCH dst, PCH src)                 A  done
 7 STRCPY                 retf 8    (PCH dst, PCH src)                 A  done
 8 STRLEN                 retf 4    (PCH s) -> USHORT                  A  done
 9 ITOA                   retf 6    (USHORT value, PCH buf) decimal    A  done
10 STRCHR                 retf 6    (PCH s, USHORT ch) -> PCH          A  done
11 STRRCHR                retf 6    (PCH s, USHORT ch) -> PCH          A  done
12 MEMCPY                 retf 0xa  (PVOID dst, PVOID src, USHORT n)   A  done
13 STRCMP                 retf 8    (PCH a, PCH b) -> SHORT            A  done
14 ATOI                   retf 4    (PCH s) -> SHORT                   A  done
15 ULTOA                  retf 8    (ULONG value, PCH buf) decimal     A  done
16 SETICONDATA            retf 0xc                                     B  WIP
17 ISDBCS                 retf 2    (USHORT ch) -> BOOL (DBCS lead)    A  done
18 NEXTPCH                retf 4    (PCH p) -> PCH                     A  done
19 PREVPCH                retf 8    (PCH start, PCH cur) -> PCH        A  done
20 STRCMPI                retf 8    (PCH a, PCH b) -> SHORT            A  done
21 STRNCMPI               retf 0xa  (PCH a, PCH b, USHORT n)          A  done
22 STRNCAT_TRUNC          retf 0xa  (PCH dst, PCH src, USHORT n)      A  done
23 STRNCAT                retf 0xa  (PCH dst, PCH src, USHORT n)      A  done
24 STRNCPY_TRUNC          retf 0xa  (PCH dst, PCH src, USHORT n)      A  done
25 STRNCPY                retf 0xa  (PCH dst, PCH src, USHORT n)      A  done
26 STRSTR                 retf 8    (PCH hay, PCH needle) -> PCH       A  done
27 STRUPR                 retf 4    (PCH s) -> PCH                     A  done
28 XLATBLANK              retf 4    (PCH s) -> PVOID (DBCS blank xlat) A  done (non-DBCS no-op)
29 ATOL                   retf 4    (PCH s) -> LONG                    A  done
30 STRNCMP                retf 0xa  (PCH a, PCH b, USHORT n) -> SHORT  A  done
31 FSTRAVERSE             retf 0x14                                    B  WIP  DosFindFirst2/Next/Close
32 FSQUERYDRIVEDATA       retf 4    (PVOID) -> USHORT                  B  WIP  DosQFSAttach
33 FSRENAME               retf 0x14                                   B  WIP  DosMove
34 FSCOPY                 retf 0x14 (frame 0x12a)                      B  WIP  DosOpen/Read/Write loop
35 FSMOVE                 retf 0x14 (frame 0x12a)                      B  WIP  DosMove/copy+delete
36 FSDELETE               retf 0x10 (4 dwords)                         B  WIP  DosDelete via ctrl-block engine
37 OBJCREATEMSGPROC       retf 0xc  (dw a, dw b, dw c) -> b            B  done (returns 2nd param)
38 OBJDESTROYMSGPROC      retf 4    (dw a) -> 0                        B  done (returns 0)
39 OBJSENDMSG             retf 0x16                                    B  WIP  WinSendMsg dispatch
40 FSTRAVINFO             retf 4                                       B  WIP
41 MRILOADRESOURCE        retf 0x10 (frame 0x510)                      B  WIP  DosGetResource/WinLoadMessage
42 STRPBRK                retf 8    (PCH s, PCH set) -> PCH            A  done
43 _BldFEAList            retf      (__cdecl)                          B  WIP  build FEA2 list
44 _BldMVMT               retf      (__cdecl)                          B  WIP  build MVMT EA value
45 ISNUMBER               retf 4    (PCH s) -> BOOL                    A  done  (STRIPBLANKS then digits)
46 PRNCOPYDRIVER          retf 0x14                                   B  WIP
47 PRNQUERYEAVALUE        retf 0xe                                     B  WIP  DosQPathInfo EA
48 PRNGETFILELIST         retf 0x14                                   B  WIP
49 WSPVAL                 retf 8    (SEL sel, USHORT off, USHORT len, USHORT rw) -> BOOL  A done (lsl/verr/verw)
50 INITLOCKHOOKPROC       retf      (no args)                          B  WIP  install PM input hooks (desktop lock)
51 LOCKHOOKPROC           retf 0xa                                    B  WIP  PM input hook proc
52 LOCKDTPROC             retf      (no args)                          B  WIP
53 BLOCKHOOKPROC          retf 0xa                                    B  WIP  PM sendmsg hook proc
54 LOCKHWNDPROC           retf 4                                      B  WIP  lock window proc
```

## Import vocabulary (resolved via tools/dis16.py fixup map + toolkit bseord.h)

DOSCALLS (by ordinal → 16-bit API): 4 DosGetDBCSEv · 33 DosGetDateTime ·
34 DosAllocSeg · 38 DosReallocSeg · 39 DosFreeSeg · 44 DosLoadModule ·
46 DosFreeModule · 47 DosGetModHandle · 53 DosDevIOCtl · 57 DosChDir ·
58 DosChgFilePtr · 59 DosClose · 60 DosDelete · 63 DosFindClose ·
65 DosFindNext · 66 DosMkDir · 67 DosMove · 68 DosNewSize · 70 DosOpen ·
71 DosQCurDir · 72 DosQCurDisk · 74 DosQFileInfo · 75 DosQFileMode ·
78 DosQVerify · 80 DosRmDir · 81 DosSelectDisk · 83 DosSetFileInfo ·
84 DosSetFileMode · 86 DosSetVerify · 95 DosOpen2 · 98 DosQPathInfo ·
104 DosSetPathInfo · 120 DosError · 130 DosGetCp · 137 DosRead ·
138 DosWrite · 155 DosGetResource · 161 DosFSRamSemRequest ·
162 DosFSRamSemClear · 163 DosQAppType · 182 DosQFSAttach ·
184 DosFindFirst2 · 185 DosMkDir2 · 191 DosEditName.
NLS: 1/3/4 (DBCS/codepage). MSG.2 (message retrieval). PMWIN.25/69/139,
PMMERGE.2154 (window/resource helpers).

## Build

16-bit DLL, exactly like BVHWNDW: `wcc -bt=OS2 -ml -2 -s -wx -zu -zp1`, `wlink`.
`os2286.lib` + `clibl.lib`. All 54 exports at their original ordinals AND names
(drop-in parity). See PMSHLTKT.LNK / PMSHLTKT_MAKEFILE.

## FS-operation engine (ords 31/33/34/35/36) — architecture (RE 2026-07-09)

FSDELETE/FSRENAME/FSMOVE/FSCOPY are thin ABI wrappers that install a far pointer
into the caller's **control block** at field +0x60/+0x62 (saved/replaced/restored;
looks like a per-operation callback or exception frame) and call a shared generic
worker **obj1:0x301c** (`retf 0x14`, 20 bytes of args = a mix of paths + flags +
the control block + a far status pointer). 0x301c is NOT a thin Dos wrapper — it
is the WPS drag-drop file-operation core:

* **obj1:0x5148** (near/cdecl, 3 far-ptr args) — allocates a large (~2.4 KB)
  **context segment** and parses source/target paths into it. Returned far ptr is
  kept in a local; freed at the end via **DosFreeSeg (DOSCALLS.39)**. Context
  fields seen: +0,+2 (a string), +4,+6 (a string), +0x14,+0x16 (a string),
  +0x430 (path buffer), +0x548 (flag byte, bit1 tested), +0x98a/+0x98c (state).
* **obj1:0x4d84** (near/cdecl) — path parsing/normalization; calls the internal
  STRLEN/STRCPY/STRIPBLANKS/NEXTPCH/ISDBCS + more. No direct Dos calls (the real
  filesystem ops are deeper in its callees: DosFindFirst2/Delete/Move/QPathInfo…).
* **obj1:0x5426** (near/cdecl) — a later stage; result folded into the return.
* CR/LF stripping loop over the target string; DBCS-aware via ISDBCS.
* Error/confirm reporting: **OBJSENDMSG (obj1:0x434)** to the WPS object in the
  control block, with message id **0x13f7** and a Dos error code — i.e. the
  file-op progress/confirm/error dialogs are driven through the caller's object.

### Corrected model (RE 2026-07-10) — traverse + callback, not a monolith

* `obj1:0x301c` is the export **FSTRAVERSE** itself. FSDELETE/RENAME/MOVE/COPY are
  wrappers that (1) build a per-file operation **callback** via OBJCREATEMSGPROC,
  (2) swap the control-block +0x60 far pointer, (3) call FSTRAVERSE to walk the
  path and invoke the callback per file, (4) OBJDESTROYMSGPROC, (5) restore +0x60.
* **The OBJ*MSGPROC trio is a far-function-pointer trampoline, not PM messaging:**
  - OBJCREATEMSGPROC(a, fn, c) → returns `fn` (identity register).
  - OBJSENDMSG(fn, a,b,c,d,e) → `if(!fn) return 1; else return fn(a,b,c,d,e);`
    (pushes the 18-byte arg frame, `lcall [fn]`, callee-cleanup ⇒ __far __pascal).
  - OBJDESTROYMSGPROC(fn) → returns 0 (no-op).
  → **DONE & faithful** in PMSHLTKT.C (OBJSENDMSG now actually invokes the callback).
* **FSQUERYDRIVEDATA(path)** returns a far pointer to a per-drive 6-byte record:
  UNC (`\\`) paths get a rotating slot (`g16c`, counter `g178`); a drive-letter
  path returns `&gDriveData[(upcase(letter)-'A')*6]` at data offset 0xd0. Depends
  on the obj2 data-segment layout + whatever fills the records → deferred until
  the filler is traced.

Scope (crawl2.py, fixpoint boundaries): the FS exports reach ~50+ genuine engine
functions / several thousand instructions once the already-done string helpers are
excluded — biggest bodies: FSTRAVERSE (307), sub_33b4 (597), sub_3c02 (389),
sub_4278 (347), sub_4d84 (308, path parse), sub_49fe (164). Multi-session grind.

Implication: a *faithful* clone of the FS family = reconstructing (a) the caller's
**control-block struct** (40+ fields, undocumented WPS layout), (b) the ~2.4 KB
**context struct**, and (c) the multi-stage engine (0x5148 + 0x4d84 + 0x5426 +
callees, >1000 instructions) plus the OBJSENDMSG message protocol (id 0x13f7 …).
This is effectively decompiling a WPS subsystem, and — like PMMRGRES — a wrong
implementation is *harmful* (breaks WPS copy/move/delete), not inert. Terminal Dos
ops are ordinary (DosDelete/DosMove/DosFindFirst2/DosOpen+Read+Write/EA), so the
work is in the WPS-coupling, not the filesystem primitives. **Decision point
raised with the user 2026-07-09.**

## Status

Family A (26 functions: all string/mem/convert/DBCS/validate) — implemented and
faithful. Family B — ABI-correct declarations in place; OBJ{CREATE,DESTROY}MSGPROC
done; the file-op engine, MRI loader, icon extraction, printer-EA helpers, EA
builders and desktop-lock hooks are being reimplemented against the imports above
across subsequent passes (each verified with tools/dis16.py before coding).

## FS engine — decompilation working notes (in progress, 2026-07-10)

### Caller control block (CB) — fields used by the engine
Offsets into the caller's (WPS) struct passed as the FS functions' 2nd far-ptr arg.
NOTE: the engine READS and WRITES this WPS-owned struct at these fixed offsets —
every offset must be exact.
* +0x04  USHORT  work-segment size hint (0 => default 0x8000)
* +0x60  PVOID   callback/exception frame (the FS wrappers save/replace/restore it)
* +0x68  PVOID   (copied to CTX+0x536 when flagsA & 0x400)
* +0x6c  USHORT  (copied to CTX+0x544 when flagsA & 0x800)
* +0x70  PVOID   (copied to CTX+0x87d)

### Work-context (CTX) — allocated by sub_5148 (DosAllocSeg), far ptr returned
Segment size = hint (min 0x2000, round up 4K) or 0x8000 default. Layout:
* +0x00  PVOID -> CTX+0x18    (src path buffer ptr)   ; buf[0]=0
* +0x04  PVOID -> CTX+0x11e   (tgt path buffer ptr)   ; buf[0]=0
* +0x10  PVOID -> CTX+0x224
* +0x14  PVOID -> CTX+0x32a
* +0x18  char[0x106]  path buffer A
* +0x11e char[0x106]  path buffer B
* +0x224 char[0x106]  buffer C
* +0x32a char buffer D
* +0x430 char[]  per-file work path (STRCPY target in the loop)
* +0x536 USHORT  = 0 or CB+0x68 if flagsA&0x400
* +0x538 PVOID -> CTX+0x990   (DosFindFirst2 result buffer)
* +0x53c USHORT  = segsize - 0x990  (find-buffer capacity)
* +0x53e USHORT  = segsize
* +0x540 PVOID   = 0 or CB+0x60 if flagsA&0x100  (callback frame)
* +0x544 USHORT  = 1 or CB+0x6c if flagsA&0x800
* +0x546 ULONG   = flagsA (arg3; bit8=0x100,bit10=0x400,bit11=0x800 seen)
* +0x54a PVOID   = CB (control block)
* +0x54e PVOID   = arg (target/dest far ptr)
* +0x552 PVOID -> CTX+0x556
* +0x556 buffer
* +0x883 char    current drive letter (DosQCurDisk + 0x40)
* +0x884 ':'  +0x885 '\'  +0x886 char[] current dir (DosQCurDir)
* +0x87d PVOID   = CB+0x70
* +0x988 USHORT  = 1
* +0x98a USHORT  = 0   +0x98c USHORT = 0
* +0x990 DosFindFirst2 result buffer (rest of segment)

### FSTRAVERSE(callback, CB, flagsA, dest, pathlist)  [ord 31, retf 0x14]
1. CTX = sub_5148(flags/size, CB, dest-ish)  → alloc+init context (above).
   On failure: post error via OBJSENDMSG(CB-frame, msg 0x13f7, code) and return.
2. For each path in `pathlist` (null-separated, double-null terminated):
   a. STRCPY the path into a CTX buffer.
   b. rc = sub_4d84(CTX, CTX->bufptrs)   — path parser/normalizer (308 instr).
   c. if parse ok: rc = sub_33b4(CTX)    — find+callback worker (597 instr):
      expands wildcards via DosFindFirst2/FindNext/FindClose and calls the
      per-file callback through OBJSENDMSG. rc 0x12 (NO_MORE_FILES) / 8 => 0.
   d. advance cursor: STRLEN + NEXTPCH to the next path; stop at the double null.
3. Free CTX (DosFreeSeg) and return the final rc; errors reported via OBJSENDMSG.

### Remaining to decompile for the FSDELETE path
* sub_4d84 (path parser, 308 instr)
* sub_33b4 (find + per-file callback worker, 597 instr) — the core
* the delete callback at obj1:0x74a2 (invoked per file by sub_33b4)
Then generalize to rename/move/copy (their callbacks + dest handling).

### sub_33b4 (find dispatcher, arg=CTX)  [called by FSTRAVERSE per path]
Splits source (CTX+0/+2) and target (CTX+4/+6) paths into directory + pattern:
* STRPBRK(path, "*?") or STRLEN(path)==3 ("X:\") => path has a wildcard/pattern.
  Then STRRCHR(path,'\') -> split ptr saved (src: CTX+8/+0xa, tgt: CTX+0xc/+0xe),
  the '\' is overwritten with NUL (dir part), and si (src) / di (tgt) flag set.
* No wildcard => split ptr = end-of-string.
Sends a "begin" notify to the callback: OBJSENDMSG(CTX+0x540 frame, msg **0x12da**,
srcptr, CTX, dest CTX+0x54e). Then dispatches:
* if si (src wildcard): if CTX->flags(+0x546) & 2 -> **sub_3c02** else **sub_4278**
  (the DosFindFirst2 enumeration + per-file callback workers). return rc.
* else (single named file): fall through at 0x3578 (single-file callback path).
CTX fields added: +8/+0xa src split ptr, +0xc/+0xe tgt split ptr, +0x14 -> CTX+0x32a
scratch, +0x988 (mode), +0x546 ULONG op-flags (bit1=alt path, bit2, bit10/11 seen).

### obj2 data constants (data segment)
* 0x7d "*?"  0x80 "*?"  (wildcard-detect sets for STRPBRK)
* 0x83 "\"  0x85 "\*"  0x8f "."  0x91 ".."  0x9d/0x9f "."  0xa3 "*.*"
* 0x178 UNC drive counter g178 = 0x1e (FSQUERYDRIVEDATA rotating id base)
* 0x17b "FsCopy"   0x183 ".LONGNAME" (EA name for long filenames)

### Remaining call tree for a faithful FSDELETE (instr counts)
FSDELETE(57) -> FSTRAVERSE(307) -> sub_5148(ctx,~90 DONE) , sub_4d84(308 parse),
  sub_33b4(597 dispatch: part-1 DONE) -> { sub_4278(347), sub_3c02(389) } enumerate
  + 0x3578 single-file path (~rest of sub_33b4) -> per-file callback obj1:0x74a2.
Still to decompile: sub_4d84, sub_4278, sub_3c02, sub_33b4 tail (0x3578), the
delete callback. ~1600+ instr. rename/move/copy add their own callbacks + dest logic.

## Differential findings vs IBM binary (tests/diff, 2026-07-14)

Running the same Family-A vectors against IBM's `OrigFiles\PM\PMSHLTKT.DLL` and
the clone (via DosLoadModule) revealed 5 behavioural divergences the contract-only
tests missed. All now corrected in PMSHLTKT.C to MATCH IBM (verified authoritative):

* **STRNCPY (25)** — IBM does NOT null-pad. Copies up to n chars, stops after the
  NUL; leaves the rest of the buffer untouched. (Not standard C strncpy.)
* **STRNCPY_TRUNC (24)** — on a single-byte codepage, identical to STRNCPY (no pad,
  no forced NUL). The _TRUNC difference is DBCS-boundary only (deferred).
* **STRNCAT_TRUNC (22)** — same as STRNCAT: terminator written at d[n], so it can
  truncate the destination (earlier clone terminated at d[len] — wrong).
* **STRSTR (26)** — empty needle returns NULL (not `hay` as standard C does).
* **ATOL (29)** — accepts an optional leading '+' ONLY; does NOT handle '-'
  (returns 0 for negative) and does NOT skip whitespace. (IBM quirk; ATOI, by
  contrast, handles both — confirmed by disasm at obj1:0x982.)

Cosmetic-only (not a real divergence): XLATBLANK's single-byte return is an
undefined register value in IBM (stale dx); the clone returns clean NULL. Both
leave ax=0; the return is unused. Not compared in the harness.

~20 other Family-A functions matched IBM exactly on first run.

## COMPLETE FSDELETE path decompiled (2026-07-15)

The engine is a generic recursive enumerator that sends 3 messages to a per-
operation callback; the callback does the real Dos work. Full call graph:

FSDELETE(cb-frame,ctrlblk,flags,pathlist)                      [wrapper, ord 36]
  -> builds a delete callback via OBJCREATEMSGPROC(obj1:0x74a2), swaps ctrl+0x60
  -> FSTRAVERSE(callback,ctrlblk,flags,dest,pathlist)          [ord 31]
       sub_5148  : alloc+init work context (done)
       per path in the double-NUL list:
         sub_4d84 : parse/normalise the path            (still TODO in detail)
         sub_33b4 : split dir/pattern at last '\'; STRPBRK(,"*?");
                    OBJSENDMSG(frame, msg 0x12da BEGIN); dispatch:
           sub_4278 : RECURSIVE DosFindFirst2 enumerator
                        per-level find handle array CTX+0x671[lvl],
                        state CTX+0x777[lvl], level index CTX+0x881,
                        find buffer CTX+0x990 (FILEFINDBUF level 1)
             sub_48ea : iterate FILEFINDBUF entries (cchName@+0x16, name@+0x17,
                        variable stride); call sub_49fe per name
               sub_49fe : per entry --
                   sub_50be  : wildcard match name vs pattern (unless flag bit1)
                   build full source path (dir + '\' + name via STRCPY)
                   if DIRECTORY (attr&0x10): OBJSENDMSG(msg 0x12dd DIR)
                   else FILE: sub_4c40 (build target path if dest set);
                              OBJSENDMSG(frame, msg 0x12dc FILE) -> callback;
                              CTX+0x98c++ (processed count)
                   if dir && recurse-flag(CTX+0x548&4) && name!="."/"..":
                              recurse into the subdir
             sub_5356 : find-error handler
             sub_4be0 : per-level setup
           sub_3c02 : ALT enumerator (when op-flag bit1 set)  (TODO)

### Per-operation callback protocol (obj1:0x74a2 = DELETE), retf 0x12
A message switch on msg (bp+0x16):
  0x12da BEGIN  -> OBJSENDMSG(parent, msg 0x1316, ...) forward begin
  0x12dc FILE   -> (obj1:0x7530): OBJSENDMSG(parent, msg 0x1318 CONFIRM);
                   if confirmed: DosDelete(path) [DOSCALLS.60];
                   on ACCESS_DENIED(5)/SHARING(0x20): DosSetFileMode(path,0)
                   [DOSCALLS.84] to clear read-only, then retry;
                   errors -> sub_558e (error dialog).
  0x12dd DIR    -> default handler (0x77a1)
  0x12e2/0x12e3/0x13ed -> other phases (end/abort/etc.)

So the ACTUAL delete = DosDelete + read-only-clear-and-retry, wrapped in the
WPS confirm/error message protocol (callback<->parent object: msg 0x1316 begin,
0x1318 confirm). rename/move/copy reuse the SAME engine with their own callback
(FSRENAME/FSMOVE/FSCOPY each build a different obj1:0xNNNN callback) + dest path.

### Data globals used by the engine
0x97 scratch path; 0x9d "."; 0x9f ".."; (plus the 0x7d/0x80 "*?" wildcard sets).

### Still to decompile before writing C: sub_4d84, sub_50be (wildcard match),
sub_4c40, sub_5356, sub_4be0, sub_558e, sub_3c02; and the rename/move/copy
callbacks. The engine skeleton + delete callback are now fully understood.

### sub_50be (wildcard matcher) -- DONE, DosEditName-based
sub_50be(name, pattern) -> BOOL:
  if (!name || !pattern) return 0;
  if (STRCMP(pattern, "*")==0)   return 1;   /* data 0xbc = "*"   */
  if (STRCMP(pattern, "*.*")==0) return 1;   /* data 0xbe = "*.*" */
  if (DosEditName(1, name, pattern, buf[0x104], 0x104) != 0) return 0;  /* DOSCALLS.191 */
  return STRCMPI(name, buf)==0 ? 1 : 0;      /* pattern-applied name == name -> match */
Fully clean-room reimplementable (documented API) + unit-testable. No hand-rolled
glob logic -- OS/2 idiom is to round-trip the name through DosEditName.

### sub_4be0 / sub_5356 -- DONE
sub_4be0(CTX): only for info-level 3 (EA) -- fills the EAOP2 in the find buffer
  (fpGEAList=CB+0x70, fpFEAList=findbuf+0xc, oError=0). Level-1 (delete) = no-op.
sub_5356(CTX, err, path): returns 0x12 (ERROR_NO_MORE_FILES) unchanged (benign
  loop terminator); real errors handled at 0x536c (-> error dialog).
