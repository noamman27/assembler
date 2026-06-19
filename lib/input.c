#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils.h"
#include "../main/assembler.h"

/*gets the next param after lp in line.
* if its a symble it puts it in sym, if its an immediate value puts it in immed.
* returns 0 on fail, return 1 on success, returns -1 if symble is detected, returns -2 if immediate value is detected, returns the register number + 1 otherwise*/
int getparam(char line[], int *lp, char sym[], int *immed){
    char c, param[MAXLINE], tmp[MAXLINE];
    int reg, comma = 0, i = 0;
    if(!getch(line, c, lp)){ /*grab first char of the params*/
        err("error: no parameters given"); /*if none are given we print an error*/
        return 0; /*and signal error*/
    }
    if(c == ','){ /*check if theres a comma before the param*/
        comma = 1; /*turn on comma flag*/
    }
    if(!getch(line, c, lp)){
        err("error: nothing after a comma");
        return 0;
    }
    if(c == "$"){ /*if first char is $ we have a register*/
        if(!(getword(param,line))){ /*get the register name using getword*/
            err("error: incomplete register"); /*if none is given we signal an error*/
            return 0;
        }
        if(!isnum(param)){ /*if its not a number we also print and signal errors.*/
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
        if(getch(line, c , lp)){ /*check for comma in end*/
            if(!comma && c == ','){
                comma = 1; 
            }
        }
        if(comma){ /*only return success if we saw comma*/
            return reg + 1; /*add one to reg since reg can be 0 and we need 0 to signal fail*/
        }
        /*if no comma was detected we pirnt and signal more errors*/
        err("missing comma");
        return 0;
    }
    if(isdigit(c)){
        tmp[0] = c;
        while(isdigit(tmp[++i] = line[*lp++])){
            if(lp > 80){
                return 0;
            }
        }
        tmp[i+1] = '\0';
        *immed = atoi(tmp);
        return IMMED;
    }
    if(isalpha(c)){
        sym[0] = c;
        while(!isspace(sym[++i] = line[*lp++])){
            if(lp > 80){
                return 0;
            }
        }
        sym[i+1] = '\0';
        return SYM;
    }
}

/*puts the first non space char in buffer after start in ch and updates start. returns 0 on fail. 1 on success*/
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