#include "../main/assembler.h" 
#include "../lib/utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
 
static nlist *symbletab[HASHSIZE]; /* head of the symble table linked list, starts empty */
static int code_image[MAX_CODE];
static int data_image[MAX_DATA];
static int IC = IC_START;
static int DC = 0;

int first_pass(FILE *input, FILE *output){
    char line[MAXLINE], word[MAXLINE], tmp, type;
    int isSym = 0, len;
    R_BF rc_bf;
    I_BF ic_bf;
    J_BF jc_bf;
    while(fgets(line,MAXLINE, input) != EOF){
        if(line[0] == ';' || whiteline(line)){
            continue;
        }
        len = getword(word, line);
        if(word[len-1] == ':'){ /*check if last char of word is :, if so its a label*/
            isSym = 1;
        }
        if(strcmp(word, ".dh") || strcmp(word, ".dw") || strcmp(word, ".db") || strcmp(word, ".dh") || strcmp(word, ".asciz")) {
            if(isSym) {
                if(lookup(word, symbletab)){
                    fprintf(stderr, "label %s already defined", word);
                    return 0;
                }
                /*TODO: add to symble table with data attribute*/
            }
        }
        if(!gettype(word,type)){
            fprintf(stderr, "err");
        }
        switch (type)
        {
        case 'r':
            if(param_count(word) == 2){
                rc_bf.rs = 0;
                if(!(rc_bf.rd = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.rt = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.funct = getfunct(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.opcode = getopcode(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
            } else {
                if(!(rc_bf.rd = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;                    
                }
                if(!(rc_bf.rd = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.rt = getreg(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.funct = getfunct(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
                if(!(rc_bf.opcode = getopcode(line))){
                    fprintf(stderr,"input error");
                    return 0;
                }
            }
            break;
        default:
            break;
        }

    }
    return 1;
}