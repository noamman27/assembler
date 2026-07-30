#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "../main/assembler.h"

/*gets all the parameters in line.
* puts the type of the parameter (register, immediate, label) in types
* returns the amount of parameters the function found*/
int getparams(char line[], int *lp, char *params[], int types[], int lc){
    char param[MAXLINE];
    int reg, i = 0;
    
    /* rely on getword which skips leading whitespace/commas */
    while (getword(param, line, lp)){
        
        if(param[0] == '$'){ /*if first char is $ we have a register*/
            /*remove the $ from the register*/
            param[0] = param[1];
            param[1] = param[2];
            param[2] = '\0';
            if(!isnum(param)){ /*if its not a number we also print and signal errors*/
                err("register isnt a number");
                return 0;
            }
            reg = atoi(param); /*use atoi to get the number*/
            if(reg > 31){ /*check that 0 <= num <= 31*/
                err("a register cannot be larger than 31");
                return 0;
            }
            else if(reg < 0){
                err("a register cannot be less than 0");
            }
            types[i] = REG;
            params[i] = dupstr(param);
        }
        else if(isnum(param)){
            /*parameter is an immediate value*/
            types[i] =  IMMED;
            params[i] = dupstr(param);
        }
        else if(isalpha((unsigned char)param[0])){
            /*parameter is a label*/
            types[i] =  SYM;
            params[i] = dupstr(param);
        }
        else{
            err("parameter is not register, immediate value, or label");
        }
        i++;
    }
    if(i == 0) return 0;
    return i;
}

/*gets a single parameter from the line and classifies it as register, immediate, or symbol*/
int getparam(char line[], int *lp, char sym[], int *immed, int lc){
    char param[MAXLINE];
    int len;

    /* rely on getword which skips leading whitespace and commas */
    len = getword(param, line, lp);
    if(len == 0){
        return 0;
    }

    while(len > 0 && param[len - 1] == ','){
        param[--len] = '\0';
    }

    if(param[0] == '$'){
        memmove(param, param + 1, strlen(param));
        if(!isnum(param)){
            err("register isnt a number");
            return 0;
        }
        *immed = atoi(param);
        return *immed + 1;
    }

    if(isnum(param)){
        *immed = atoi(param);
        return IMMED;
    }

    if(isalpha((unsigned char)param[0])){
        strcpy(sym, param);
        return SYM;
    }

    err("parameter is not register, immediate value, or label");
    return 0;
}

/*puts the first non space char in buffer after lp in ch and updates lp. returns 0 on fail. 1 on success*/
int getch(char buffer[], char *ch, int *lp){
    char c;
    /* skip whitespace */
    while(buffer[*lp] != '\0' && isspace((unsigned char)buffer[*lp])){
        (*lp)++;
    }
    c = buffer[*lp++];
    if(c == '\0'){
        return 0;
    }
    *ch = c;
    return 1;
}   

/*puts c into buffer so that the next time getch is called with line and *lp it grabs c. returns 0 on fail and 1 on success*/
int ungetch(char buffer[], char c, int *lp){
    if(*lp == 0) return 0;
    /* move pointer back one position so the next getch/getword sees c again */
    (*lp)--;
    return 1;
}
/*puts the first word after lp in line inside word. returns the length of the word and updated lp*/
int getword(char word[], char line[], int *lp){
    int i = *lp, j = 0;

    /* skip leading whitespace or commas from current lp */
    while(line[i] != '\0' && (isspace((unsigned char)line[i]) || line[i] == ',')){
        i++;
    }

    /* copy the next token into word; stop on whitespace or comma */
    if(line[i] == '"'){
        /* quoted string: include quotes and all until closing quote */
        word[j++] = line[i++];
        while(line[i] != '\0' && line[i] != '"'){
            word[j++] = line[i++];
        }
        if(line[i] == '"'){
            word[j++] = line[i++];
        }
    } else {
        while(line[i] != '\0' && !isspace((unsigned char)line[i]) && line[i] != ','){
            word[j++] = line[i++];
        }
    }
    word[j] = '\0';

    /* update lp to point after the token */
    *lp = i;
    return j; /* return length of word */
}