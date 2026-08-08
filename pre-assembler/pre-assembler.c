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
int pre_assemble(FILE *f, FILE *write, char *name){
    char line[MAXLINE], word[MAXLINE], macroName[MAXLINE], *macroContent = NULL, commandType;
    int lp = 0, error = 0, lc = 0;
    size_t macroContentCap = 0;
    size_t macroContentLen = 0;
    int mcro, line_count = 0; /*initialize mcro flag and line count that is set to 0*/
    macro *mp, *macro_list = NULL, **macrotab = &macro_list; /*a macro pointer to use with lookup, the macro table, and the macro list (the macro list is the head of the list and the table is a pointer to it).*/
    while(fgets(line, MAXLINE, f)){ /*while f has more lines*/
        lc++;
        if(!lineend(line)){
            /* print concise message (err adds the "error in line %d:" prefix) */
            err("line is longer than 80 chars");
            error = 1;
            /* clear the rest of the (too long) line; stop if EOF encountered */
            while(fgets(line, MAXLINE, f)){
                if(lineend(line)) break;
            }
            continue;
        }
        lp = 0;
        if(!getword(word, line, &lp)){ /*use getword to put the first word in the line in word and check if line is empty*/
            continue; /*ignore*/
        }
        if(word[0] == ';'){ /*check if line is note*/
            continue; /*ignore*/
        }
        if((mp = lookup_macro(word, *macrotab))){ /*if first word is a macro name*/
            fputs(mp->defn, write); /*write the content of the macro to the file*/
            continue; 
        }
        if(strcmp(word, "mcro") == 0){ /*if first word is a macro decleration*/
            getword(macroName, line, &lp); /*place the next word in macroName*/
            if(gettype(macroName, &commandType)){ /*make sure new macro's name isnt a command*/
                err("a macro cannot have the same name as a command");
                error = 1;
                continue;
            }
            /*make sure macro name isnt an instruction*/
            if(strcmp(macroName, ".entry") == 0 || strcmp(macroName, ".extern") == 0 || strcmp(macroName, ".db") == 0 || strcmp(macroName, ".dh") == 0 || strcmp(macroName, ".dw") == 0){
                err("a macro cannot have the same name as an instruction");
                error = 1;
                continue;
            }
            if(lookup_macro(macroName, *macrotab)){ /*make sure new macro isnt already defined*/
                fprintf(stderr, "error in line %d: macro '%s' already defined\n",lc, macroName);
                error = 1;
                continue;
            }
            if(getword(word, line, &lp)){ /*look for chars after macro name*/
                err("chars found after macro decleration");
                error = 1;
                continue;
            }
            if(isdigit(macroName[0])){
                err("a macro cannot begin with a number");
                error = 1;
                continue;
            }
            mcro=1; /*we set mcro flag to 1*/
            while(fgets(line, MAXLINE, f)){ /*and start another loop to get the content of the macro*/ 
                lp = 0; /*reset line pointer for the new line*/
                getword(word,line, &lp); /*we get the first word in word*/
                if(word[0] == '\0' || strcmp(word, "mcroend") == 0){
                    if(getword(word, line, &lp)){ /*look for chars after macro ending*/
                        err("chars found after macro ending");
                        error = 1;
                        continue;
                    }
                    install_macro(macroName, macroContent, macrotab); /*add the macro to macrotab*/
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
                    /* append token and remaining line safely to macroContent */
                    if(!append_text(&macroContent, &macroContentCap, &macroContentLen, word)){
                        fprintf(stderr, "realloc error\n");
                        error = 1;
                        continue;
                    }
                    if(!append_text(&macroContent, &macroContentCap, &macroContentLen, line + lp)){
                        fprintf(stderr, "realloc error\n");
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
        /* write token and the rest of the line directly to avoid buffer overflow */
        fprintf(write, "%s%s", word, line + lp);
    }
    free(macroContent);/*reached EOF so we free the array*/
    if(error){ /*if we found errors*/
        fprintf(stderr,"errors detected in pre assembly. assembly will not continue\n");
        return 0;
    }
    fflush(write); /*flush and rewind the file to avoid issues with next passes*/
    rewind(write);
    if(!first_pass(write, name, macrotab)){
        return 0;
    }
    return 1;
}