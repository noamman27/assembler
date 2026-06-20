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
    int isSym = 0, len, i, reg, error = 0;
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
                error = 1;
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
                error = 1;
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
                error = 1;
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
                        error = 1;
                    }
                    data_image = tmp;
                    data_image[dp] = (char) atoi(word);
                }
                dp++;
                tmp = realloc(data_image, dp);
                if(!tmp){
                    err("realloc error");
                    error = 1;
                }
                data_image = tmp;
                data_image[dp] = '\0';
            }
            tmp = realloc(data_image, dp);
            if(!tmp){
                err("realloc error");
                error = 1;
            }
            data_image = tmp;
        }
        /*handle .entry and .extern*/
        if(strcmp(word, ".entry") == 0){
            continue;
        }
        if(strcmp(word, ".extern") == 0){
            if(!getword(word, line)){
                err("error: no symble given as parameter for .extern");
                error = 1;
            }
            if(!validSym(word)){
                err("symble given as parameter for .extern isnt valid");
                error = 1;
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
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a symble cannot be given as a parameter to a move command");
                    error = 1;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a symble cannot be given as a parameter to a move command");
                    error = 1;
                }
                rc_bf.rt = reg - 1;
                rc_bf.opcode = 1;
                rc_bf.funct = getfunct(word);
            } else {
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rs = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rt = reg - 1;
                rc_bf.opcode = 0;
                rc_bf.funct = getfunct(word);
            }
            break;
        case 'i':
            if(isarithorlog(word)){
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a label cannot be given as a parameter to an arithmatric or logical command");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(getparam(line, lp, tmp, i) != IMMED){
                    err("error: no immediate value given to i command");
                    error = 1;
                }
                ic_bf.immed = i;
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a label cannot be given as a parameter to an arithmatric or logical command");
                    error = 1;
                }
                ic_bf.rt = reg - 1;
                if(!(ic_bf.opcode = getopcode(word))){
                    /*we really shouldnt get here since we made sure this was a command so it was most likely an issue with getopcode which is reported within the function*/
                    error = 1;
                }
            }
            if(iscond(word)){
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: the first and second parameters of a conditional command must be registers");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: the first and second parameters of a conditional command must be registers");
                    error = 1;
                }
                ic_bf.rt = reg - 1;
                if(getparam(line, lp, tmp, i) != SYM){
                    err("error: no immediate value given to i command");
                    error = 1;
                }
                /*prolly need to add tmp to code image*/
                if(!(ic_bf.opcode = getopcode(word))){
                    /*we really shouldnt get here since we made sure this was a command so it was most likely an issue with getopcode which is reported within the function*/
                    error = 1;
                }
            }
            if(isloading(word)){
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a label cannot be given as a parameter to an arithmatric or logical command");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(getparam(line, lp, tmp, i) != IMMED){
                    err("error: no immediate value given to i command");
                    error = 1;
                }
                if(i < -16 || i > 16){
                    err("error: cannot offset by more than 16 bits");
                    error = 1;
                }
                ic_bf.immed = i;
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a label cannot be given as a parameter to an arithmatric or logical command");
                    error = 1;
                }
                ic_bf.rt = reg - 1;
                if(!(ic_bf.opcode = getopcode(word))){
                    /*we really shouldnt get here since we made sure this was a command so it was most likely an issue with getopcode which is reported within the function*/
                    error = 1;
                }
            }
            break;
        case 'j':
            if(strcmp(word, "jmp") == 0){
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg == SYM){
                    jc_bf.opcode = 30;
                    jc_bf.reg = 0;
                }
                else if(reg != IMMED){
                    jc_bf.opcode = 30;
                    jc_bf.reg = 1;
                    jc_bf.address = reg-1;
                }
            }
            if(strcmp(word, "la") == 0){
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg != SYM){
                    err("error: no label given to la");
                    error = 1;
                }
                jc_bf.opcode = 31;
                jc_bf.reg = 0;
            }
            if(strcmp(word, "call") == 0){
                if(!(reg = getparam(line, lp, tmp, i))){
                    error = 1;
                }
                if(reg != SYM){
                    err("error: no label given to call");
                    error = 1;
                }
                jc_bf.opcode = 32;
                jc_bf.reg = 0;
            }
            if(strcmp(word, "call") == 0){
                jc_bf.opcode = 63;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            break;
        default:
            break;
        }
    }
    if(error){
        return 0;
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