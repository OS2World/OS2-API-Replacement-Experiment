wcc386 -bm -bt=OS2 -6s -fpi87 -fp6 -sg -otexanr -wx -fo=BDCALL32.OBJ BDCALL32.C 2>&1 |tee make.out
wlink @BDCALL32.LNK 2>&1 |tee link.out