#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils.h"
#include "../main/assembler.h"

/*gets all the parameters in line.
* puts the type of the parameter (register, immediate, label) in types
* returns the amount of parameters the function found*/
int getparams(char line[], int *lp, char *params[], int types[], int lc){
    char c, param[MAXLINE], tmp[MAXLINE];
    int reg, comma = 0, i = 0;
    if(!getch(line, &c, lp)){ /*grab first char of the params*/
        return 0; /*if no param is given we return 0*/
    }
    if(c == ','){ /*check if theres a comma before the param*/
        err("error: comma placed before first parameter");
        return 0;
    }
    ungetch(line, c, lp);
    while (getword(param, line, lp)){ /*run loop as long as there are more parameters*/
        if(param[0] == '$'){ /*if first char is $ we have a register*/
            /*remove the $ from the register*/
            param[0] = param[1];
            param[1] = param[2];
            param[2] = '\0';
            if(!isnum(param)){ /*if its not a number we also print and signal errors*/
                err("error: register isnt a number");
                return 0;
            }
            reg = atoi(param); /*use atoi to get the number*/
            if(reg > 31){ /*check that 0 <= num <= 31*/
                err("error: a register cannot be larger than 31");
                return 0;
            }
            else if(reg < 0){
                err("error: a register cannot be less than 0");
            }
            types[i] = REG;
            params[i] = param;
        }
        else if(isnum(param)){
            /*parameter is an immediate value*/
            types[i] =  IMMED;
            params[i] = param;
        }
        else if(isalpha(param[0])){
            /*parameter is a label*/
            types[i] =  SYM;
            params[i] = param;
        }
        else{
            err("error: parameter is not register, immediate value, or label");
        }
        if(!getch(line, &c, lp)){
            /*we reached the end of the line*/
            return i + 1;
        }
        if(c != ','){
            err("error: missing comma between parameters");
            return 0;
        }
        i++;
    }
    return 0;
}

/*gets a single parameter from the line and classifies it as register, immediate, or symbol*/
int getparam(char line[], int *lp, char sym[], int *immed, int lc){
    char c, param[MAXLINE];
    int len;

    if(!getch(line, &c, lp)){
        return 0;
    }
    if(c == ','){
        err("error: comma placed before first parameter");
        return 0;
    }
    ungetch(line, c, lp);

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
            err("error: register isnt a number");
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

    err("error: parameter is not register, immediate value, or label");
    return 0;
}

/*puts the first non space char in buffer after lp in ch and updates lp. returns 0 on fail. 1 on success*/
int getch(char buffer[], char *ch, int *lp){
    char c;
    while(isspace((c = buffer[*lp++])))
        ;
    if(c == EOF){
        return 0;
    }
    *ch = c;
    return 1;
}   

/*puts c into buffer so that the next time getch is called with line and *lp it grabs c. returns 0 on fail and 1 on success*/
int ungetch(char buffer[], char c, int *lp){
    while(!isspace(buffer[*lp])){
        if(*lp >= 1){
           *lp--; 
        }
        else{
            return 0;
        }
    }
    buffer[*lp] = c;
    return 1;
}
/*puts the first word after lp in line inside word. returns the length of the word and updated lp*/
int getword(char word[], char line[], int *lp){
    int i = *lp, j = 0;

    while(line[i] != '\0' && isspace((unsigned char)line[i])){
        i++;
    }

    while(line[i] != '\0' && !isspace((unsigned char)line[i])){
        word[j++] = line[i++];
    }
    word[j] = '\0';

    memmove(line, line + i, strlen(line + i) + 1);
    *lp += i;
    return i;
}