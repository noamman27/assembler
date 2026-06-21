#define HASHSIZE 101

typedef struct nlist {
    char *name;
    char *defn;
    struct nlist *next;
} nlist;

unsigned hash(char *s, nlist *hashtab[]);
nlist *lookup(char *s, nlist *hashtab[]);
nlist *install(char *name, char *defn, nlist *hashtab[]);
int add_symble(const char *name, int value, char *attribute, Symble *symbletab);
int lookup_symble(const char *name, Symble **sp, Symble *symbletab);
int getword(char word[], char line[]);
int gettype(char *s, char *t);
int isR(char *s);
int isI(char *s);
int isJ(char *s);
int isnum(char *s);
int getparam(char line[], int *lp, char sym[], int *immed);
int isarithorlog(char *s);
int iscond(char *s);
int isloading(char *s);
int getopcode(char *s);
int getfunct(char *s);

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