#include "../lib/utils.h"

#define MAXLINE 81
#define MAX_DATA 256
#define MAX_CODE 256
#define IC_START 100
#define WORD 4
#define HALF_WORD 2
#define SYM -1
#define IMMED -2
#define err(s) fprintf(stderr, s)
static int lp;
static nlist *macrotab[HASHSIZE];
int pre_assemble(FILE *f, FILE *write);

typedef struct Symble {
    char *name;
    int value;
    char *attribute;
    struct Symble *next;
} Symble;

typedef struct{
    unsigned int address: 25;
    unsigned int reg: 1;
    unsigned int opcode: 6;
} J_BF;
typedef struct{
    unsigned int immed: 16;
    unsigned int rs: 5;
    unsigned int rt: 5;
    unsigned int opcode: 6;
} I_BF;
typedef struct{
    unsigned int unused: 6;
    unsigned int funct: 5;
    unsigned int rs: 5;
    unsigned int rd: 5;
    unsigned int rt: 5;
    unsigned int opcode: 6;
} R_BF;