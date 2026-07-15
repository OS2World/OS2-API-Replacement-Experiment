# BVHWNDW.DLL — reverse-engineering findings

Complete architectural RE of IBM's BVHWNDW.DLL (16-bit, LX, 16797 b), obtained
by disassembling the binary (capstone, 16-bit) after decompressing its EXEPACK:2
code/data objects. Tools: `tools/dump_lx.py`, `tools/extract_res.py` (EXEPACK
decoders), plus the scratchpad disasm scripts.

## Module shape
- 16-bit DLL, 3 exports: **BUFFERUPDATE (1)**, **DEVENABLE (2)**, **GETCONFIG (3)**.
- Objects: obj1 = 16-bit code (~0x4300 b), obj2 = 16-bit data (1525 b), obj3 = BSS.
- Imports: **PMVIOP** (WinCreateConsole/WinDestroyConsole/WinSyncWithPS/LockVioPS/
  UnlockVioPS), **PMGPI.6**, **PMMERGE.2036/2041**, **OS2CHAR.2019** (fn) +
  **OS2CHAR.148** (data), **DOSCALLS** (8=DosGetInfoSeg, 130, 863(data), 70, 38,
  42, 59, 141, 20, 36, 393, 394, 732).

## It is NOT a DDK vector-table BVH
The DDK `xgabvh20` model (fill a caller-supplied `CallVectorTable[256..279]`) does
**not** apply. BVHWNDW uses its own contract:

### Calling convention
All 3 exports start with `call 0x4f1` (a Watcom context-saving prologue) and end
`call 0x51e; retf 0xc`. Helper 0x4f1: `pushf; pushal; push ds/es/gs; mov bp,sp`
then loads **ES:DI = param1** (`[bp+0x34]`, the *device block*), **DS:SI = param2**
(`[bp+0x30]`, the *ParmBlock*), sets DS = data seg, and stores param1's segment
into `OS2CHAR.148[0x18]`. Param3 = `[bp+0x2c]` = Function code. So:
`USHORT far pascal fn(DEVICEBLOCK far *p1, PARMBLOCK far *p2, ULONG Function)`.

### DEVENABLE (ord 2)
1. Require `Function == 1` (FnFillLogicalDevBlock) else return 0xFFFF.
2. Require `ParmBlock[+4] >= 0x11a` else return 0x1DF.
3. **First init only** (global `g[4]==0`): copy a **26-entry function-pointer
   table** (data 0x14d) into `*(DEVICEBLOCK[+4]) + 0x400`, skipping entries whose
   segment word == 0. Then `DosGetInfoSeg(&g[4], &g[2])` (g[4]=GIS sel, g[2]=LIS
   sel); `al = GIS[0x19]-1; g[6]=al`; `DOSCALLS.130(6,&g[0x18],&g[0x16]); g[0x16]>>=1`.
4. **Every call**: register selectors into the shared **OS2CHAR.148** struct:
   `[0x126]=[0x16c]=g[4]` (GIS), `[0x128]=[0x16e]=g[2]` (LIS). Then `es=GIS;
   if (GIS[8] > g[6]) { PMMERGE.<0x9d>(0); if err 0x1DF; PMMERGE.2036(0,0); if err 0x1DF }`.

### The 26-entry handler table (data 0x14d), Fn 256..281
| Fn | off | meaning |
|----|-----|---------|
|256|0x10e8|BUFFERUPDATE|
|257|0x00c9|InitEnv (sets Env[0x1a]=2, Env[0x1c]=0x00500019 → 80x25, Env[0x28]=0x00070006, Env[0x68]=0, Env[0x85]=1)|
|258,259|0x04ec|Save/RestoreEnv = `sub ax,ax; retf 0xc` (return 0)|
|260|0x0333|GETCONFIG|
|261|0x07ed| |
|262,263,266,267,270,271,272,273|0x04e6|common stub `mov ax,0x1ee; retf 0xc`|
|264|0x053a| |265|0x05fa| |
|268|0x08af|GetMode| |269|0x094b|SetMode|
|274|0x06f5| |275|0x0756| |276|0x03b5| |
|277,278|0x0000|null → skipped|
|279|0x0824|GetLVBInfo| |280|0x052e (`mov ax,0x1a5`)| |281|0x0534 (`mov ax,0x1a5`)|

