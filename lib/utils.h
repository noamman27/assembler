#define HASHSIZE 101

typedef struct nlist {
    char *name;
    char *defn;
    struct nlist *next;
} nlist;

unsigned hash(char *s, nlist *hashtab[]);
nlist *lookup(char *s, nlist *hashtab[]);
nlist *install(char *name, char *defn, nlist *hashtab[]);
int gettype(char *s, char *t);
int isR(char *s);
int isI(char *s);
int isJ(char *s);

typedef struct {
    char *name;
    int opcode;
    int funct;
} RCommand;

typedef struct {
    char *name;
    int opcode;
} ICommand;

typedef struct {
    char *name;
    int opcode;
} JCommand;

extern RCommand rCommands[];
extern ICommand iCommands[];
extern JCommand jCommands[];