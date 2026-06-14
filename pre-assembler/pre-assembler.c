#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../main/assembler.h"
#include "../lib/utils.h"

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
    char line[MAXLINE], word[MAXLINE], macroName[MAXLINE];
    char *macroContent = NULL;
    size_t macroContentCap = 0;
    size_t macroContentLen = 0;
    char commandType;
    unsigned hashed; /*hashed representation for a word for comapring to keywords*/
    int mcro, line_count = 0; /*initialize mcro flag and line count that is set to 0*/
    nlist *np; 
    while(fgets(line, MAXLINE, f)){ /*while f has more lines*/
        getword(word, line); /*get the first word and put it in word*/
        if((np = lookup(word,macrotab))){ /*if first word is a macro name*/
            fputs(np->defn, write); /*write the content of the macro to the file*/
            continue; 
        }
        hashed = hash(word, macrotab); /*hash the word in macrotab*/
        if(hashed == hash("mcro", macrotab)){ /*if first word is a macro decleration*/
            getword(macroName, line); /*place the next word in macroName*/
            if(gettype(macroName, &commandType)){
                fprintf(stderr, "a macro cannot have the same name as a command");
                return 0;
            }
            if(!lookup(macroName, macrotab)){ /*make sure new macro isnt already defined*/
                mcro=1; /*we set mcro flag to 1*/
                while(fgets(line, MAXLINE, f)){ /*and start another loop to get the content of the macro*/ 
                    getword(word,line); /*we get the first word in word*/
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
                            return 0; 
                        }
                        continue;
                    }
                } 
                continue;
            }
            free(macroContent); /*if we got here the macro was already defined so we free macroContent to avoid a memory leak*/
            fprintf(stderr, "macro %s already defined", word); /*and print an error*/
            return 0;
        }
        strcat(word, line); /*strcat line to word since getword removes it from the line*/
        fputs(word, write); /*write word to the file since we added line to the rest of it*/
    }
    free(macroContent);/*reached EOF so we free the array*/
    return 1; /*and return 1*/

}