@REM Build the cWarp PMGRE.DLL clone (pure forwarder to PMMERGE).
@REM Requires IBM LINK386 on PATH (OS/2 Toolkit) -- wlink cannot emit forwarders.
wmake -f PMGRE_MAKEFILE clean
wmake -f PMGRE_MAKEFILE 2>&1 |tee make.out
