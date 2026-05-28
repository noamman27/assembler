#define HASHSIZE 101

typedef struct {
    struct nlist *next;
    char *name;
    char *defn;
} nlist;

unsigned hash(char *s, nlist *hashtab[]);
nlist *lookup(char *s, nlist *hashtab[]);
nlist *install(char *name, char *defn, nlist *hashtab[]);