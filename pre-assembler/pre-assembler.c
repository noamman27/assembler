#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lib/table.h"
#define MAXLINE 81

static nlist *macrotab[HASHSIZE];
int pre_assemble(FILE *f);
void getword(char word[], char line[]);

int main(int argc, char *argv[]){

    int i;
    FILE *f;
    if(argc < 2){
        fprintf(stderr, "pre-assembler error: no given files");
    }
    for(i=1; i<argc; i++){
        f = fopen(argv[i],"rw");
        pre_assemble(f);
    }
    
}

int pre_assemble(FILE *f){
    char line[MAXLINE], word[MAXLINE], macroName[MAXLINE], macroContent[MAXLINE];
    unsigned hashed;
    int mcro;
    nlist *np;
    while(fgets(line, MAXLINE, f)){
        getword(word, line);
        if((np = lookup(word,macrotab))){
            /*TODO: add content of macro to file*/
        }
        hashed = hash(word, macrotab);
        getword(macroName, line);
        if(hashed == hash("mcro", macrotab)){
            mcro = 1;
        }
        while(fgets(line, MAXLINE, f)){ /*file ended*/
            getword(word,line);
            if(mcro && hash(word, macrotab) != "mcroend"){
                /*TODO: add current line to macroContent*/
            }
            /*TODO: remove mcroend from file*/
            install(macroName, macroContent, macrotab); /*add the macro to macrotab*/
            mcro = 0;
            continue;
        }
    }
    return 1;

}

void getword(char word[], char line[]){
    char c;
    int i = 0, j = 1;
    while(isspace((c = line[i++])))
        ;
    word[1] = c;
    while(!isspace((line[i++]))){
        word[++j] = c;
    }
    word[j] = '\0';
}