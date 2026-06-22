pre-assemble: main.o table.o utils.o input.o pre-assembler.o first-pass.o
	gcc -g main.o table.o utils.o input.o pre-assembler.o first-pass.o -o pre-assemble
main.o table.o utils.o input.o pre-assembler.o first-pass.o: main.c ../lib/table.c ../lib/utils.c ../lib/input.c ../pre-assembler/pre-assembler.c ../first-pass/first-pass.c
	gcc -c main.c ../lib/input.c  ../lib/table.c ../lib/utils.c ../pre-assembler/pre-assembler.c ../first-pass/first-pass.c