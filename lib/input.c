#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils.h"
#include "../main/assembler.h"

int getreg(char line[], int *lp){
    char c, param[MAXLINE];
    int reg, comma = 0;
    if(!getch(line, c, lp)){
        err("no register given");
        return 0;
    }
    if(c == ','){
        comma = 1;
    }
    if(!getch(line, c, lp)){
        err("nothing after a comma");
        return 0;
    }
    if(c == "$"){
        if(!(getword(param,line))){
            err("error: incomplete register");
            return 0;
        }
        if(!isdigit(param)){
            err("error: register isnt a number");
            return 0;
        }
        reg = atoi(param);
        if(reg > 31){
            err("error: a register cannot be larger than 31");
        }
        if(getch(line, c , lp)){
            if(!comma && c == ','){
                comma = 1; 
            }
        }
    }
    if(isdigit(c)){
        err("error: no $ placed before register");
        return 0;
    }
    if(isalpha(c)){
        /*TODO: handle labels*/
        return SYM;
    }
}

/*puts the first non space char in buffer after start in ch and updates start. returns 0 on fail. 1 on success*/
int getch(char buffer[], char *ch, int *start){
    char c;
    while(isspace((c = buffer[*start++])))
        ;
    if(c == EOF){
        return 0;
    }
    *ch = c;
    return 1;
}   