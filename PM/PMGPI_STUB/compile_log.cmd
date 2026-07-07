@REM Build the INSTRUMENTED PMGPI.DLL (-dPMGPI_LOG).
@REM Logs every Gpi* call to C:\PMGPI.LOG (write-through, survives a boot hang).
@REM Swap this DLL in, boot the PM desktop, then read C:\PMGPI.LOG for the hot-path.
wmake -f PMGPI_MAKEFILE clean
wmake -f PMGPI_MAKEFILE LOGFLAG=-dPMGPI_LOG 2>&1 |tee make.out
