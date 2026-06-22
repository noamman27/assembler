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
        if(!(getword(param,line,lp))){ /*get the register name using getword*/
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
        /*parameter is an immediate value*/
        tmp[0] = c; /*we start putting the digits into tmp and then use atoi to get the number*/
        while(isdigit(tmp[++i] = line[*lp++]) && lp < 81) /*place chars into tmp while making sure we dont exceed the length of the line*/
            ;
        tmp[i+1] = '\0';
        *immed = atoi(tmp);
        return IMMED;
    }
    if(isalpha(c)){
        /*parameter is a label*/
        sym[0] = c; /*start putting chars into sym*/
        while(!isspace(sym[++i] = line[*lp++]) && lp < 81) /*place chars into sym while making sure we dont exceed the length of the line*/
            ;
        sym[i+1] = '\0';
        if(gettype(sym, tmp)){
            err("error: a label cannot habe the same name as a command");
            return 0;
        }
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