#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static int handle_data(char line[], char word[], char **data_image, int *lp, int isSym, char *sym, int lc, symbol **symboltab, int *DC);
static int encode_command(char line[], char word[], int *code_image, int *lp, char type, int lc, int ip);

int first_pass(FILE *input, char *name, nlist *macrotab[]){
    char line[MAXLINE], word[MAXLINE], sym[MAXLINE], type, *data_image = NULL; /*char arrays to represent the whole line, a word in that line, the symbol being defined in that line, a temporary pointer for realloc, type of command, and array to hold parameters*/
    int isSym = 0, len, error = 0, lp = 0, status, *code_image, lc = 0, IC = IC_START, DC = 0, ICF, DCF, ip; /*flag to tell a label is being defined, length of word, error flag, line pointer, status of function, instruction counter, data counter adn their final values, and instruction pointer to useinstead of */
    symbol *symboltab = NULL, *sp; /*head of symbol list and a symbol pointer*/
    void *tmpbuf;

    code_image = (int *)malloc(sizeof(int));
    ip = 0;
    while(fgets(line, MAXLINE, input) != NULL){ /*run as long as we can read more from file*/
        lc++;
        lp = 0, isSym = 0; /*reset line pointer and isSym flag*/
        if(line[0] == ';' || ((len = getword(word, line, &lp)) == 0)){ /*check if line is omment or empty; if so we ignore it*/
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
                err("a label cannot have the same name as a macro");
                error = 1;
                continue;
            }
            if(gettype(word, &type)){ /*check if label name is a command*/
                err("a label cannot have the same name as a command");
                error = 1;
                continue;
            }
            if(strcmp(word, ".entry") == 0 || strcmp(word, ".extern") == 0 || strcmp(word, ".db") == 0 || strcmp(word, ".dh") == 0 || strcmp(word, ".dw") == 0){
                err("a label cannot have the same name as an instruction");
                error = 1;
                continue;
            }
            if(isdigit(word[0])){
                err("a label cannot begin with a number");
                error = 1;
                continue;
            }
            isSym = 1; /*toggle sym flag*/
            strcpy(sym, word); /*save label name in sym*/
            getword(word, line, &lp); /*and get the next word*/
        }
        /*handle data instructions*/
        if((status = handle_data(line, word, &data_image, &lp, isSym, sym, lc, &symboltab, &DC)) == 1){
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
            if(!getword(word, line, &lp)){
                err("no symbol given as parameter for .extern");
                error = 1;
                continue;
            }
            if(isdigit(word[0])){
                err("symbol given as parameter for .extern isnt valid");
                error = 1;
                continue;
            }
            if(gettype(word, &type)){
                err(".extern cannot be given a command");
                error = 1;
                continue;
            }
            if(strcmp(word, ".entry") == 0 || strcmp(word, ".extern") == 0 || strcmp(word, ".db") == 0 || strcmp(word, ".dh") == 0 || strcmp(word, ".dw") == 0){
                err(".extern cannot be given an instruction");
                error = 1;
                continue;
            }
            sp = lookup_symbol(word, symboltab);
            if(sp && strcmp(sp->attribute, "external")){
                fprintf(stderr,"error in line %d: label '%s' already defined not as external\n",lc, word);
                error = 1;
                continue;
            }
            add_symbol(word, 0, "external", &symboltab);
            continue;
        }
        if(!gettype(word,&type)){
            fprintf(stderr, "error in line %d: command %s not recognized\n", lc, word);
            error = 1;
            continue;
        }
        /*handle encoding of commands*/        
        if(!encode_command(line, word, code_image, &lp, type, lc, ip)){
            error = 1;
            continue;
        }
        if(isSym){
            add_symbol(sym, IC, "code", &symboltab);
        }
        ip++;
        IC+=4;
        tmpbuf = realloc(code_image, (ip + 1) * sizeof(int)); /*realloc the array*/
        if(!tmpbuf){ /*ensure success*/
            err("realloc failed");
            error = 1;
            continue;
        }
        code_image = tmpbuf;
    }
    if(error){
        fprintf(stderr, "errors detected in first pass - assembly will not continue\n");
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
static int handle_data(char line[], char word[], char **data_image, int *lp, int isSym, char *sym, int lc, symbol **symboltab, int *DC){
    int count, types[MAXLINE], i, len;
    char *tmpbuf;
    char *params[MAXLINE], *tmp;
    char *endptr;
    long val;
    unsigned int uval;
    if(strcmp(word, ".dh") == 0 ) {
    count = getparams(line, lp, params, types, lc);
    if(isSym){ /*if a label is being defined we add it as data*/
        add_symbol(sym, *DC, "data", symboltab);
    }
    for(i = 0; i<count; i++){
        if(types[i] != IMMED){
            err(".dh only accepts numbers");
            return 0;
        }
        val = strtol(params[i], &endptr, 10);
        if(*endptr != '\0' || val > MAX_HALF_WORD || val < MIN_HALF_WORD){
            err("number given to .dh exceeds maximum");
            return 0;
        }
        uval = (unsigned int)val;
        tmp = (char *) realloc(*data_image, *DC += HALF_WORD);
        if(!tmp){
            err("realloc failed");
            return 0;
        }
        *data_image = tmp;
        (*data_image)[*DC - 2] = (unsigned char)(uval & 0xFF);
        (*data_image)[*DC - 1] = (unsigned char)((uval >> 8) & 0xFF);
    }
    return 1;
}
else if(strcmp(word, ".db") == 0){
    count = getparams(line, lp, params, types, lc);
    if(isSym){ /*if a label is being defined we add it as data*/
        add_symbol(sym, *DC, "data", symboltab);
    }
    for(i = 0; i<count; i++){
        if(types[i] != IMMED){
            err(".db only accepts numbers");
            return 0;
        }
        val = strtol(params[i], &endptr, 10);
        if(*endptr != '\0' || val > MAX_BYTE || val < MIN_BYTE){
            err("number given to .db exceeds maximum");
            return 0;
        }
        uval = (unsigned int)val;
        tmp = (char *) realloc(*data_image, *DC += 1);
        if(!tmp){
            err("realloc failed");
            return 0;
        }
        *data_image = tmp;
        (*data_image)[*DC - 1] = (unsigned char)(uval & 0xFF);
    }
    return 1;
}
else if(strcmp(word, ".dw") == 0){
    count = getparams(line, lp, params, types, lc);
    if(isSym){ /*if a label is being defined we add it as data*/
        add_symbol(sym, *DC, "data", symboltab);
    }
    for(i = 0; i<count; i++){
        if(types[i] != IMMED){
            err(".dw only accepts numbers");
            return 0;
        }
        val = strtol(params[i], &endptr, 10);
        if(*endptr != '\0' || val > MAX_WORD || val < MIN_WORD){
            err("number given to .dw exceeds maximum");
            return 0;
        }
        uval = (unsigned int)val;
        tmp = (char *) realloc(*data_image, *DC += WORD);
        if(!tmp){
            err("realloc failed");
            return 0;
        }
        *data_image = tmp;
        (*data_image)[*DC - 4] = (unsigned char)(uval & 0xFF);
        (*data_image)[*DC - 3] = (unsigned char)((uval >> 8) & 0xFF);
        (*data_image)[*DC - 2] = (unsigned char)((uval >> 16) & 0xFF);
        (*data_image)[*DC - 1] = (unsigned char)((uval >> 24) & 0xFF);
    }
    return 1;
}
    else if(strcmp(word, ".asciz") == 0){
        if(!(len = getword(word, line, lp))){ /*get parameter and save its length into len*/
            err("no string given to .asciz");
            return 0;
        }
        if(isSym){ 
            add_symbol(sym, *DC, "data", symboltab);
        }
        for(i = 0; i < len; i++){
            if(isdigit(word[i])){
                err("digit given to .asciz");
                return 0;
            }
        }
        if(word[0] != '"' || word[len-1] != '"'){
            err("string given to .asciz is not valid");
            return 0;
        }
        remove_quotes(word);
        len = (int)strlen(word);
        *DC += len + 1;
        tmpbuf = realloc(*data_image, *DC);
        if(!tmpbuf){
            err("realloc failed");
            return 0;
        }
        *data_image = tmpbuf;
        for(i = 0; i < len; i++){
            (*data_image)[*DC - (len + 1) + i] = (unsigned char)word[i];
        }
        (*data_image)[*DC - 1] = '\0';
        return 1;
    }
    else{
        return -1;
    }
}
/*hadnles the encoding of commands, takes the line, the word, the code image, line pointer, snd command type. returns 1 on success amnd 0 on error*/
static int encode_command(char line[], char word[], int *code_image, int *lp, char type, int lc, int ip){
    int count, types[MAXLINE];
    char *params[MAXLINE];
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    unsigned int word_value;
    word_value = 0;
    switch (type)
    {
    case 'r':
        if(count_params(word) == 2){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to move command");
                return 0;
            }
            if(count > 2){
                err("too many parameters given to move command");
                return 0;
            }
            else if(count < 2){
                err("not enough parameters given to move command");
                return 0;
            }
            if(types[0] !=  REG || types[1] != REG){
                err("a move command requires two registers");
                return 0;
            }
            rc_bf.rs = atoi(params[0]);
            rc_bf.rt = 0;
            rc_bf.rd = atoi(params[1]);
            rc_bf.opcode = 1;
            rc_bf.funct = getfunct(word);
            word_value = ((unsigned int)rc_bf.opcode << 26)
                       | ((unsigned int)rc_bf.rs << 21)
                       | ((unsigned int)rc_bf.rt << 16)
                       | ((unsigned int)rc_bf.rd << 11)
                       | ((unsigned int)rc_bf.funct << 6);
            code_image[ip] = (int)word_value;
        } else {
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to arithmatic or logical R command");
                return 0;
            }
            if(count > 3){
                err("too many parameters given to arithmatic or logical R command");
                return 0;
            }
            else if(count < 3){
                err("not enough parameters given to arithmatic or logical R command");
                return 0;
            }
            if(types[0] != REG || types[1] != REG || types[2] != REG){
                err("an arithmatic or logical R command requires three registers");
                return 0;
            }
            rc_bf.rs = atoi(params[0]);
            rc_bf.rt = atoi(params[1]);
            rc_bf.rd = atoi(params[2]);
            rc_bf.opcode = 0;
            rc_bf.funct = getfunct(word);
            word_value = ((unsigned int)rc_bf.opcode << 26)
                       | ((unsigned int)rc_bf.rs << 21)
                       | ((unsigned int)rc_bf.rt << 16)
                       | ((unsigned int)rc_bf.rd << 11)
                       | ((unsigned int)rc_bf.funct << 6);
            code_image[ip] = (int)word_value;
        }
        return 1;
        break;
    case 'i':
        /*handle arithmatic or logical commands*/
        if(isarithorlog(word)){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to arithmatic or logical I command");
                return 0;
            }
            if(count > 3){
                err("too many parameters given to arithmatic or logical I command");
                return 0;
            }
            else if(count < 3){
                err("not enough parameters given to arithmatic or logical I command");
                return 0;
            }
            if(types[0] != REG || types[1] != IMMED || types[2] != REG){
                err("an arithmatic or logical I command should be given a register, an immediate value, and another register");
                return 0;
            }
            ic_bf.rs = atoi(params[0]);
            ic_bf.immed = atoi(params[1]);
            ic_bf.rt = atoi(params[2]);
            ic_bf.opcode = getopcode(word);
            word_value = ((unsigned int)ic_bf.opcode << 26)
                       | ((unsigned int)ic_bf.rs << 21)
                       | ((unsigned int)ic_bf.rt << 16)
                       | ((unsigned int)(unsigned short)ic_bf.immed);
        }
        /*handle conditional commands*/
        if(iscond(word)){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to conditional I command");
                return 0;
            }
            if(count > 3){
                err("too many parameters given to conditional I command");
                return 0;
            }
            else if(count < 3){
                err("not enough parameters given to conditional I command");
                return 0;
            }
            if(types[0] != REG || types[1] != REG || types[2] != SYM){
                err("a conditional I command needs 2 registers and a label");
                return 0;
            }
            ic_bf.rs = atoi(params[0]);
            ic_bf.rt = atoi(params[1]);
            ic_bf.opcode = getopcode(word);
            ic_bf.immed = 0;
            word_value = ((unsigned int)ic_bf.opcode << 26)
                       | ((unsigned int)ic_bf.rs << 21)
                       | ((unsigned int)ic_bf.rt << 16);
        }
        /*handle memory loading or saving commands*/
        if(isloading(word)){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to memory loading I command");
                return 0;
            }
            if(count > 3){
                err("too many parameters given to memory loading I command");
                return 0;
            }
            else if(count < 3){
                err("not enough parameters given to memory loading I command");
                return 0;
            }
            if(types[0] != REG || types[1] != IMMED || types[2] != REG){
                err("a memory loading or saving I command should be given a register, an immediate value, and another register");
                return 0;
            }
            /*ensure we dont exceed maximum values*/
            if((strcmp(word, "lb") == 0 && atoi(params[1]) > MAX_BYTE) || (strcmp(word, "lh") == 0 && atoi(params[1]) > MAX_HALF_WORD) || (strcmp(word, "lw") == 0 && atoi(params[1]) > MAX_WORD)){
                err("value given to loading command exceeds maximum");
            }
            ic_bf.rs = atoi(params[0]);
            ic_bf.immed = atoi(params[1]);
            ic_bf.rt = atoi(params[2]);
            ic_bf.opcode = getopcode(word);
            word_value = ((unsigned int)ic_bf.opcode << 26)
                       | ((unsigned int)ic_bf.rs << 21)
                       | ((unsigned int)ic_bf.rt << 16)
                       | ((unsigned int)(unsigned short)ic_bf.immed);
        }
        code_image[ip] = (int)word_value;
        return 1;
        break;
    case 'j':
        if(strcmp(word, "jmp") == 0){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to jmp command");
                return 0;
            }
            if(count > 1){
                err("too many parameters given to jmp command");
                return 0;
            }
            if(types[0] == SYM){
                jc_bf.opcode = 30;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            else if(types[0] == REG){
                jc_bf.opcode = 30;
                jc_bf.reg = 1;
                jc_bf.address = atoi(params[0]);
            }
            else{
                err("an immediate value cannot be given to a jmp command");
                return 0;
            }
        }
        else if(strcmp(word, "la") == 0){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to la command");
                return 0;
            }
            if(count > 1){
                err("too many parameters given to la command");
                return 0;
            }
            if(types[0] == SYM){
                jc_bf.opcode = 31;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            else{
                err("an la command must be given a label");
                return 0;
            }
        }
        else if(strcmp(word, "call") == 0){
            if(!(count = getparams(line, lp, params, types, lc))){
                err("no parameter given to call command");
                return 0;
            }
            if(count > 1){
                err("too many parameters given to call command");
                return 0;
            }
            if(types[0] == SYM){
                jc_bf.opcode = 32;
                jc_bf.reg = 0;
                jc_bf.address = 0;
            }
            else{
                err("a call command must be given a label");
                return 0;
            }
        }
        else if(strcmp(word, "hlt") == 0){
            if(getparams(line, lp, params, types, lc)){
                err("no parameters should be given to an hlt command");
                return 0;
            }
            jc_bf.opcode = 63;
            jc_bf.reg = 0;
            jc_bf.address = 0;
        }
        else{
            return 0;
        }
        word_value = ((unsigned int)jc_bf.opcode << 26)
                   | ((unsigned int)jc_bf.reg << 25)
                   | ((unsigned int)jc_bf.address);
        code_image[ip] = (int)word_value; /*add to code image*/
        return 1;
        break;
    default:
        return 0;
    }
    return 0;
}