#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../main/assembler.h"
#include "../lib/utils.h"

nlist *macrotab;

/*appends text into buffer. makes sure that buffer has enough room,if not it reallocs based in capacity and length. returns 1 on success and 0 on failiure*/
static int append_text(char **buffer, size_t *capacity, size_t *length, const char *text){
    size_t text_len = strlen(text); /*get length of text*/
    size_t required = *length + text_len + 1; /*compute the required size for buffer*/
    char *tmp;

    if(required > *capacity){ /*if we need to realloc*/
        size_t new_capacity = (*capacity == 0) ? required : *capacity; /*make sure new_capacity cant be 0 since we will in an infinite loop*/
        while(new_capacity < required){
            new_capacity *= 2; /*increment new_capacity as much as needed*/
        }
        tmp = (char *) realloc(*buffer, new_capacity); /*realloc*/
        if(tmp == NULL){ /*ensure success*/
            return 0;
        }
        *buffer = tmp; /*update buffer and capacity*/
        *capacity = new_capacity;
    }

    memcpy(*buffer + *length, text, text_len + 1); /*put content in buffer and update length*/
    *length += text_len;
    return 1;
}


/*gets file *f and *write. reads f writes a pre assembled version to write. deployes all macros in f*/
int pre_assemble(FILE *f, FILE *write){
    char line[MAXLINE], word[MAXLINE], macroName[MAXLINE], *macroContent = NULL, commandType;
    int *lp = 0, error = 0;
    size_t macroContentCap = 0;
    size_t macroContentLen = 0;
    int mcro, line_count = 0; /*initialize mcro flag and line count that is set to 0*/
    nlist *np; 
    while(fgets(line, MAXLINE, f)){ /*while f has more lines*/
        *lp = 0;
        getword(word, line, lp); /*get the first word and put it in word*/
        if((np = lookup(word,macrotab))){ /*if first word is a macro name*/
            fputs(np->defn, write); /*write the content of the macro to the file*/
            continue; 
        }
        if(strcmp(word, "mcro") == 0){ /*if first word is a macro decleration*/
            getword(macroName, line, lp); /*place the next word in macroName*/
            if(gettype(macroName, &commandType)){ /*if the name of the macro is a command*/
                err("error: a macro cannot have the same name as a command");
                error = 1;
                continue;
            }
            if(lookup(macroName, macrotab)){ /*make sure new macro isnt already defined*/
                fprintf(stderr, "error: macro %s already defined", macroName);
                error = 1;
                continue;
            }
            mcro=1; /*we set mcro flag to 1*/
            while(fgets(line, MAXLINE, f)){ /*and start another loop to get the content of the macro*/ 
                getword(word,line, lp); /*we get the first word in word*/
                if(word[0] == '\0' || strcmp(word, "mcroend") == 0){
                    install(macroName, macroContent, macrotab); /*add the macro to macrotab*/
                    line_count = 0; /*reset line count*/
                    macroContentLen = 0; /*reset macro content length*/
                    if(macroContent != NULL){
                        macroContent[0] = '\0';
                    }
                    mcro = 0; /*reset mcro flag*/
                    break;
                }
                if(mcro){ /*and check if mcro flag is true*/
                    line_count++; /*increment line_count*/
                    strcat(word, line); /*add line to the end of word*/
                    if(!append_text(&macroContent, &macroContentCap, &macroContentLen, word)){ /*append the current line to the macro*/
                        fprintf(stderr, "realloc error"); /*if we got errors we print them to stderr*/
                        error = 1;
                        continue;
                    }
                    continue;
                }
            } 
            continue;
            free(macroContent); /*if we got here the macro was already defined so we free macroContent to avoid a memory leak*/
            fprintf(stderr, "macro %s already defined", word); /*and print an error*/
            return 0;
        }
        strcat(word, line); /*strcat line to word since getword removes it from the line*/
        fputs(word, write); /*write word to the file since we added line to the rest of it*/
    }
    free(macroContent);/*reached EOF so we free the array*/
    if(error){ /*if we found errors*/
        err("errors detected in pre assembly. assembly will not continue");
        return 0;
    }
    return 1;

}