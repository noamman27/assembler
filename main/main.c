#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "../lib/utils.h"

int main(int argc, char *argv[]){
    int i;
    char name[MAXLINE];
    FILE *original, *pre_assembled;
    if(argc < 2){ /*make sure we were given at least one file*/
        fprintf(stderr, "not enough given params");
        return 1;
    }
    for(i=1; i<argc; i++){
        snprintf(name, sizeof(name), "%s-PP.txt", argv[i]);
        original = fopen(argv[i], "r");
        pre_assembled = fopen(name, "w");
        if(original == NULL || pre_assembled == NULL){
            fprintf(stderr, "failed to open input or output file\n");
            return 1;
        }
        if(!pre_assemble(original, pre_assembled)){ /*call pre assembler and make suer no errors were given*/
            return 1; /* if errors were given we just return 1 since pre_assemble prints errors*/
        }
        fclose(original);
        fclose(pre_assembled);
        /*TODO: once we implement next passes call them*/
    }
    return 0;

}
