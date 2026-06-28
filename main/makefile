pre-assemble: main.o table.o utils.o input.o pre-assembler.o first-pass.o second-pass.o
	gcc -g main.o table.o utils.o input.o pre-assembler.o first-pass.o second-pass.o -o pre-assemble
main.o table.o utils.o input.o pre-assembler.o first-pass.o second-pass.o: main.c ../lib/table.c ../lib/utils.c ../lib/input.c ../pre-assembler/pre-assembler.c ../first-pass/first-pass.c ../second-pass/second-pass.c
	gcc -c main.c ../lib/input.c  ../lib/table.c ../lib/utils.c ../pre-assembler/pre-assembler.c ../first-pass/first-pass.c ../second-pass/second-pass.c