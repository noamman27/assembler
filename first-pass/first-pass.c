#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
 
static Symble *symbletab = NULL; /* head of the symbol table linked list */
char *code_image, *data_image;
static int cp = 0, dp = 0, IC = IC_START, DC = 0; /*code pointer*/

int first_pass(FILE *input, FILE *output){
    char line[MAXLINE], word[MAXLINE], *tmp, type;
    int isSym = 0, len, i, reg;
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    code_image = NULL;
    data_image = NULL;
    lp = 0;
    while(fgets(line, MAXLINE, input) != NULL){
        if(line[0] == ';' || ((len = getword(word, line)) == 0)){
            continue;
        }
        lp = len;
        if(word[len-1] == ':'){ /*check if last char of word is :, if so its a label*/
            word[len-1] = '\0';
            if(lookup_symble(word, NULL)){
                fprintf(stderr, "error: label %s is already defined", word);
            }
            if(lookup(word, macrotab)){
                err("error: a label cannot have the same name as a macro");
            }
            isSym = 1;
            continue;
        }
        /*handle data instructions*/
        if(strcmp(word, ".dh") == 0 ) {
            if(isSym){
                add_symble(word, DC, "data");
            }
            dp +=HALF_WORD;
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
            for(i = dp - HALF_WORD; i<=dp; i++){
                if(getword(word, line) == 1){
                    data_image[i] = word;
                }
                err("error");
            }
        }
        else if(strcmp(word, ".db") == 0){
            dp+= 1;
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
            if(getword(word, line) == 1){
                data_image[i] = word;
            }
        }
        else if(strcmp(word, ".dw") == 0){
            dp+= WORD;
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
            for(i = dp - WORD; i<=dp; i++){
                if(getword(word, line) == 1){
                    data_image[i] = word;
                }
                err("input error");
            }
        }
        else if(strcmp(word, ".asciz") == 0){
            i = len;
            while (getword(word, line))
            {
                if(isspace(line[i])){
                    tmp = realloc(data_image, dp);
                    if(!tmp){
                        err("realloc error");
                        return 0;
                    }
                    data_image = tmp;
                    data_image[dp] = (char) atoi(word);
                }
                dp++;
                tmp = realloc(data_image, dp);
                if(!tmp){
                    err("realloc error");
                    return 0;
                }
                data_image = tmp;
                data_image[dp] = '\0';
            }
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                return 0;
            }
            data_image = tmp;
        }
        if(strcmp(word, ".entry") == 0){
            continue;
        }
        if(strcmp(word, ".extern") == 0){
            if(!getword(word, line)){
                err("error: no symble given as parameter for .extern");
                return 0;
            }
            if(!validSym(word)){
                err("symble given as parameter for .extern isnt valid");
                return 0;
            }
            add_symble(word, 0, "external");
        }
        if(!gettype(word,&type)){
            err("error: command not recognized");
        }
        switch (type)
        {
        case 'r':
            if(param_count(word) == 2){
                rc_bf.rs = 0;
                if(!(reg = getparam(line, lp, tmp, i))){
                    return 0;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    return 0;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rt = reg - 1;
                if(!(rc_bf.opcode = getopcode(word))){
                    fprintf(stderr,"input error");
                    return 0;
                }
            } else {
                if(!(reg = getparam(line, lp, tmp, i))){
                    return 0;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rs = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    return 0;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    return 0;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rt = reg - 1;
                if(!(rc_bf.opcode = getopcode(word))){
                    /*we really shouldnt get here since we made sure this was a command so it was most likely an issue with getopcode which is reported within the function*/
                    return 0;
                }
            }
            break;
        case 'i':
            if(!(reg = getparam(line, lp, tmp, i))){
                return 0;
            }
            if(reg == SYM){
                continue;
            }
            ic_bf.rs = reg - 1;
            if(getparam(line, lp, tmp, i) != IMMED){
                err("error: no immediate value given to i command");
                return 0;
            }
            ic_bf.immed = i;
            if(!(reg = getparam(line, lp, tmp, i))){
                return 0;
            }
            if(reg == SYM){
                continue;
            }
            ic_bf.rs = reg - 1;
            if(!(ic_bf.opcode = getopcode(word))){
                /*we really shouldnt get here since we made sure this was a command so it was most likely an issue with getopcode which is reported within the function*/
                return 0;
            }
            break;
        case 'j':
            break;
        default:
            break;
        }

    }
    return 1;
}


int lookup_symble(const char *name, Symble **sp){
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

int add_symble(const char *name, int value, char *attribute){
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
        fprintf(stderr, "Error: malloc failed\n");
        return 0;
    }
    s->value = value;         
    s->attribute  = attribute;          
    s->next  = symbletab;   
    symbletab   = s; 
    return 1;                
}