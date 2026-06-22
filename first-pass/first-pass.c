#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static Symble *symbletab = NULL, *sp; /* head of the symbol table linked list */
int code_image[MAX_CODE], data_image[MAX_DATA]; /*code image and data image*/
static int cp = 0, lp = 0, IC = IC_START, DC = 0, ICF, DCF; /*code pointer, data pointer, line pointer, IC and DC*/

int first_pass(FILE *input){
    char line[MAXLINE], word[MAXLINE], sym[MAXLINE], *tmp, type;
    int isSym = 0, len, i, reg, error = 0;
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    while(fgets(line, MAXLINE, input) != NULL){ /*run while we can read more from file*/
        lp = 0; /*reset line pointer*/
        if(line[0] == ';' || ((len = getword(word, line,lp)) == 0)){ /*check if line is omment or empty; if so we ignore it*/
            continue;
        }
        lp = len; /*set line pointer to len since that is how much*/
        /*check if last char of word is :, if so its a label*/
        if(word[len-1] == ':'){ 
            word[len-1] = '\0';
            if(lookup_symble(word, NULL, symbletab)){
                fprintf(stderr, "error: label %s is already defined", word);
                error = 1;
                continue;
            }
            if(lookup(word, macrotab)){
                err("error: a label cannot have the same name as a macro");
                error = 1;
                continue;
            }
            if(gettype(word, tmp)){
                err("error: a label cannot habe the same name as a command");
                error = 1;
                continue;
            }
            isSym = 1;
            *sym = *word;
            getword(word, line, lp);
        }
        /*handle data instructions*/
        if(strcmp(word, ".dh") == 0 ) {
            if(!getword(word, line, lp)){
                err("error: no value given to .dh");
                error = 1;
                continue;
            }
            if(DC + HALF_WORD > MAX_DATA){
                err("error: data image is too large");
                error = 1;
                continue;
            }
            DC +=HALF_WORD;
            if(isSym){
                /*we already checked that the label is valid so we can just add it*/
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            getword(word, line, lp);
            data_image[DC] = word;
            continue;
        }
        else if(strcmp(word, ".db") == 0){
            if(!getword(word, line, lp)){
                err("error: no value given to .db");
                error = 1;
                continue;
            }
            if(DC + 1 > MAX_DATA){
                err("error: data image is too large");
                error = 1;
                continue;
            }
            DC+= 1;
            if(isSym){
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            data_image[DC] = word;
            
        }
        else if(strcmp(word, ".dw") == 0){
            if(!getword(word, line, lp)){
                err("error: no value given to .dw");
                error = 1;
                continue;
            }
            if(DC + WORD > MAX_DATA){
                err("error: data image is too large");
                error = 1;
                continue;
            }
            DC+= WORD;
            if(isSym){
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            data_image[DC] = word;
        }
        else if(strcmp(word, ".asciz") == 0){
            if(!(len = getword(word, line, lp))){
                err("error: no string given to .asciz");
                error = 1;
                continue;
            }
            i = len;
            if(isSym){
                DC+=len;
                add_symble(sym, DC, "data", symbletab);
                continue;
            }
            if(isnum(word)){
                if(DC + len > MAX_DATA){
                    err("error: data image is too large");
                    error = 1;
                    continue;
                }
                DC+= len;
                i = atoi(word);
                data_image[DC] = i;
                continue;
            }
            if(word[0] !='"' || word[len-1] != '"'){
                err("error: string given to .asciz is not valid");
                error = 1;
                continue;
            }
            remove_quotes(word);
            DC += len - 2;
            data_image[DC] = word;
        }
        /*handle .entry and .extern*/
        if(strcmp(word, ".entry") == 0){
            continue;
        }
        if(strcmp(word, ".extern") == 0){
            if(!getword(word, line, lp)){
                err("error: no symble given as parameter for .extern");
                error = 1;
                continue;
            }
            if(isdigit(word[0])){
                err("symble given as parameter for .extern isnt valid");
                error = 1;
                continue;
            }
            if(gettype(word, tmp)){
                err("error: a label cannot habe the same name as a command");
                error = 1;
                continue;
            }
            if(lookup_symble(word, &sp, symbletab) && strcmp(sp->attribute, "external")){
                fprintf(stderr,"error: label %s already defined not as external", word);
                error = 1;
                continue;
            }
            add_symble(word, 0, "external", symbletab);
        }
        if(!gettype(word,&type)){
            err("error: command not recognized");
        }
        if(isSym){
            add_symble(sym, IC, "code", symbletab);
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
                    rc_bf.rs = 0;
                    continue;
                }
                rc_bf.rs = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    rc_bf.rd = 0;
                    continue;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM){
                    rc_bf.rt = 0;
                    continue;
                }
                rc_bf.rt = reg - 1;
                rc_bf.opcode = 0;
                rc_bf.funct = getfunct(word);
            }
            memcpy(&data_image[IC], &rc_bf, sizeof(rc_bf)); /*add to code image*/
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
                if(reg == SYM || reg == IMMED){
                    err("error: the first and second parameters of a conditional command must be registers");
                    error = 1;
                }
                ic_bf.rs = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                }
                if(reg == SYM || reg == IMMED){
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
            memcpy(&data_image[IC], &ic_bf, sizeof(ic_bf)); /*add to code image*/
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
            else if(strcmp(word, "hlt") == 0){
                jc_bf.opcode = 63;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            memcpy(&data_image[IC], &jc_bf, sizeof(jc_bf)); /*add to code image*/
            break;
        default:
            break;
        }
        if(IC + 4 >= MAX_CODE){
            err("error: IC is too large. ending first pass of assembly");
            error = 1;
            return 0;
        }
        IC+=4;
    }
    if(error){
        err("errors detected in first pass - assembly will not continue");
        return 0;
    }
    ICF = IC;
    DCF = DC;
    update_data_symbles(ICF, symbletab);
    return 1;
}