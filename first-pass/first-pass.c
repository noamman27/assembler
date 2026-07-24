#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

symbol *symboltab = NULL, *sp; /* head of the symbol table linked list */
int IC = IC_START, DC = 0, ICF, DCF, ip; /*line pointer, IC, DC, and their final values, as well as an instruction pointer (current location in code_image), which is used instead of IC to cut down on memory*/

int first_pass(FILE *input, char *name, nlist *macrotab){
    char line[MAXLINE], word[MAXLINE], sym[MAXLINE], *tmp, type, *data_image; /*char arrays to represent the whole line, a word in that line, the symbol being defined in that line, a temporary pointer for realloc, type of command, and array to hold parameters*/
    int isSym = 0, len, error = 0, *lp = 0, status, *tmpint, *code_image; /*flag to say if a symbol is being defined, length of word, count of params, index, and error flag*/
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
        if((status = handle_data(line, word, data_image, lp, isSym, sym)) == 1){
            /*function handled data so we continue*/
            continue;
        }
        else if(!status){
            /*errors detected with instructions*/
            error = 1;
            continue;
        }
        /*if we got here it means the function returned -1, which means it wasnt a data instrution, so we continue*/
        /*handle .entry and .extern*/
        if(strcmp(word, ".entry") == 0){
            /*ignore since it will be handled in the second pass*/
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
        if(!encode_command(line, word, code_image, lp, type)){
            error = 1;
            continue;
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
    update_symbols(ICF, symboltab); /*update the symbols by adding icf*/
    if(!second_pass(input, name, code_image, ICF, DCF, symboltab, data_image )){
        return 0;
    }
    return 1;
}

/*hadnles the data instructions, takes the line, the word, the data image, line pointer, whether a label is being defined, and the name of the label. return 1 on success, 0 of error, and -1 if the word isnt a data instruction*/
static int handle_data(char line[], char word[], char *data_image, int *lp, int isSym, char *sym){
    int count, types[MAXLINE], i, j, len, *tmpint;
    char *params[MAXLINE], *tmp;
    if(strcmp(word, ".dh") == 0 ) {
    count = getparams(line, lp, params, types);
    if(isSym){ /*if a label is being defined we add it as data*/
        add_symbol(sym, DC, "data", symboltab);
    }
    for(i = 0; i<count; i++){
        if(types[i] != IMMED){
            err("error: .dh only accepts numbers");
            return 0;
        }
        if(atoi(params[i]) > MAX_HALF_WORD || atoi(params[i]) MIN_HALF_WORD){
            err("error: number given to .dh exceeds values representable");
            return 0;
        }
        tmp = (char *) realloc(data_image, DC += HALF_WORD);
        if(!tmp){
            err("error: realloc failed");
            return 0;
        }
        data_image = tmp;
        for(j = 0; j < HALF_WORD; j++){ /*add data to data image*/
            data_image[DC - j] = (char) atoi(params[i]);
        }
    }
}
else if(strcmp(word, ".db") == 0){
    count = getparams(line, lp, params, types);
    if(isSym){ /*if a label is being defined we add it as data*/
        add_symbol(sym, DC, "data", symboltab);
    }
    for(i = 0; i<count; i++){
        if(types[i] != IMMED){
            err("error: .dh only accepts numbers");
            return 0;
        }
        if(atoi(params[i]) > MAX_BYTE || atoi(params[i]) MIN_BYTE){
            err("error: number given to .dh exceeds values representable");
            return 0;
        }
        tmp = (char *) realloc(data_image, DC += 1);
        if(!tmp){
            err("error: realloc failed");
            return 0;
        }
        data_image = tmp;
        data_image[DC] = atoi(params[i]);
    }
}
else if(strcmp(word, ".dw") == 0){
    count = getparams(line, lp, params, types);
    if(isSym){ /*if a label is being defined we add it as data*/
        add_symbol(sym, DC, "data", symboltab);
    }
    for(i = 0; i<count; i++){
        if(types[i] != IMMED){
            err("error: .dh only accepts numbers");
            return 0;
        }
        if(atoi(params[i]) > MAX_WORD || atoi(params[i]) MIN_WORD){
            err("error: number given to .dh exceeds values representable");
            return 0;
        }
        tmp = (char *) realloc(data_image, DC += WORD);
        if(!tmp){
            err("error: realloc failed");
            return 0;
        }
        data_image = tmp;
        for(j = 0; j < WORD; j++){ /*add data to data image*/
            data_image[DC - j] = (char) atoi(params[i]);
        }
    }
    return 1;
}
    else if(strcmp(word, ".asciz") == 0){
        if(!(len = getword(word, line, lp))){ /*get parameter and save its length into len*/
            err("error: no string given to .asciz");
            return 0;
        }
        if(isSym){ 
            add_symbol(sym, DC, "data", symboltab);
        }
        for(i = 0; i < len; i++){
            if(isdigit(word[i])){
                err("error: digit given to .asciz");
                return 0;
            }
        }
        if(word[0] !='"' || word[len-2] != '"'){
            err("error: string given to .asciz is not valid");
            return 0;
        }
        remove_quotes(word);
        len -= 2;
        DC += len + 1;
        tmpint = realloc(data_image, DC * sizeof(*data_image));
        if(!tmpint){
            err("error: realloc failed");
            return 0;
        }
        data_image = tmpint;
        for(i = 0; i < len; i++){
            data_image[DC - (len + 1) + i] = (unsigned char)word[i];
        }
        data_image[DC - 1] = '\0';
        return 1;
    }
    else{
        return -1;
    }
}
/*hadnles the encoding of commands, takes the line, the word, the code image, line pointer, snd command type. returns 1 on success amnd 0 on error*/
static int encode_command(char line[], char word[], int *code_image, int *lp, char type){
    int count, types[MAXLINE];
    char *params[MAXLINE];
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    switch (type)
    {
    case 'r':
        if(count_params(word) == 2){
            rc_bf.rs = 0;
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to move command");
                return 0;
            }
            if(count > 2){
                err("error: too many parameters given to move command");
                return 0;
            }
            else if(count < 2){
                err("error: not enough parameters given to move command");
            }
            if(types[0] !=  REG){
                err("error: a move command cannot be given a parmeter that is not a register");
                return 0;
            }
            rc_bf.rd = atoi(params[0]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[1] !=  REG){
                err("error: a move command cannot be given a parmeter that is not a register");
                return 0;
            }
            rc_bf.rt = atoi(params[1]);
            rc_bf.opcode = 1;
            rc_bf.funct = getfunct(word);
        } else {
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to arithmatic or logical R command");
                return 0;
            }
            if(count > 3){
                err("error: too many parameters given to arithmatic or logical R command");
                return 0;
            }
            else if(count < 3){
                err("error: not enough parameters given to arithmatic or logical R command");
                return 0;
            }
            if(types[0] !=  REG){
                err("error: an arithmatic or logical R command cannot be given a parmeter that is not a register");
                return 0;
            }
            rc_bf.rs = atoi(params[0]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[1] !=  REG){
                err("error: an arithmatic or logical R command cannot be given a parmeter that is not a register");
                return 0;
            }
            rc_bf.rt = atoi(params[1]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[1] !=  REG){
                err("error: an arithmatic or logical R command cannot be given a parmeter that is not a register");
                return 0;
            }
            rc_bf.rd = atoi(params[1]);
            rc_bf.opcode = 0;
            rc_bf.funct = getfunct(word);
        }
        memcpy(&code_image[ip], &ic_bf, sizeof(ic_bf)); /*add to code image*/
        return 1;
        break;
    case 'i':
        /*handle arithmatic or logical commands*/
        if(isarithorlog(word)){
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to arithmatic or logical I command");
                return 0;
            }
            if(count > 3){
                err("error: too many parameters given to arithmatic or logical I command");
                return 0;
            }
            else if(count < 3){
                err("error: not enough parameters given to arithmatic or logical I command");
                return 0;
            }
            if(types[0] !=  REG){
                err("error: an arithmatic or logical I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.rs = atoi(params[0]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[1] !=  IMMED){
                err("error: an arithmatic or logical I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.immed = atoi(params[1]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[2] !=  REG){
                err("error: an arithmatic or logical I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.rt = atoi(params[2]);
            ic_bf.opcode = getopcode(word);
        }
        /*handle conditional commands*/
        if(iscond(word)){
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to conditional I command");
                return 0;
            }
            if(count > 3){
                err("error: too many parameters given to conditional I command");
                return 0;
            }
            else if(count < 3){
                err("error: not enough parameters given to conditional I command");
                return 0;
            }
            if(types[0] !=  REG){
                err("error: a conditional I command needs 2 registers and a label");
                return 0;
            }
            ic_bf.rs = atoi(params[0]);
            if(!getparams(line, lp, params, types)){
                err("error: no parameter given to R command");
                return 0;
            }
            if(types[1] !=  REG){
                err("error: a conditional I command needs 2 registers and a label");
                return 0;
            }
            ic_bf.rt = atoi(params[1]);
            if(!getparams(line, lp, params, types)){
                err("error: no parameter given to I command");
                return 0;
            }
            if(types[2] !=  SYM){
                err("error: a conditional I command needs 2 registers and a label");
                return 0;
            }
            ic_bf.rt = 0;
            ic_bf.opcode = getopcode(word);
        }
        /*handle memory loading or saving commands*/
        if(isloading(word)){
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to memory loading I command");
                return 0;
            }
            if(count > 3){
                err("error: too many parameters given to memory loading I command");
                return 0;
            }
            else if(count < 3){
                err("error: not enough parameters given to memory loading I command");
                return 0;
            }
            if(types[0] !=  REG){
                err("error: a memory loading or saving I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.rs = atoi(params[0]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[1] !=  IMMED){
                err("error: a memory loading or saving I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.immed = atoi(params[1]);
            if(!getparams(line, lp, params, types)){
                err("error: missing parameter");
                return 0;
            }
            if(types[2] !=  REG){
                err("error: a memory loading or saving I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.rt = atoi(params[2]);
            ic_bf.opcode = getopcode(word);
        }
        memcpy(&code_image[ip], &ic_bf, sizeof(ic_bf)); /*add to code image*/
        return 1;
        break;
    case 'j':
        if(strcmp(word, "jmp") == 0){
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to jmp command");
                return 0;
            }
            if(count > 1){
                err("error: too many parameters given to jmp command");
                return 0;
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
                return 0;
            }
            }
        else if(strcmp(word, "la") == 0){
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to la command");
                return 0;
            }
            if(count > 1){
                err("error: too many parameters given to la command");
                return 0;
            }
            if(types[0] == SYM){
                jc_bf.opcode = 30;
                jc_bf.reg = 0;
            }
            else{
                err("error: an la command must be given a label");
                return 0;
            }
            jc_bf.opcode = 31;
            jc_bf.reg = 0;
            }
        else if(strcmp(word, "call") == 0){
            if(!(count = getparams(line, lp, params, types))){
                err("error: no parameter given to call command");
                return 0;
            }
            if(count > 1){
                err("error: too many parameters given to call command");
                return 0;
            }
            if(types[0] == SYM){
                jc_bf.opcode = 30;
                jc_bf.reg = 0;
            }
            else{
                err("error: a call command must be given a label");
                return 0;
            }
            jc_bf.opcode = 32;
            jc_bf.reg = 0;
            }
        else if(strcmp(word, "hlt") == 0){
            if(getparams(line, lp, params, types)){
                err("error: no parameters should be given to an hlt command");
                return 0;
            }
            jc_bf.opcode = 63;
            jc_bf.reg = 0;
            jc_bf.address = 0;
        }
        memcpy(&code_image[ip], &jc_bf, sizeof(jc_bf)); /*add to code image*/
        return 1;
        break;
    default:
        break;
    }
}