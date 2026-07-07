# Top-level makefile - builds all five spooler components in dependency order.
# Run setenv.cmd once first, then: wmake   (or run each component's compile.cmd)

all: spl1b pmspl spoolcp spool pmspool

spl1b: .SYMBOLIC
    cd spl1b && wmake -f SPL1B_MAKEFILE && cd ..

pmspl: .SYMBOLIC
    cd pmspl && wmake -f PMSPL_MAKEFILE && cd ..

spoolcp: .SYMBOLIC
    cd spoolcp && wmake -f SPOOLCP_MAKEFILE && cd ..

spool: .SYMBOLIC
    cd spool && wmake -f SPOOL_MAKEFILE && cd ..

pmspool: .SYMBOLIC
    cd pmspool && wmake -f PMSPOOL_MAKEFILE && cd ..

clean: .SYMBOLIC
    cd spl1b   && wmake -f SPL1B_MAKEFILE clean && cd ..
    cd pmspl   && wmake -f PMSPL_MAKEFILE clean && cd ..
    cd spoolcp && wmake -f SPOOLCP_MAKEFILE clean && cd ..
    cd spool   && wmake -f SPOOL_MAKEFILE clean && cd ..
    cd pmspool && wmake -f PMSPOOL_MAKEFILE clean && cd ..
