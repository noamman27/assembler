#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lib/utils.h"   /* nlist, hash, lookup, install, RCommand etc.    */

/* ── Constants ── */
#define MAXLINE   81        /* max chars per line (80 + null terminator)       */
#define IC_START  100       /* instruction counter starts at 100 per spec      */
#define WORD      4         /* bytes in a .dw value                            */
#define HALF_WORD 2         /* bytes in a .dh value                            */
#define REG      -1         /* getparam: operand was a register                */
#define SYM      -2         /* getparam: operand was a symbol/label            */
#define IMMED    -3         /* getparam: operand was an immediate value        */

/*max sizes*/
#define MAX_HALF_WORD 32767
#define MIN_HALF_WORD -32768
#define MAX_BYTE 127
#define MIN_BYTE -128
#define MAX_WORD 2147483647
#define MIN_WORD (-2147483647 - 1)

/* ── Error macro ── */
#define err(s) fprintf(stderr, "error in line %d: %s\n",lc ,(s))

/* ── Bit fields ── */

/* J-type instruction — 32 bits total */
typedef struct{
    unsigned int address: 25; /* absolute target address or register number   */
    unsigned int reg:      1; /* 1=operand is a register, 0=it is an address  */
    unsigned int opcode:   6; /* operation code                               */
} J_BF;

/* I-type instruction — 32 bits total */
typedef struct{
    unsigned int immed:  16; /* immediate value or signed branch offset        */
    unsigned int rt:      5; /* second register (dest for arith, src for branch)*/
    unsigned int rs:      5; /* first source register                         */
    unsigned int opcode:  6; /* operation code                                */
} I_BF;

/* R-type instruction — 32 bits total */
typedef struct{
    unsigned int unused:  6; /* always zero                                   */
    unsigned int funct:   5; /* function code — identifies which R instruction*/
    unsigned int rs:      5; /* first source register                         */
    unsigned int rd:      5; /* destination register                          */
    unsigned int rt:      5; /* second source register                        */
    unsigned int opcode:  6; /* operation code (0 or 1 for R-type)           */
} R_BF;

/* ── Function declarations ── */

/* pre-assembler/pre-assembler.c */
int     pre_assemble(FILE *f, FILE *write, char *name);

/* first-pass/first-pass.c */
int     first_pass(FILE *f, char *name, macro *macrotab[]);

/* second-pass/second-pass.c */
int     second_pass(FILE *input, char *basename, int *code_image, int icf, int dcf, symbol *symboltab, char *data_image);

/* lib/utils.c — command helpers */
int     gettype(char *s, char *t);
int     isR(char *s);
int     isI(char *s);
int     isJ(char *s);
int     isnum(char *s);
int     count_params(char *s);
int     isarithorlog(char *s);
int     iscond(char *s);
int     isloading(char *s);
int     getopcode(char *s);
int     getfunct(char *s);
void    remove_quotes(char *s);

#endif /* ASSEMBLER_H */