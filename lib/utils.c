#include <string.h>
#include "utils.h"

int gettype(char *s, char *t){
    int i = 0;
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
}

int isR(char *s){
    int i = 0;
    while(*s){
        if(i++ < 8 && strcmp(*s, rCommands[i].name)){
            return 1;
        }
    }
    return 0;
}
int isI(char *s){
    int i = 0;
    while(*s){
        if(i++ < 8 && strcmp(*s, iCommands[i].name)){
            return 1;
        }
    }
    return 0;
}
int isJ(char *s){
    int i = 0;
    while(*s){
        if(i++ < 8 && strcmp(*s, jCommands[i].name)){
            return 1;
        }
    }
    return 0;
}