### GETCONFIG (ord 3) — DONE, reimplemented
Validate `ParmBlock->Length == 8`; copy the exact 40-byte config table (data
0x11d) into `ConfigDataPTR` up to its `cb`. Table:
`28 00 ffff ffff 00800000 0000 0000 0000 00ff0000 00800000 00800000 20 00 24 00 01 00 ffff 01 00 ffff`.

### BUFFERUPDATE (ord 1) — characterized, not yet reimplemented
Param = VIOPS. Reads dims at +0x1a/+0x1c/+0x1e, cols/rows at +0x7c/+0x7e (defaults
0x50/0x19), lock selector at +0xf7 → `LockVioPS`; then draws via PMGPI.6; `UnlockVioPS`.

## InitEnv (Fn 257, 0xc9) — full anatomy (why windowed cmd.exe needs it)
Not just a few field sets. It:
1. Zeroes a per-session slot in a table at data 0x3c0, indexed by `parmblock[6]&0xff`.
2. If `parmblock[6]>>8 == 2` (text session): **copies a 255-byte env template**
   (data 0x1e) into the environment, then sets fields: env[0xfd]=al, env[0x14]=
   parmblock[4], env[0x1a]=2, env[0x1c]=0x00500019 (80x25), env[0x28]=0x00070006,
   env[0x68]=0, env[0x85]=1, env[0xf9]=0, env[0xbb]=g[0x1a], env[0xb9]=0.
3. Calls an import (OS2CHAR?) with (0xa, env:0xb9, env:0xad); stores far ptr
   env:0xad at env[0x6c].
4. Computes LVB size = env[0x1c]*env[0x1e]*env[0x1a], stores env[0xf5]=size-dx,
   then **DosAllocSeg(size, &env[0xf7], 2)** — allocates the logical video
   buffer; env[0xf7] = its selector; then zero-fills it. (This buffer is the
   reason a stubbed InitEnv makes windowed cmd.exe fail.)
5. Else branch (0x1f1): graphics-mode path (DOSCALLS.70/732/59).
Confirmed by build: table-install-only DEVENABLE boots and full-screen cmd.exe
works, but windowed cmd.exe fails because these handlers are stubs.

## CURRENT STATUS (paused 2026-07-08)
BVHWNDW.C is a partial transcription. Build state, confirmed on the VM:
- **Boots** with `BVHWNDW_REGISTER 0` (DEVENABLE installs the 26-handler table
  only; the OS2CHAR.148 registration is disabled because enabling it HUNG the
  boot — the `&Os2Char148` data-import pointer or the magic offsets are wrong).
- **Full-screen cmd.exe works** (uses the hardware BVH, not ours).
- **Windowed cmd.exe still does NOT start** (silent; no error/trap) even with the
  transcribed InitEnv (255-byte template + fields + DosAllocSeg LVB @env[0xf7]).
- **Marker-file instrumentation** (`BVHWNDW_MARK 1`, replaces the old DosBeep
  build — VirtualBox has no PC speaker, so beeps were inaudible). Each handler
  creates a distinct `C:\BVHxx.MRK` file via `DosOpen`(create)/`DosClose`:
  DEVENABLE=`BVHDE`, InitEnv entry=`BVHIE`, InitEnv done=`BVHIE2`, GetMode=`BVHGM`,
  SetMode=`BVHSM`, GenericOK=`BVHGN`, BufferUpdate=`BVHBU`. Procedure: delete
  `C:\BVH*.MRK`, launch a windowed cmd.exe, then `dir C:\BVH*.MRK`. Presence of
  `BVHIE` proves session-create reached BVHWNDW (we are hooked in); its ABSENCE
  means the create fails before us → the OS2CHAR.148 registration is required.

