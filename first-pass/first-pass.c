#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
 
static nlist *symbletab[HASHSIZE]; /* head of the symble table linked list, starts empty */

int first_pass(FILE *input, FILE *output){
    char line[MAXLINE], word[MAXLINE], tmp;
    int isSym = 0, len;
    while(fgets(line,MAXLINE, input) != EOF){
        if(line[0] == ';' || whiteline(line)){
            continue;
        }
        len = getword(word, line);
        if(word[len-1] == ':'){ /*check if last char of word is :, if so its a label*/
            isSym = 1;
        }
        if(isDataStorageInst(word)) {
            if(isSym) {
                if(lookup(word, symbletab)){
                    fprintf(stderr, "label %s already defined", word);
                    return 0;
                }
                /*TODO: add to symble table with datat attribute*/
            }
        }

    }
    return 1;
}


int whiteline(char line[]){
    int i;
    for(i=0; i< sizeof(line); i++){
        if(!isspace(line[i])){
            return 0;
        }
    }
    return 1;
}