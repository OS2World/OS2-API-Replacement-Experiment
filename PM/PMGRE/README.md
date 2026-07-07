# PMGRE (cWarp clone)

Clone of OS/2 `PMGRE.DLL` — the Presentation Manager **Graphics Engine** entry DLL.

## What it is
The original `PMGRE.DLL` (2231 bytes, ArcaOS/Warp 4.52) is a **pure forwarder**:
**78 exports, every one an LX forwarder into `PMMERGE.DLL`**, with zero code of its own.
The real graphics-engine code was merged into `PMMERGE.DLL`; `PMGRE.DLL` is just the
named doorway PMGPI/PMWIN call through. Exports cover the GRE dispatch entries
(`GREENTRY2..10`, `GRE32ENTRY*`, `INNERGRE*`), driver info (`GETDRIVERINFO`,
`POSTDEVICEMODES`), 16↔32 call gates (`DISPATCH16GATE*`), semaphore/memory helpers
(`RAMSEM*`, `SSALLOC*`, `GREMEMPTRS`), NLS (`GRENLS`) and `SETDEVICESURFACE`.

## Files
- `PMGRE.DEF`  — the 78 forwarder exports (auto-generated from the original's LX table;
  each `NAME = PMMERGE.<ordinal> @<ordinal>`). **This is the whole clone.**
- `PMGRE.C`    — a minimal `_DLL_InitTerm` stub so the linker has a module to build.
- `PMGRE_MAKEFILE`, `compile.cmd` — build.

## Build note (important)
Must be linked with **IBM `LINK386`** (OS/2 Toolkit), NOT OpenWatcom `wlink`. wlink
cannot emit LX forwarder exports — `EXPORT foo = PMMERGE.n` makes it search for a local
symbol `PMMERGE.n` (this is exactly why the PMGPI ord-399 forwarder failed). LINK386's
DEF `EXPORTS name = module.ordinal` produces real forwarders. LINK386 runs on OS/2.

## Status & value
- Faithful, **definitively-completable** clone (a forwarder shim — unlike PMGPI, no engine
  to reimplement).
- Functionally it re-points to the same (still-closed) `PMMERGE.DLL`, so it adds no new
  capability by itself; its value is (a) a clean-room copy of the file, (b) a forward that
  will point at an *open* PMMERGE once that is cloned, and (c) a **template** for the many
  other OS/2 forwarder DLLs.
- The forwarder ordinals also map PMGRE's public API onto PMMERGE ordinals — a useful
  reference for any future PMMERGE work.
