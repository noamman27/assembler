#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
 
static Symble *symbletab = NULL; /* head of the symbol table linked list */
char *code_image, *data_image; /*code image and data image*/
static int cp = 0, lp = 0, IC = IC_START, DC = 0, ICF, DCF; /*code pointer, data pointer, line pointer, IC and DC*/

int first_pass(FILE *input){
    char line[MAXLINE], word[MAXLINE], sym[MAXLINE], *tmp, type;
    int isSym = 0, len, i, reg, error = 0;
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    code_image = malloc(0);
    data_image = malloc(0);
    lp = 0;
    while(fgets(line, MAXLINE, input) != NULL){
        if(line[0] == ';' || ((len = getword(word, line)) == 0)){
            continue;
        }
        lp = len;
        /*check if last char of word is :, if so its a label*/
        if(word[len-1] == ':'){ 
            word[len-1] = '\0';
            if(lookup_symble(word, NULL, symbletab)){
                fprintf(stderr, "error: label %s is already defined", word);
            }
            if(lookup(word, macrotab)){
                err("error: a label cannot have the same name as a macro");
            }
            isSym = 1;
            *sym = word;
            getword(word, line);
        }
        /*handle data instructions*/
        if(strcmp(word, ".dh") == 0 ) {
            DC +=HALF_WORD;
            tmp = realloc(data_image, DC);
            if(!tmp){
                err("realloc error");
                error = 1;
            }
            data_image = tmp;
            if(isSym){
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            for(i = DC - HALF_WORD; i<=DC; i++){
                if(getword(word, line) == 1){
                    data_image[i] = word;
                }
                err("error");
            }
            continue;
        }
        else if(strcmp(word, ".db") == 0){
            DC+= 1;
            tmp = realloc(data_image, DC);
            if(!tmp){
                err("realloc error");
                error = 1;
            }
            data_image = tmp;
            if(isSym){
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            if(getword(word, line) == 1){
                data_image[i] = word;
            }
        }
        else if(strcmp(word, ".dw") == 0){
            DC+= WORD;
            tmp = realloc(data_image, DC);
            if(!tmp){
                err("realloc error");
                error = 1;
            }
            data_image = tmp;
            if(isSym){
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            for(i = DC - WORD; i<=DC; i++){
                if(getword(word, line) == 1){
                    data_image[i] = word;
                }
                err("input error");
            }
        }
        else if(strcmp(word, ".asciz") == 0){
            i = len;
            if(isSym){
                DC+=len;
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            while (getword(word, line))
            {
                if(isspace(line[i])){
                    tmp = realloc(data_image, DC);
                    if(!tmp){
                        err("realloc error");
                        error = 1;
                    }
                    data_image = tmp;
                    data_image[DC] = (char) atoi(word);
                }
                DC++;
                tmp = realloc(data_image, DC);
                if(!tmp){
                    err("realloc error");
                    error = 1;
                }
                data_image = tmp;
                data_image[DC] = '\0';
            }
            tmp = realloc(data_image, DC);
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
            add_symble(word, 0, "external", symbletab);
        }
        if(!gettype(word,&type)){
            err("error: command not recognized");
        }
        /*handle encoding of commands*/
        switch (type)
        {
        case 'r':
            if(param_count(word) == 2){
                rc_bf.rs = 0;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a symble cannot be given as a parameter to a move command");
                    error = 1;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
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
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rs = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    continue;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
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
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a label cannot be given as a parameter to an arithmatric or logical command");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(getparam(line, &lp, tmp, &i) != IMMED){
                    err("error: no immediate value given to i command");
                    error = 1;
                }
                ic_bf.immed = i;
                if(!(reg = getparam(line, &lp, tmp, &i))){
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
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: the first and second parameters of a conditional command must be registers");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: the first and second parameters of a conditional command must be registers");
                    error = 1;
                }
                ic_bf.rt = reg - 1;
                if(getparam(line, &lp, tmp, &i) != SYM){
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
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    err("error: a label cannot be given as a parameter to an arithmatric or logical command");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(getparam(line, &lp, tmp, &i) != IMMED){
                    err("error: no immediate value given to i command");
                    error = 1;
                }
                if(i < -16 || i > 16){
                    err("error: cannot offset by more than 16 bits");
                    error = 1;
                }
                ic_bf.immed = i;
                if(!(reg = getparam(line, &lp, tmp, &i))){
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
                    if(!(reg = getparam(line, &lp, tmp, &i))){
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
            else if(strcmp(word, "la") == 0){
                    if(!(reg = getparam(line, &lp, tmp, &i))){
                        error = 1;
                    }
                    if(reg != SYM){
                        err("error: no label given to la");
                        error = 1;
                    }
                    jc_bf.opcode = 31;
                    jc_bf.reg = 0;
                }
            else if(strcmp(word, "call") == 0){
                    if(!(reg = getparam(line, &lp, tmp, &i))){
                        error = 1;
                    }
                    if(reg != SYM){
                        err("error: no label given to call");
                        error = 1;
                    }
                    jc_bf.opcode = 32;
                    jc_bf.reg = 0;
                }
            else if(strcmp(word, "halt") == 0){
                jc_bf.opcode = 63;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            break;
        default:
            break;
        }
        IC+=4;
    }
    if(error){
        err("errors detected in first pass - assembly will not continue");
        return 0;
    }
    ICF = IC;
    DCF = DC;
    return 1;
}
