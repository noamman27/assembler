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
#define SYM      -1         /* getparam: operand was a symbol/label            */
#define IMMED    -2         /* getparam: operand was an immediate value        */

/* ── Error macro ── */
#define err(s) fprintf(stderr, "%s\n", (s))

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

/* ── Symbol table entry ── */
typedef struct Symble {
    char          *name;      /* label name string                            */
    int            value;     /* address of the symbol in memory              */
    char          *attribute; /* "code", "data", "external", or "entry"       */
    struct Symble *next;      /* next node in the linked list                 */
} Symble;

/* ── Globals defined in first-pass.c ──────────────────────────────────────
   NOTE: in first-pass.c the following must NOT be declared static:
     Symble *symbletab   (remove 'static')
     int ICF, DCF        (remove 'static' from their declaration)            */
extern int    *data_image; /* unified memory image                   */
extern int    *code_image; /* kept for compatibility                 */
extern int    IC;                   /* current instruction counter            */
extern int    DC;                   /* current data counter                   */
extern int    ICF;                  /* final IC after first pass completes    */
extern int    DCF;                  /* final DC after first pass completes    */
extern nlist *macrotab;
extern Symble *symbletab;           /* head of symbol table linked list       */

/* ── macrotab is defined as static inside pre-assembler.c only ──
   do NOT declare it here — each .c file that includes this header
   would get its own separate (empty) copy, breaking lookup/install          */

/* ── Function declarations ── */

/* pre-assembler/pre-assembler.c */
int     pre_assemble(FILE *f, FILE *write);

/* first-pass/first-pass.c */
int     first_pass(FILE *f);

/* second-pass/second-pass.c */
int     second_pass(FILE *input, char *basename);

/* lib/table.c — symbol table */
int     add_symble(const char *name, int value, char *attribute, Symble *symbletab);
Symble *lookup_symble(char *name, Symble *symbletab);
void    update_data_symbles(int icf, Symble *symbletab);

/* lib/input.c — input helpers */
int     getword(char word[], char line[], int *lp);
int     getparam(char line[], int *lp, char sym[], int *immed);
int     getch(char buffer[], char *ch, int *lp);
int     ungetch(char buffer[], char c, int *lp);

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