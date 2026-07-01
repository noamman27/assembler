#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static Symble *symbletab = NULL, *sp; /* head of the symbol table linked list */
int *code_image, *data_image, *tmpint; /*code image, data image and tmpint - a temporary int pointer (im a software engineer so I cant name things well)*/
static int lp = 0, IC = IC_START, DC = 0, ICF, DCF; /*line pointer, IC and DC*/

int first_pass(FILE *input){
    char line[MAXLINE], word[MAXLINE], sym[MAXLINE], *tmp, type; /*char arrays to represent the whole line, a word in that line, the symble being defined in that line, a temporary pointer for realloc, and type of command*/
    int isSym = 0, len, i, reg, error = 0; /*flag to say if a symble is being defined, length of word, value of register, index, and error flag*/
    R_BF rc_bf; /*bitfields for all commands*/
    I_BF ic_bf;
    J_BF jc_bf;
    code_image = malloc(0), data_image = malloc(0); /*initialize code and data image with malloc 0 (we could start off with some value but I dont want to deal with that, so whenever we add anything to these we realloc them)*/
    while(fgets(line, MAXLINE, input) != NULL){ /*run while we can read more from file*/
        lp = 0, isSym = 0; /*reset line pointer and isSym flag*/
        if(line[0] == ';' || ((len = getword(word, line, lp)) == 0)){ /*check if line is omment or empty; if so we ignore it*/
            continue;
        }
        /*check if last char of word is :, if so its a label definition*/
        if(word[len-1] == ':'){ 
            word[len-1] = '\0'; /*remove the : from the label*/
            if(lookup_symble(word, NULL, symbletab)){ /*check if label is already defined*/
                fprintf(stderr, "error: label %s is already defined", word);
                error = 1;
                continue;
            }
            if(lookup(word, macrotab)){ /*check if label was already defined as a macro*/
                err("error: a label cannot have the same name as a macro");
                error = 1;
                continue;
            }
            if(gettype(word, tmp)){ /*check if label name is a command*/
                err("error: a label cannot habe the same name as a command");
                error = 1;
                continue;
            }
            isSym = 1; /*toggle sym flag*/
            *sym = *word; /*save label name isn sym*/
            getword(word, line, lp); /*and get the next word*/
        }
        /*handle data instructions*/
        if(strcmp(word, ".dh") == 0 ) {
            DC += HALF_WORD; /*add half word to DC*/
            if(!getword(word, line, lp)){ /*make sure we get an argument*/
                err("error: no value given to .dh");
                error = 1;
                continue;
            }
            if(isSym){ /*if a label is being defined we add it as data*/
                add_symble(sym, DC, "data", symbletab);
            }
            if(lookup_symble(word, symbletab)){
                continue;
            }
            *tmpint = realloc(data_image, DC);
            if(!tmpint){
                err("error: realloc failed");
                error = 1;
                continue;
            }
            data_image = tmpint;
            data_image[DC] = word;
            continue;
        }
        else if(strcmp(word, ".db") == 0){
            DC += 1; /*add a bit to DC*/
            if(!getword(word, line, lp)){ /*make sure we get an argument*/
                err("error: no value given to .dh");
                error = 1;
                continue;
            }
            if(isSym){ /*if a label is being defined we add it as data*/
                add_symble(sym, DC, "data", symbletab);
            }
            if(lookup_symble(word, symbletab)){
                continue;
            }
            *tmpint = realloc(data_image, DC);
            if(!tmpint){
                err("error: realloc failed");
                error = 1;
                continue;
            }
            data_image = tmpint;
            data_image[DC] = word;
            continue;
        }
        else if(strcmp(word, ".dw") == 0){
            DC += WORD; /*add half word to DC*/
            if(!getword(word, line, lp)){ /*make sure we get an argument*/
                err("error: no value given to .dh");
                error = 1;
                continue;
            }
            if(isSym){ /*if a label is being defined we add it as data*/
                add_symble(sym, DC, "data", symbletab);
            }
            if(lookup_symble(word, symbletab)){
                continue;
            }
            *tmpint = realloc(data_image, DC);
            if(!tmpint){
                err("error: realloc failed");
                error = 1;
                continue;
            }
            data_image = tmpint;
            data_image[DC] = word;
            continue;
        }
        else if(strcmp(word, ".asciz") == 0){
            if(!(len = getword(word, line, lp))){ /*get parameter and save its length into len*/
                err("error: no string given to .asciz");
                error = 1;
                continue;
            }
            if(isSym){ 
                add_symble(sym, DC, "data", symbletab);
            }
            if(isnum(word)){ /*if we got a number*/
                err("error: asciz cannot get a number as a parameter");
                error = 1;
                continue;
            }
            if(word[0] !='"' || word[len-1] != '"'){
                err("error: string given to .asciz is not valid");
                error = 1;
                continue;
            }
            remove_quotes(word);
            DC += len - 2;
            *tmpint = realloc(data_image, DC);
            if(!tmpint){
                err("error: realloc failed");
                error = 1;
                continue;
            }
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
                    continue;
                }
                if(reg == SYM){
                    err("error: a symble cannot be given as a parameter to a move command");
                    error = 1;
                    continue;
                }
                rc_bf.rd = reg - 1;
                if(!(reg = getparam(line, &lp, tmp, &i))){
                    error = 1;
                    continue;
                }
                if(reg == SYM){
                    err("error: a symble cannot be given as a parameter to a move command");
                    error = 1;
                    continue;
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
            memcpy(&data_image[IC], &ic_bf, sizeof(ic_bf)); /*add to code image*/
            break;
        case 'i':
            /*handle arithmatic or logical commands*/
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
            /*handle conditional commands*/
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
            /*handle memory loading or saving commands*/
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
        IC+=4;
        tmpint = realloc(code_image, IC); /*realloc the array*/
        if(!tmpint){
            err("error: realloc failed");
            error = 1;
            continue;
        }
        code_image = tmpint;
    }
    if(error){
        err("errors detected in first pass - assembly will not continue");
        return 0;
    }
    ICF = IC;
    DCF = DC;
    update_symbles(ICF, DCF, symbletab); /*update the symbles by adding icf and dcf*/
    return 1;
}