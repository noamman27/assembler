#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

/*returns a hashed version of s in hashtab*/
unsigned hash(char *s, nlist *hashtab[]){
    unsigned hashval;

    for(hashval = 0; *s != '\0'; s++){
        hashval = *s + 31 *hashval;
    }
    return hashval % HASHSIZE;
}
/*looks up s in hashtab. returns a pointer to s on success, and pointer to NULL of failure*/
nlist *lookup(char *s, nlist *hashtab[]){
    nlist *np;
    for(np = hashtab[hash(s,hashtab)]; np != NULL; np = np->next){
        if(strcmp(s, np->name) == 0){
            return np;
        }
    }
    return NULL;
}
/*adds an nlist object in hashtab with name and defn. returns a pointer to it in success and pointer to NULL of failure*/
nlist *install(char *name, char *defn, nlist *hashtab[]){
    nlist *np;
    unsigned hashval;
    if((np = lookup(name, hashtab)) == NULL){
        np = (nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL){
            return NULL;
        }
        np->defn = strdup(defn != NULL ? defn : "");
        if(np->defn == NULL){
            free(np->name);
            free(np);
            return NULL;
        }
        hashval = hash(name, hashtab);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    }
    else {
        free((void *) np->defn);
        if((np->defn = strdup(defn)) == NULL){
            return NULL;
        }
    }
    return np;
}