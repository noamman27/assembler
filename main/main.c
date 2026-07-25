#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "../lib/utils.h"

int main(int argc, char *argv[]){
    int i, j = 0;
    char name[MAXLINE];
    FILE *original, *pre_assembled;
    if(argc < 2){ /*make sure we were given at least one file*/
        fprintf(stderr, "error: not enough given params\n");
        return 1;
    }
    for(i=1; i<argc; i++){
        while(*argv[i] != '.'){
            name[j++] = *argv[i];
            argv[i]++;
        }
        original = fopen(argv[i], "r"); /*open the input and output files*/
        pre_assembled = fopen(strcat(name, ".am"), "rw");
        if(original == NULL || pre_assembled == NULL){ /*check if we can open them*/
            fprintf(stderr, "error: failed to open input or output file\n"); /*if not we print and error to stderr*/
            return 1;
        }
        if(!pre_assemble(original, pre_assembled, name)){ /*call pre assembler and make sure no errors were given*/
            return 1; /* if errors were given we just return 1 since pre assembler prints errors*/
        }
        /*if we got to this point it means the assembly didnt encounter any errors, since pre assembler calls first pass which calls second pass*/
        fclose(original); /*close the files*/
        fclose(pre_assembled);
    }
    j = 0;
    return 0;
}
