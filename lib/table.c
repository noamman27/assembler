#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "../main/assembler.h"

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

int add_symble(const char *name, int value, char *attribute, Symble *symbletab){
    Symble *existing = NULL;

    if(lookup_symbol(name, &existing)){                              
        fprintf(stderr, "Error: symbol '%s' already defined\n", name);                                  
        return 0;
    }              
    Symble *s = malloc(sizeof(*s));
    if(!s){                                               
        fprintf(stderr, "Error: malloc failed\n");                                       
        return 0;                                        
    }
    s->name  = strdup(name);  
    if(s->name == NULL){
        free(s);
        err("error: malloc failed");
        return 0;
    }
    s->value = value;
    s->attribute = attribute;
    s->next = symbletab;
    symbletab = s;
    return 1;
}

Symble *lookup_symble(char *name, Symble *symbletab){
    Symble *s = symbletab;               /* start at head of list */
    while(s){                         /* walk the list */
        if(strcmp(s->name, name) == 0){
           return s; /* found - return pointer to node */ 
        }
        s = s->next;                  /* move to next node */
    }
    return NULL;                      /* name not found */
}

void update_data_symbols(int icf, Symble *symbletab){
    Symble *s = symbletab;               /* start at head of list */
    while(s){                         /* walk every symbol */
        if(strcmp(s->attribute, "data") == 0)       /* only update data symbols */
            s->value += icf;          /* shift value by ICF so it points to correct memory location */
        s = s->next;                  /* move to next node */
    }
}