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
        if (np == NULL || (np->name = dupstr(name)) == NULL){
            return NULL;
        }
        np->defn = dupstr(defn != NULL ? defn : "");
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
        if((np->defn = dupstr(defn)) == NULL){
            return NULL;
        }
    }
    return np;
}

int add_symbol(const char *name, int value, char *attribute, symbol **symboltab){
    symbol *s;

    if(lookup_symbol((char *)name, *symboltab)){                              
        fprintf(stderr, "error: symbol '%s' already defined\n", name);                                  
        return 0;
    }              
    s = malloc(sizeof(*s));
    if(!s){                                               
        fprintf(stderr, "error: malloc failed\n");                                       
        return 0;                                        
    }
    s->name  = dupstr(name);  
    if(s->name == NULL){
        free(s);
        fprintf(stderr, "error: malloc failed");
        return 0;
    }
    s->value = value;
    s->attribute = attribute;
    s->next = *symboltab;
    *symboltab = s;
    return 1;
}

symbol *lookup_symbol(char *name, symbol *symboltab){
    symbol *s = symboltab;               /* start at head of list */
    while(s){                         /* walk the list */
        if(strcmp(s->name, name) == 0){
           return s; /* found - return pointer to node */ 
        }
        s = s->next;                  /* move to next node */
    }
    return NULL;                      /* name not found */
}

void update_symbols(int icf , symbol *symboltab){
    symbol *s = symboltab;               /* start at head of list */
    while(s){                         /* walk every symbol */
        if(strcmp(s->attribute, "data") == 0){       /* update data symbols */
            s->value += icf;          /* shift value by ICF so it points to correct memory location */
        }
        s = s->next;                  /* move to next node */
    }
}