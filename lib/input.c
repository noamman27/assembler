#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils.h"
#include "../main/assembler.h"

/*gets the next param after lp in line.
* if its a symble it puts it in sym, if its an immediate value puts it in immed.
* returns 0 on fail, return 1 on success, returns -1 if symble is detected, returns -2 if immediate value is detected, returns the register number + 1 otherwise*/
int getparams(char line[], int *lp, char *params[], int types[]){
    char c, param[MAXLINE], tmp[MAXLINE];
    int reg, comma = 0, i = 0;
    if(!getch(line, c, lp)){ /*grab first char of the params*/
        err("error: no parameters given"); /*if none are given we print an error*/
        return 0; /*and signal error*/
    }
    if(c == ','){ /*check if theres a comma before the param*/
        err("error: comma placed before first parameter");
        return 0;
    }
    ungetch(line, c, lp);
    while (getword(param, line, lp)){
        if(param[0] == "$"){ /*if first char is $ we have a register*/
            /*remove the $ from the register*/
            param[0] = param[1];
            param[1] = param[2];
            param[2] = NULL;
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
        if(!getch(line, c, lp)){
            return i + 1;
        }
        if(c != ','){
            err("error: missing comma between parameters");
            return 0;
        }
        i++;
    }
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