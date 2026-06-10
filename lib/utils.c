#include <string.h>
#include <ctype.h>
#include "utils.h"

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

/*puts the first word in line inside word*/
void getword(char word[], char line[]){
    int i = 0, j = 0;

    while(line[i] != '\0' && isspace((unsigned char)line[i])){
        i++;
    }

    while(line[i] != '\0' && !isspace((unsigned char)line[i])){
        word[j++] = line[i++];
    }
    word[j] = '\0';

    memmove(line, line + i, strlen(line + i) + 1);
}