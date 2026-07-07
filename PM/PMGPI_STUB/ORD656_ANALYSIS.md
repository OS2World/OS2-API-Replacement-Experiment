# PMGPI ordinal 656 — analysis & Ghidra handoff

The blocker for a *booting* PMGPI clone. This captures everything known so the next
session (or you, with Ghidra) can pin it down and implement it for real.

## How we got here
- Clone reaches full ordinal parity (638/638) → **loads** on ArcaOS, boots into PM init.
- With `PmgpiOrd656` returning `0`: TRAP 000D (GP fault) in **PMDD** (display driver),
  `EAX=0`, `CR2=0x340`.
- Boot call log (`C:\PMGPI.LOG`, from the `-dPMGPI_LOG` build): **`Ord656` is the first and
  only PMGPI call at boot, invoked ~4x, then the trap.**
- With `PmgpiOrd656` returning a zeroed 2 KB buffer: no trap, but desktop **hangs** (black
  screen, blinking cursor) — 2x `Ord656` logged then silence. A zeroed buffer is not a valid
  structure, so PM stalls.

## What ord 656 IS (established)
- **Location:** original `OrigFiles\PMGPI.DLL`, LX **object 3, offset 0xB400** (file 0x131C4),
  32-bit code. Object 3 = the ~275 KB *core engine implementation* (not the thunk objects 1/2).
- **Role:** a **Device Support Function** (`INCL_GRE_DEVSUPPORT`, a `Gre*` engine service)
  that the display driver **PMDD imports and calls back into**, during driver enable.
- **Contract clue:** its return is used as a **pointer to a structure, dereferenced at +0x340**
  (from `CR2=0x340`, `EAX=0`). This matches the DC/DDC **instance data** ("Magic Cookie", `hddc`)
  that the graphics engine supplies to the driver.
- Called ~4x at boot — likely once per screen DC / per enable subfunction.

## Architecture (why a stub can't satisfy it)
From Winn 1991 *OS/2 PM GPI* + DDK `PDRREF.pdf` (*Presentation Device Driver Reference*):
```
app → GPI layer(owns PS) → Graphics Engine(owns DC) → Presentation Driver(owns DDC) → adapter
```
- Engine allocates a per-driver **dispatch table** of 32-bit `Gre*` pointers.
- On `OS2_PM_DRV_ENABLE` subfn 01 = **FillLogicalDeviceBlock**, the engine passes the table;
  the driver hooks its routines in. The DC "is seen internally as a dispatch table."
- Enable sequence: `FillLogicalDeviceBlock → FillPhysicalDeviceBlock → EnableDeviceContext →
  CompleteOpenDC`. The engine supplies a **pointer to per-DC instance data** to the driver.
- So ord 656 hands the driver a live engine-owned struct; only the real engine produces a
  valid one. No stub value works.

## Candidate identities (by contract, PDRREF Ch 14)
Enable-time `Gre*` device-support functions that return/populate a struct pointer:
1. an **instance-data / DDC accessor** (returns the per-DC instance-data block; +0x340 is a
   field in it) — strongest fit.
2. **`GreMemPtrs`** — returns a block of engine memory pointers.
3. an **engine memory allocator** (`SSAllocMem`-class) — driver allocs its instance data;
   `0` return → deref `NULL+0x340`.
4. **`QueryDeviceSurface` / `GreSetDeviceSurface`** — device-surface struct.

## The gap (needs disassembly)
PDRREF gives the function *contracts* but cannot map **PMGPI ord 656 → a name** (PMGPI export
ordinals ≠ dispatch-table function numbers; 656 is by-ordinal/unnamed). The in-repo Python
LX+capstone disassembler locates the code but its **fixup application drifts** — unreliable.

## Ghidra handoff — DO THIS
1. Install a Ghidra **LX loader** extension: `yetmorecode/ghidra-lx-loader` or
   `oshogbo/ghidra-lx-loader` (drop the release ZIP in `GHIDRA_INSTALL_DIR/Extensions/Ghidra`,
   then *File → Install Extensions*). Stock Ghidra has NO LX loader — it decompiles the DOS
   stub instead (INT 21h code = wrong).
2. Re-import `OrigFiles\PMGPI.DLL`, run auto-analysis.
3. Go to **object 3, offset 0xB400** (ordinal 656). Locate map via
   `scratchpad/lxdump.py` if needed.
4. Capture and share:
   - the decompiled function body,
   - its **call targets / imports** (calls to `DosAllocMem` → allocator; a specific `PMMERGE`
     entry → delegator; etc.),
   - what struct it returns and how **+0x340** is used.

## Then (implementation)
With the name + struct known: identify it in `PDRREF.pdf` Ch 14, define the real return
structure (at least through offset 0x340), and replace `PmgpiOrd656` (currently the zeroed
2 KB experiment) with a faithful implementation. NOTE: 656 is one link in the dispatch-table
chain — implementing it will surface the *next* `Gre*` call PMDD makes. This is the first
brick of the engine, not a one-shot desktop fix.

## References (all local)
- `refs\os2ddk2004\Docs-PDF\PDRREF.pdf` — Presentation Device Driver Reference (the DDI spec).
- `refs\os2ddk2004\Docs-PDF\DISPLAY.pdf` — VGA/SVGA display driver ref.
- `refs\os2ddk2004\ZIP\TCDISP\`, `ZIP\COMVIDEO\DDK\video\` — real display-driver SOURCE (shows
  concrete `Gre*`/dispatch-table usage).
- `refs\Dev Retail Books\1991 - OS2 Presentation Manager GPI - Winn.pdf` — PS/DC/DDC ownership,
  the "Magic Cookie" hddc.
