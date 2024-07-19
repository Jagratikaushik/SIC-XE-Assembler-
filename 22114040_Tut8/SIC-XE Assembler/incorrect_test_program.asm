HEAD    START   0
FIRST   LDT     #11
        LDX     #0
MOVECH  LDC    STR1,X
        STCH    STR1,X
        TIXR    T
        JL     MOVECH
STR1    BYTE    C'TEST STRING'
STR2    RESB    11      
        END     FIRST