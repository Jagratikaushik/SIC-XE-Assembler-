Line	Address	Label	OPCODE	OPERAND	ObjectCode	Comment
5	00000	0	HEAD	START	0		
10	00000	0	FIRST	LDT	#11	75000B	
15	00003	0		LDX	#0	050000	
20	00006	0	MOVECH	LDC	STR1,X		
25	00006	0		STCH	STR1,X	57A002	
30	00009	0		TIXR	T	B850	
35	0000B	0		JL	MOVECH		
40	0000B	0	STR1	BYTE	C'TEST STRING'	5445535420535452494E47	
45	00016	0	STR2	RESB	11		
50	00021	 		END	FIRST		
