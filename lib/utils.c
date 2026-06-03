#include <string.h>
#include "utils.h"

int gettype(char *s, char *t){
    int i = 0;
    while (*s){
        if(i <= 8 && strcmp(*s, rCommands[i].name)){
            *t = 'r';
            return 1;
        }
        if(i <= 4 && strcmp(*s, iCommands[i].name)){
            *t = 'i';
            return 1;
        }
        if(i <= 16 && strcmp(*s, jCommands[i].name)){
            *t = 'j';
            return 1;
        }
    }
    return 0;
}