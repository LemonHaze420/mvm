# int v1 = 1;
PUSH 1 
# int v2 = 2;
PUSH 2
# print(v1 + v2);
ADD
PRINT

# int v3 = 4;        
PUSH 4
# int v4 = 4;
PUSH 4
# if(v3 == v4) print(1);
EQU
JMPZ 26
PRINT

# exit(0);        
HALT