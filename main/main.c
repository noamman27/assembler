#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "../lib/utils.h"

int main(int argc, char *argv[]){
    int i, j = 0;
    char name[MAXLINE], *tmp;
    FILE *original, *pre_assembled;
    if(argc < 2){ /*make sure we were given at least one file*/
        fprintf(stderr, "error: not enough given params\n");
        return 1;
    }
    for(i=1; i<argc; i++){
        tmp = argv[i];
        while(*argv[i] != '.'){
            name[j++] = *tmp;
            tmp++;
        }
        if(strcmp(tmp, "as")){ /*check the file extension to ensuer we were given a .as file*/
            fprintf(stderr, "error: the program must be given a file ending with .as");
            continue;
        }
        name[j - 1] = '\0';/*close the string without including the .*/
        original = fopen(argv[i], "r"); /*open the input and output files*/
        strcat(name, ".am"); /*add .am to the name*/
        pre_assembled = fopen(name, "rw"); /*open the file as rw since we need to read and write from it*/
        name[j-1] = '\0'; /*remove the .am since we need the basename for assembly*/
        if(original == NULL || pre_assembled == NULL){ /*check if we can open them*/
            fprintf(stderr, "error: failed to open input or output file\n"); /*if not we print and error to stderr*/
            continue;
        }
        if(!pre_assemble(original, pre_assembled, name)){ /*call pre assembler and make sure no errors were given*/
            continue; /* if errors were given we just continue to next file since pre assembler prints errors*/
        }
        /*if we got to this point it means the assembly didnt encounter any errors, since pre assembler calls first pass which calls second pass*/
        fclose(original); /*so we close the files*/
        fclose(pre_assembled);
    }
    j = 0;
    return 0;
}