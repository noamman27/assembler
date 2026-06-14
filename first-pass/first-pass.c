#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
 
static Symble *symbletab = NULL; /* head of the symbol table linked list */
char *code_image;
char *data_image;
static int cp = 0; /*code pointer*/
static int dp = 0; /*data pointer*/
static int IC = IC_START;
static int DC = 0;

int first_pass(FILE *input, FILE *output){
    char line[MAXLINE], word[MAXLINE], *tmp, type;
    int isSym = 0, len;
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    code_image = NULL;
    data_image = NULL;
    while(fgets(line, MAXLINE, input) != NULL){
        if(line[0] == ';' || ((len = getword(word, line)) == 0)){
            continue;
        }
        if(word[len-1] == ':'){ /*check if last char of word is :, if so its a label*/
            word[len-1] = '\0';
            if(lookup_symbol(word, NULL)){
                fprintf(stderr, "label %s is already defined", word);
            }
            if(lookup(word, macrotab)){
                err("ERROR: a label cannot have the same name as a macro");
            }
            isSym = 1;
        }
        if(strcmp(word, ".dh") == 0 ) {
            dp +=2;
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
        }
        else if(strcmp(word, ".db") == 0){
            dp+= 1;
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
        }
        else if(strcmp(word, ".dw") == 0){
            dp+= 4;
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
        }
        if(!gettype(word,&type)){
            err("ERROR: command not recognized");
        }
        switch (type)
        {
        case 'r':
            if(param_count(word) == 2){
                rc_bf.rs = 0;
                if(!(rc_bf.rd = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.rt = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.funct = getfunct(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.opcode = getopcode(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
            } else {
                if(!(rc_bf.rd = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;                    
                }
                if(!(rc_bf.rd = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.rt = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.funct = getfunct(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.opcode = getopcode(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
            }
            break;
        default:
            break;
        }

    }
    return 1;
}


int lookup_symbol(const char *name, Symble **sp){
    Symble *s = symbletab;

    while(s){
        if(strcmp(s->name, name) == 0){
            if(sp != NULL){
                *sp = s;
            }
            return 1;
        }
        s = s->next;
    }
    return 0;
}

int add_symbol(const char *name, int value, int attribute, int *err_flag){
    Symble *existing = NULL;

    if(lookup_symbol(name, &existing)){                              
        fprintf(stderr, "Error: symbol '%s' already defined\n", name);
        *err_flag = 1;                                    
        return 0;               
    }                          
    Symble *s = malloc(sizeof(*s));
    if(!s){                                               
        fprintf(stderr, "Error: malloc failed\n");        
        *err_flag = 1;                                    
        return 0;                                        
    }
    s->name  = strdup(name);  
    if(s->name == NULL){
        free(s);
        fprintf(stderr, "Error: malloc failed\n");
        *err_flag = 1;
        return 0;
    }
    s->value = value;         
    s->attribute  = attribute;          
    s->next  = symbletab;        
    symbletab   = s; 
    return 1;                
}