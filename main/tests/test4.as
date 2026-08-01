; this test is meant to show errors in pre assembly

; the next line is longer than 80
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

mcro HELLO
 im going to redeclare this macro later
ncroend

mcro HELLO
 and here I have done it
mcroend

mcro jmp
 this one has the same name as a command
mcroend

mcro .extern
 this one is also invalid
mcroend

mcro smth a
 this one is wrong because there is something after the decleration
mcroend

mcro smth2
 a
mcroend a

mcro 1smth
 cant have names starting with a number
mcroend

; that is it for the pre assembly