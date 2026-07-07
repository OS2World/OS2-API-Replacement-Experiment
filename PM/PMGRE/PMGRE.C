/* PMGRE.C -- cWarp PMGRE clone: pure LX forwarder to PMMERGE (real type-0x04
 * forwarders via IMPORT+EXPORT in the .LNK -- ZERO thunk code, bitness preserved,
 * exactly like the original (same method as the working PMDRAG clone). This file
 * only supplies a DLL init entry / object for the linker. */
unsigned long _System _DLL_InitTerm(unsigned long hmod,unsigned long flag);
#pragma aux _DLL_InitTerm "_DLL_InitTerm";
unsigned long _System _DLL_InitTerm(unsigned long hmod,unsigned long flag)
{ (void)hmod;(void)flag; return 1UL; }
