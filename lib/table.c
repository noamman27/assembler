#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "../main/assembler.h"

/*looks up s in macrotab. returns a pointer to s on success, and pointer to NULL of failure*/
macro *lookup_macro(char *s, macro *macrotab){
    macro *mp = macrotab;
    while(mp){
        if(strcmp(mp->name, s) == 0){
            return mp;
        }
        mp = mp->next;
    }
    return NULL;
}
/*adds an macro object in macrotab with name and defn. returns a pointer to it in success and pointer to NULL of failure*/
macro *install_macro(char *name, char *defn, macro **macrotab){
    macro *mp;

    if(lookup_macro(name, *macrotab)){
        fprintf(stderr, "error: macro '%s' already defined\n", name);
        return 0;
    }
    mp = malloc(sizeof(macro));
    if(!mp){
        fprintf(stderr, "error: malloc failed\n");
        exit(1);     
    }
    mp->name = dupstr(name);
    if(!mp->name){
        free(mp);
        fprintf(stderr, "error: malloc failed");
        exit(1);
    }
    mp->defn = dupstr(defn);
    if(!mp->defn){
        free(mp->name);
        free(mp);
        fprintf(stderr, "error: malloc failed");
        exit(1);
    }
    mp->next = *macrotab;
    *macrotab = mp;
    return mp;
}
/*frees the list of macros starting from macrotab, not including the head of the list*/
void free_macros(macro *macrotab){
    macro *next;

    while(macrotab){
        next = macrotab->next;
        free(macrotab->name);
        free(macrotab->defn);
        free(macrotab);
        macrotab = next;
    }
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
        exit(1);                                       
    }
    s->name  = dupstr(name);  
    if(s->name == NULL){
        free(s);
        fprintf(stderr, "error: malloc failed");
        exit(1);
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

/*frees the list of symbols starting from symboltab, not including the head of the list*/
void free_symbols(symbol *symboltab){
    symbol *next;

    while(symboltab){
        next = symboltab->next;
        free(symboltab->name);
        free(symboltab);
        symboltab = next;
    }
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