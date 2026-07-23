#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

symbol *symboltab = NULL, *sp; /* head of the symbol table linked list */
int *code_image, *tmpint; /*code image and tmpint - a temporary int pointer (im a software engineer so I cant name things well)*/
char *data_image;
int IC = IC_START, DC = 0, ICF, DCF, ip; /*line pointer, IC, DC, and their final values, as well as an instruction pointer (current location in code_image), which is used instead of IC to cut down on memory*/

int first_pass(FILE *input){
    char line[MAXLINE], word[MAXLINE], sym[MAXLINE], *tmp, type, *params[MAXLINE]; /*char arrays to represent the whole line, a word in that line, the symbol being defined in that line, a temporary pointer for realloc, type of command, and array to hold parameters*/
    int isSym = 0, len, i, j, count, error = 0, types[MAXLINE], *lp = 0; /*flag to say if a symbol is being defined, length of word, count of params, index, and error flag*/
    R_BF rc_bf; /*bitfields for all commands*/
    I_BF ic_bf;
    J_BF jc_bf;
    code_image = (char *) malloc(sizeof(char)), data_image = (int *) malloc(sizeof(int)); /*initialize code and data image with malloc, sizeof(char) is used instead of 1 for clarity*/
    while(fgets(line, MAXLINE, input) != NULL){ /*run as long as we can read more from file*/
        lp = 0, isSym = 0; /*reset line pointer and isSym flag*/
        if(line[0] == ';' || ((len = getword(word, line, lp)) == 0)){ /*check if line is omment or empty; if so we ignore it*/
            continue;
        }
        /*check if last char of word is :, if so its a label definition*/
        if(word[len-1] == ':'){ 
            word[len-1] = '\0'; /*remove the : from the label*/
            if(lookup_symbol(word, symboltab)){ /*check if label is already defined*/
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
            count = getparams(line, lp, params, types);
            if(isSym){ /*if a label is being defined we add it as data*/
                add_symbol(sym, DC, "data", symboltab);
            }
            for(i = 0; i<count; i++){
                if(types[i] != IMMED){
                    err("error: .dh only accepts numbers");
                    error = 1;
                    continue;
                }
                if(atoi(params[i]) > MAX_HALF_WORD || atoi(params[i]) MIN_HALF_WORD){
                    err("error: number given to .dh exceeds values representable");
                    error = 1;
                    continue;
                }
                tmp = (char *) realloc(data_image, DC += HALF_WORD);
                if(!tmp){
                    err("error: realloc failed");
                    error = 1;
                    continue;
                }
                data_image = tmp;
                for(j = 0; j < HALF_WORD; j++){ /*add data to data image*/
                    data_image[DC - (HALF_WORD - j)] = (char) atoi(params[i]) << 8 * j;
                }
            }
            continue;
        }
        else if(strcmp(word, ".db") == 0){
            count = getparams(line, lp, params, types);
            if(isSym){ /*if a label is being defined we add it as data*/
                add_symbol(sym, DC, "data", symboltab);
            }
            for(i = 0; i<count; i++){
                if(types[i] != IMMED){
                    err("error: .dh only accepts numbers");
                    error = 1;
                    continue;
                }
                if(atoi(params[i]) > MAX_BYTE || atoi(params[i]) MIN_BYTE){
                    err("error: number given to .dh exceeds values representable");
                    error = 1;
                    continue;
                }
                tmp = (char *) realloc(data_image, DC += 1);
                if(!tmp){
                    err("error: realloc failed");
                    error = 1;
                    continue;
                }
                data_image = tmp;
                data_image[DC] = atoi(params[i]);
            }
            continue;
        }
        else if(strcmp(word, ".dw") == 0){
            count = getparams(line, lp, params, types);
            if(isSym){ /*if a label is being defined we add it as data*/
                add_symbol(sym, DC, "data", symboltab);
            }
            for(i = 0; i<count; i++){
                if(types[i] != IMMED){
                    err("error: .dh only accepts numbers");
                    error = 1;
                    continue;
                }
                if(atoi(params[i]) > MAX_WORD || atoi(params[i]) MIN_WORD){
                    err("error: number given to .dh exceeds values representable");
                    error = 1;
                    continue;
                }
                tmp = (char *) realloc(data_image, DC += WORD);
                if(!tmp){
                    err("error: realloc failed");
                    error = 1;
                    continue;
                }
                data_image = tmp;
                for(j = 0; j < WORD; j++){ /*add data to data image*/
                    data_image[DC - (WORD - j)] = (char) atoi(params[i]) << 8 * j;
                }
            }
            continue;
        }
        else if(strcmp(word, ".asciz") == 0){
            if(!(len = getword(word, line, lp))){ /*get parameter and save its length into len*/
                err("error: no string given to .asciz");
                error = 1;
                continue;
            }
            if(isSym){ 
                add_symbol(sym, DC, "data", symboltab);
            }
            for(i = 0; i < len; i++){
                if(isdigit(word[i])){
                    err("error: digit given to .asciz");
                    error = 1;
                    continue;
                }
            }
            if(word[0] !='"' || word[len-2] != '"'){
                err("error: string given to .asciz is not valid");
                error = 1;
                continue;
            }
            remove_quotes(word);
            len -= 2;
            DC += len + 1;
            tmpint = realloc(data_image, DC * sizeof(*data_image));
            if(!tmpint){
                err("error: realloc failed");
                error = 1;
                continue;
            }
            data_image = tmpint;
            for(i = 0; i < len; i++){
                data_image[DC - (len + 1) + i] = (unsigned char)word[i];
            }
            data_image[DC - 1] = '\0';
        }
        /*handle .entry and .extern*/
        if(strcmp(word, ".entry") == 0){
            continue;
        }
        if(strcmp(word, ".extern") == 0){
            if(!getword(word, line, lp)){
                err("error: no symbol given as parameter for .extern");
                error = 1;
                continue;
            }
            if(isdigit(word[0])){
                err("symbol given as parameter for .extern isnt valid");
                error = 1;
                continue;
            }
            if(gettype(word, tmp)){
                err("error: a label cannot habe the same name as a command");
                error = 1;
                continue;
            }
            sp = lookup_symbol(word, symboltab);
            if(sp && strcmp(sp->attribute, "external")){
                fprintf(stderr,"error: label %s already defined not as external", word);
                error = 1;
                continue;
            }
            add_symbol(word, 0, "external", symboltab);
        }
        if(!gettype(word,&type)){
            err("error: command not recognized");
        }
        if(isSym){
            add_symbol(sym, IC, "code", symboltab);
        }
        /*handle encoding of commands*/
        /*for all commands we get the parameters using getparams, and ensure we got the correct amount and type of parameters, and put the parameters in the correct location*/
        switch (type)
        {
        case 'r':
            if(count_params(word) == 2){
                rc_bf.rs = 0;
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to move command");
                    error = 1;
                    continue;
                }
                if(count > 2){
                    err("error: too many parameters given to move command");
                    error = 1;
                    continue;
                }
                else if(count < 2){
                    err("error: not enough parameters given to move command");
                }
                if(types[0] !=  REG){
                    err("error: a move command cannot be given a parmeter that is not a register");
                    error = 1;
                    continue;
                }
                rc_bf.rd = atoi(params[0]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[1] !=  REG){
                    err("error: a move command cannot be given a parmeter that is not a register");
                    error = 1;
                    continue;
                }
                rc_bf.rt = atoi(params[1]);
                rc_bf.opcode = 1;
                rc_bf.funct = getfunct(word);
            } else {
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to arithmatic or logical R command");
                    error = 1;
                    continue;
                }
                if(count > 3){
                    err("error: too many parameters given to arithmatic or logical R command");
                    error = 1;
                    continue;
                }
                else if(count < 3){
                    err("error: not enough parameters given to arithmatic or logical R command");
                }
                if(types[0] !=  REG){
                    err("error: an arithmatic or logical R command cannot be given a parmeter that is not a register");
                    error = 1;
                    continue;
                }
                rc_bf.rs = atoi(params[0]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[1] !=  REG){
                    err("error: an arithmatic or logical R command cannot be given a parmeter that is not a register");
                    error = 1;
                    continue;
                }
                rc_bf.rt = atoi(params[1]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[1] !=  REG){
                    err("error: an arithmatic or logical R command cannot be given a parmeter that is not a register");
                    error = 1;
                    continue;
                }
                rc_bf.rd = atoi(params[1]);
                rc_bf.opcode = 0;
                rc_bf.funct = getfunct(word);
            }
            memcpy(&data_image[ip], &ic_bf, sizeof(ic_bf)); /*add to code image*/
            break;
        case 'i':
            /*handle arithmatic or logical commands*/
            if(isarithorlog(word)){
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to arithmatic or logical I command");
                    error = 1;
                    continue;
                }
                if(count > 3){
                    err("error: too many parameters given to arithmatic or logical I command");
                    error = 1;
                    continue;
                }
                else if(count < 3){
                    err("error: not enough parameters given to arithmatic or logical I command");
                }
                if(types[0] !=  REG){
                    err("error: an arithmatic or logical I command should be given a register, an immediate value, and another register");
                    error = 1;
                    continue;
                }
                ic_bf.rs = atoi(params[0]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[1] !=  IMMED){
                    err("error: an arithmatic or logical I command should be given a register, an immediate value, and another register");
                    error = 1;
                    continue;
                }
                ic_bf.immed = atoi(params[1]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[2] !=  REG){
                    err("error: an arithmatic or logical I command should be given a register, an immediate value, and another register");
                    error = 1;
                    continue;
                }
                ic_bf.rt = atoi(params[2]);
                ic_bf.opcode = getopcode(word);
            }
            /*handle conditional commands*/
            if(iscond(word)){
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to conditional I command");
                    error = 1;
                    continue;
                }
                if(count > 3){
                    err("error: too many parameters given to conditional I command");
                    error = 1;
                    continue;
                }
                else if(count < 3){
                    err("error: not enough parameters given to conditional I command");
                }
                if(types[0] !=  REG){
                    err("error: a conditional I command needs 2 registers and a label");
                    error = 1;
                    continue;
                }
                ic_bf.rs = atoi(params[0]);
                if(!getparams(line, lp, params, types)){
                    err("error: no parameter given to R command");
                    error = 1;
                    continue;
                }
                if(types[1] !=  REG){
                    err("error: a conditional I command needs 2 registers and a label");
                    error = 1;
                    continue;
                }
                ic_bf.rt = atoi(params[1]);
                if(!getparams(line, lp, params, types)){
                    err("error: no parameter given to I command");
                    error = 1;
                    continue;
                }
                if(types[2] !=  SYM){
                    err("error: a conditional I command needs 2 registers and a label");
                    error = 1;
                    continue;
                }
                ic_bf.rt = 0;
                ic_bf.opcode = getopcode(word);
            }
            /*handle memory loading or saving commands*/
            if(isloading(word)){
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to memory loading I command");
                    error = 1;
                    continue;
                }
                if(count > 3){
                    err("error: too many parameters given to memory loading I command");
                    error = 1;
                    continue;
                }
                else if(count < 3){
                    err("error: not enough parameters given to memory loading I command");
                }
                if(types[0] !=  REG){
                    err("error: a memory loading or saving I command should be given a register, an immediate value, and another register");
                    error = 1;
                    continue;
                }
                ic_bf.rs = atoi(params[0]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[1] !=  IMMED){
                    err("error: a memory loading or saving I command should be given a register, an immediate value, and another register");
                    error = 1;
                    continue;
                }
                ic_bf.immed = atoi(params[1]);
                if(!getparams(line, lp, params, types)){
                    err("error: missing parameter");
                    error = 1;
                    continue;
                }
                if(types[2] !=  REG){
                    err("error: a memory loading or saving I command should be given a register, an immediate value, and another register");
                    error = 1;
                    continue;
                }
                ic_bf.rt = atoi(params[2]);
                ic_bf.opcode = getopcode(word);
            }
            memcpy(&data_image[ip], &ic_bf, sizeof(ic_bf)); /*add to code image*/
            break;
        case 'j':
            if(strcmp(word, "jmp") == 0){
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to jmp command");
                    error = 1;
                    continue;
                }
                if(count > 1){
                    err("error: too many parameters given to jmp command");
                    error = 1;
                    continue;
                }
                if(types[0] == SYM){
                    jc_bf.opcode = 30;
                    jc_bf.reg = 0;
                }
                else if(types[0] == REG){
                    jc_bf.opcode = 30;
                    jc_bf.reg = 1;
                    jc_bf.address = atoi(params[0]);
                }
                else{
                    err("error: an immediate value cannot be given to a jmp command");
                    error = 1;
                    continue;
                }
                }
            else if(strcmp(word, "la") == 0){
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to la command");
                    error = 1;
                    continue;
                }
                if(count > 1){
                    err("error: too many parameters given to la command");
                    error = 1;
                    continue;
                }
                if(types[0] == SYM){
                    jc_bf.opcode = 30;
                    jc_bf.reg = 0;
                }
                else{
                    err("error: an la command must be given a label");
                    error = 1;
                    continue;
                }
                jc_bf.opcode = 31;
                jc_bf.reg = 0;
                }
            else if(strcmp(word, "call") == 0){
                if(!(count = getparams(line, lp, params, types))){
                    err("error: no parameter given to call command");
                    error = 1;
                    continue;
                }
                if(count > 1){
                    err("error: too many parameters given to call command");
                    error = 1;
                    continue;
                }
                if(types[0] == SYM){
                    jc_bf.opcode = 30;
                    jc_bf.reg = 0;
                }
                else{
                    err("error: a call command must be given a label");
                    error = 1;
                    continue;
                }
                jc_bf.opcode = 32;
                jc_bf.reg = 0;
                }
            else if(strcmp(word, "hlt") == 0){
                if(getparams(line, lp, params, types)){
                    err("error: no parameters should be given to an hlt command");
                    error = 1;
                    continue;
                }
                jc_bf.opcode = 63;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            memcpy(&data_image[ip], &jc_bf, sizeof(jc_bf)); /*add to code image*/
            break;
        default:
            break;
        }
        IC+=4;
        tmpint = (int *) realloc(code_image, ip * sizeof(int)); /*realloc the array*/
        if(!tmpint){ /*ensure success*/
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
    update_symbols(ICF, symboltab); /*update the symbols by adding icf and dcf*/
    return 1;
}