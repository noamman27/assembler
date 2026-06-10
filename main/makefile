pre-assemble: main.o table.o utils.o pre-assembler.o
	gcc -g main.o table.o utils.o pre-assembler.o -o pre-assemble
main.o table.o utils.o pre-assembler.o: main.c ../lib/table.c ../lib/utils.c ../pre-assembler/pre-assembler.c
	gcc -c main.c ../lib/table.c ../lib/utils.c ../pre-assembler/pre-assembler.c