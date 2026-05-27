#define HASHSIZE 101

typedef struct {
    nlist *next;
    char *name;
    char *defn;
} nlist;

static nlist *hashtab[HASHSIZE];