Confirmed ordinals (toolkit `bseord.h`): DosGetInfoSeg=8, DosAllocSeg=34,
DosOpen=70, DosClose=59 — all as used.

## >>> UNBLOCKED (2026-07-08): the BVH contract is now DOCUMENTED <<<
`refs\IBM RedBooks\os2-2.0-pdd-ref-1992.pdf` (Physical Device Driver Reference,
S10G-6266) **Ch. 14 "Physical Video Device Drivers", p164+** documents the whole
BVH interface. It validates the RE and answers the open questions:
- Calling seq (p167): `PUSH Environment; PUSH ParmBlock; PUSH Function; CALL FAR`.
  Environment = "huge selector, format defined by the BVH developer"; ParmBlock =
  {WORD Length; WORD Flags; ...}.
- **DevEnable (p168)**: Parameter2 = far ptr to {DWORD FlagsPtr; DWORD CallVectorTable}.
  "the nth BVH function is the nth DWORD in the table, beginning with Function 0."
  => our `deviceblock[+4]+0x400` IS `CallVectorTable + 256*4` (BVHWNDW's functions
  are 256..281). Parameter1 = {DWORD EngineVersion; DWORD CountOfTableFunctions}.
  Subfunction must be 1 (Fill Logical Device Block).
- Function numbers confirmed: 256 Text Buffer Update, 257 Init Environment, 258/259
  Save/Restore Env, 260 Query Config Info, 261 Query DBCS Display Info, 268/269
  Get/Set Mode, 274/275, 279 GetLVBInfo.
- **Text Buffer Update (256), p171-173** — full ParmBlock: WORD ParmLength, WORD
  Flags, DWORD AppDataAddr, DWORD SecondAddr, WORD Index, WORD StartRow/StartCol,
  WORD SecondRow/SecondCol, WORD RepeatFactor, WORD LogicalBufSel, WORD TouchX/Y
  Left/Top/Right/Bottom, WORD LVBRowOff/ColOff/Width/Height, BYTE LVBFormatID, BYTE
  LVBAttrCount. Index = operation: 0 read cell-types, 1 read chars, 2 read cells,
  3-6 scroll U/D/L/R, 7 write cells, 8 write chars, 9 write chars w/ const attr.
  Touch* = the changed rectangle. THIS is the BUFFERUPDATE render contract.

### Revised approach on resume
Reimplement each handler from the DOCUMENTED contract (Ch.14) rather than
transcribing the binary — cleaner, likelier correct, and closer to clean-room.
Read Ch.14 fully (p164 to ~p200) for InitEnvironment, QueryConfigInfo, Get/SetMode,
and the LVB format, then rewrite InitEnv/BUFFERUPDATE/GetMode/etc. from spec.

## (historical) open questions now answered by the PDDR
2. **OS2CHAR.148** — what structure is it, and does the registration (writing
   GIS/LIS selectors to +0x126/+0x128/+0x16c/+0x16e) wire BVHWNDW into the
   windowed path? Need OS2CHAR.DLL to inspect ord 148, and the right way to take
   its address from C (the `&Os2Char148` import corrupted memory).
3. **Which handler the session-create path needs** — the DosBeep build answers
   this empirically (run it, note the last pitch heard).
4. **InitEnv's skipped calls** — the 0x162 (OS2CHAR?) call, the big 0x1ae call,
   and the internal helpers 0x1053 / 0x2c7 (each another function to decompile).

## Remaining to reimplement faithfully
- DEVENABLE registration reproduced exactly (needs OS2CHAR.148 imported *data* +
  its undocumented offsets 0x18/0x126/0x128/0x16c/0x16e; DosGetInfoSeg; PMMERGE.2036).
- ~13 substantive handlers decompiled (InitEnv mostly done; GetMode/SetMode/
  GetLVBInfo/261/264/265/274/275/276 + BUFFERUPDATE rendering).
- This amounts to transcribing most of the 16 KB driver + its cross-module
  (OS2CHAR/PMMERGE) structure coupling.
