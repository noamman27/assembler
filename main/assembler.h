#define MAXLINE 81
#define IC_START 100
int pre_assemble(FILE *f, FILE *write);

typedef struct{
    unsigned int address: 25;
    unsigned int reg: 1;
    unsigned int opcode: 6;
} J_BF;
typedef struct{
    unsigned int immed: 16;
    unsigned int rs: 5;
    unsigned int rd: 5;
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