#include <string.h>
#include "utils.h"

int gettype(char *s, char *t){
    int i = 0;
    while (*s){
        if(i <= 8 && strcmp(*s, rCommands[i].name)){
            *t = rCommands[i].name;
            return 1;
        }
        if(i <= 4 && strcmp(*s, rCommands[i].name)){
            *t = rCommands[i].name;
            return 1;
        }
        if(i <= 16 && strcmp(*s, jCommands[i].name)){
            *t = jCommands[i].name;
            return 1;
        }
    }
    return 0;
}