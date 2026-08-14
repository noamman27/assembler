#ifndef UTILS_H
#define UTILS_H

/*macro struct*/
typedef struct macro {
    char *name;
    char *defn;
    struct macro *next;
} macro;

/*symbol struct*/
typedef struct symbol {
    char *name;
    char *attribute;
    int value;
    struct symbol *next;
} symbol;

/*make it so symbol can be written with a lowercase or uppercase s*/
typedef symbol Symbol;

/*macro utils - table.c*/
macro *lookup_macro(char *s, macro *macrotab);
macro *install_macro(char *name, char *defn, macro **hashtab);
void free_macros(macro *macrotab);

/*symbol utils - table.c*/
int add_symbol(const char *name, int value, char *attribute, symbol **symboltab);
symbol *lookup_symbol(char *name, symbol *symboltab);
void free_symbols(symbol *symboltab);
void update_symbols(int icf , symbol *symboltab);

/*command utils - utils.c*/
int gettype(char *s, char *t);
int isR(char *s);
int isI(char *s);
int isJ(char *s);
int isnum(char *s);
int getparams(char line[], int *lp, char *params[], int types[], int lc);
int isarithorlog(char *s);
int iscond(char *s);
int isloading(char *s);
int getopcode(char *s);
int getfunct(char *s);

/*input utils - input.c*/
int getword(char word[], char line[], int *lp);
int lineend(char *s);
int getword(char word[], char line[], int *lp);
int getparam(char line[], int *lp, char sym[], int *immed, int lc);
int getch(char buffer[], char *ch, int *lp);
int ungetch(char buffer[], char c, int *lp);
char *dupstr(const char *s);
void remove_quotes(char *s);

/*command structs*/
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

/*command lists*/
extern RCommand rCommands[];
extern ICommand iCommands[];
extern JCommand jCommands[];

#endif