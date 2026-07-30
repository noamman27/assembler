#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "../lib/utils.h"

int main(int argc, char *argv[]){
    int i, j;
    char name[MAXLINE], amname[MAXLINE], *tmp;
    FILE *original, *pre_assembled;
    if(argc < 2){ /*make sure we were given at least one file*/
        fprintf(stderr, "error: not enough given params\n");
        return 1;
    }
    for(i=1; i<argc; i++){
        j = 0;
        tmp = argv[i];
        while(*tmp != '\0' && *tmp != '.'){
            if(j < MAXLINE-1) name[j++] = *tmp;
            tmp++;
        }
        name[j] = '\0'; /*terminate the basename*/
        if(*tmp != '.' || strcmp(tmp, ".as") != 0){ /*check the file extension to ensure .as*/
            fprintf(stderr, "error: the program must be given a file ending with .as\n");
            continue;
        }
        original = fopen(argv[i], "r"); /*open the input file*/
        /*prepare pre-assembled filename */
        strcpy(amname, name);
        strcat(amname, ".am"); /*add .am to the name*/
        pre_assembled = fopen(amname, "w+"); /*open the file for read/write (create/truncate)*/
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