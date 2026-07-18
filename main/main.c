#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "../lib/utils.h"

int main(int argc, char *argv[]){
    int i;
    char name[MAXLINE];
    FILE *original, *pre_assembled;
    if(argc < 2){ /*make sure we were given at least one file*/
        fprintf(stderr, "error: not enough given params\n");
        return 1;
    }
    for(i=1; i<argc; i++){
        snprintf(name, sizeof(name), "%s-PP.txt", argv[i]);
        original = fopen(argv[i], "r"); /*open the input and output files*/
        pre_assembled = fopen(name, "w");
        if(original == NULL || pre_assembled == NULL){ /*check if we can open them*/
            fprintf(stderr, "error: failed to open input or output file\n"); /*if not we print and error to stderr*/
            return 1;
        }
        if(!pre_assemble(original, pre_assembled)){ /*call pre assembler and make sure no errors were given*/
            return 1; /* if errors were given we just return 1 since pre assembler prints errors*/
        }
        if(!first_pass(pre_assembled)){
            return 1;
        }
        if(!second_pass(pre_assembled, name)){
            return 1;
        }
        fclose(original); /*close the files*/
        fclose(pre_assembled);
        /*TODO: once we implement next passes call them*/
    }
    return 0;

}
