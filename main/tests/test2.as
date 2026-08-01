; this should just be ignored and not appear in .am file
.entry LIST
.extern W
MAIN: add $3, $4, $5
mcro GEN_MC
add $1, $2, $3
move $3, $4
mcroend
LOOP: addi $1, 48, $2
GEN_MC
sub $1, $2, $4
bne $1, $2, END
mcro GEN_MC2
add $1, $2, $3
move $4, $5
mcroend
GEN_MC
beq $1, $2, END
jmp LOOP
GEN_MC
GEN_MC2
GEN_MC2
END: hlt
STR: .asciz "abcd"
LIST: .dw 6, -9
 .dw -100
.entry K
K: .dw 31
.extern val
