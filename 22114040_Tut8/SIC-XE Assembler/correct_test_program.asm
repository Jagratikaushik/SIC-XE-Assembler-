HEAD    START   0
FIRST   LDT     #11
        LDX     #0
MOVECH  LDCH    STR1,X
        STCH    STR2,X
        TIXR    T
        JLT     MOVECH
STR1    BYTE    C'TEST STRING'
STR2    RESB    11      
        END     FIRST