#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "utils.h"
#include "../main/assembler.h"

/*list of r commands*/
RCommand rCommands[] = {
    {"add",  0, 1},
    {"sub",  0, 2},
    {"and",  0, 3},
    {"or",   0, 4},
    {"nor",  0, 5},
    {"move", 1, 1},
    {"mvhi", 1, 2},
    {"mvlo", 1, 3},
    {NULL,   0, 0}
};

/*list of i commands*/
ICommand iCommands[] = {
    {"addi", 10},
    {"subi", 11},
    {"andi", 12},
    {"ori",  13},
    {"nori", 14},
    {"bne",  15},
    {"beq",  16},
    {"blt",  17},
    {"bgt",  18},
    {"lb",   19},
    {"sb",   20},
    {"lw",   21},
    {"sw",   22},
    {"lh",   23},
    {"sh",   24},
    {NULL,   0}
};

/*list of j commands*/
JCommand jCommands[] = {
    {"jmp",  30},
    {"la",   31},
    {"call", 32},
    {"hlt",  63},
    {NULL,   0}
};

/*gets the type of command passed in s, puts the type in t. returns 1 if s is command, and 0 if not*/
int gettype(char *s, char *t){
    if(isR(s)){
        *t = 'r';
        return 1;
    }
    if(isI(s)){
        *t = 'i';
        return 1;
    }
    if(isJ(s)){
        *t = 'j';
        return 1;
    }
    return 0;
}

/*checks if command given via s is type r*/
int isR(char *s){
    int i;
    for(i = 0; rCommands[i].name != NULL; i++){
        if(strcmp(s, rCommands[i].name) == 0){
            return 1;
        }
    }
    return 0;
}
/*checks if command given via s is type i*/
int isI(char *s){
    int i;
    for(i = 0; iCommands[i].name != NULL; i++){
        if(strcmp(s, iCommands[i].name) == 0){
            return 1;
        }
    }
    return 0;
}
/*checks if command given via s is type j*/
int isJ(char *s){
    int i;
    for(i = 0; jCommands[i].name != NULL; i++){
        if(strcmp(s, jCommands[i].name) == 0){
            return 1;
        }
    }
    return 0;
}

/*checks if given string *s represents a valid symble*/
int validSym(char *s){
    if(isdigit(*s)){
        return 0;
    }
    s++;
    while(*s)
        ;
    s--;
    if(*s == ':'){
        return 1;
    }
    return 0;
}

/*checks if given pointer represents a number*/
int isnum(char *s){
    while(*s){
        if(!isdigit(*s)){
            return 0;
        }
        s++;
    }
    return 1;
}

/*reutrns the amount of parameters for a given r command*/
int count_params(char *s){
    if(!isR(s)){
        return 0;
    }
    if(strcmp(s,"move") == 0 || strcmp(s,"mvhi") == 0 || strcmp(s,"mvlo") == 0){
        return 2;
    }
    else{
        return 3;
    }
}

/*checks if a given command is an arithmatic or logical i command*/
int isarithorlog(char *s){
    return strcmp(s,"addi") == 0 || strcmp(s,"subi") == 0 || strcmp(s,"andi") == 0 || strcmp(s,"ori") == 0 || strcmp(s,"nori") == 0;
}

/*checks if a given command is a conditional i command*/
int iscond(char *s){
    return strcmp(s,"beq") == 0 || strcmp(s,"bne") == 0 || strcmp(s,"blt") == 0 || strcmp(s,"bgt") == 0;
}

/*checks if a given command is a memory loading i command*/
int isloading(char *s){
    return strcmp(s,"lb") == 0 || strcmp(s,"sb") == 0 || strcmp(s,"lw") == 0 || strcmp(s,"sw") == 0 || strcmp(s,"lh") == 0 || strcmp(s,"sh") == 0;
}

/*returns the opcode of given comand*/
int getopcode(char *s){
    int i;
    for(i = 0; i < sizeof(iCommands)/sizeof(iCommands[0]); i++){
        /*look for s in iCommands*/
        if(strcmp(iCommands[i].name, s)){
            /*when found we return the opcode*/
            return iCommands[i].opcode;
        }
    }
    return 0;
}

/*returns the funct of a given command*/
int getfunct(char *s){
    int i;
    for(i = 0; i < sizeof(rCommands)/sizeof(rCommands[0]); i++){
        /*look for s in rCommands*/
        if(strcmp(rCommands[i].name, s)){
            /*when found we return the funct*/
            return rCommands[i].funct;
        }
    }
    return 0;
}