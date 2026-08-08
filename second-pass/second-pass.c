#include "../main/assembler.h"
#include "../lib/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════════════════════
   EXTERNAL REFERENCE LIST
   tracks every J-type instruction that uses an external symbol label.
   written to the .ext output file.
═══════════════════════════════════════════════════════════════════════════ */
typedef struct ext_ref {
    char          *name;    /* external symbol name                           */
    int            address; /* address of the instruction that uses it        */
    struct ext_ref *next;
} ExtRef;


static void add_ext_ref(char *name, int address, int lc, ExtRef *ext_refs){
    ExtRef *e = (ExtRef *)malloc(sizeof(ExtRef));

    if(!e){ err("malloc failed in add_ext_ref"); return; }
    e->name    = dupstr(name);
    e->address = address;
    e->next    = ext_refs;
    ext_refs   = e;
}

static void free_ext_refs(ExtRef *ext_refs){
    ExtRef *e = ext_refs, *next = NULL;
    while(e){
        next = e->next;
        free(e->name);
        free(e);
        e = next;
    }
    ext_refs = NULL;
}

/* ─────────────────────────────────────────────────────────────────────────
   add_entry_attr: adds "entry" to a symbol's existing attribute.
   result matches the spec example:
     "code"     → "code, entry"
     "data"     → "data, entry"
     "external" → ERROR (external symbols cannot be entry points)
   the old attribute string is NOT freed because it may be a string literal.
───────────────────────────────────────────────────────────────────────── */
static int add_entry_attr(symbol *s, char *sym_name, int lc){
    char *combined;

    if(strcmp(s->attribute, "external") == 0){
        fprintf(stderr, "error in line %d: symbol '%s' is external and cannot be .entry\n",lc ,sym_name);
        return 0;
    }
    /* already marked as entry — do nothing */
    if(strstr(s->attribute, "entry") != NULL) return 1;

    /* build "original, entry" string */
    combined = (char *)malloc(strlen(s->attribute) + 9); /* ", entry\0" = 8 chars */
    if(!combined){ err("malloc failed in add_entry_attr"); return 0; }
    sprintf(combined, "%s, entry", s->attribute);
    s->attribute = combined;   /* replace with combined — original was a literal, safe to discard */
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
   OUTPUT FILE WRITERS  (step 10)

   Memory layout after both passes:
     code → data_image[IC_START .. ICF-1]  (one int per instruction, step 4)
     data → data_image[0 .. DCF-1]         (DC started at 0, stored byte-by-byte)

   Output addresses:
     code → IC_START .. ICF-1
     data → ICF .. ICF+DCF-1
     (data symbol values already shifted by ICF in first-pass step 19)
═══════════════════════════════════════════════════════════════════════════ */
static void write_ob(char *basename, int *code_image, int icf, int dcf, char *data_image){
    int i, j;
    char filename[MAXLINE];
    FILE *f;

    sprintf(filename, "%s.ob", basename);
    f = fopen(filename, "w");
    if(!f){ fprintf(stderr, "error cannot open %s\n", filename); return; }

    /* header: number of instructions, number of data bytes */
    fprintf(f, "%d %d\n", icf - IC_START , dcf);
    /* code section: one int per instruction at code_image[0], [1] ... */
    for(i = 0; i < (icf - IC_START) / 4; i++){
        unsigned int word = (unsigned int)code_image[i];
        fprintf(f, "%04d %02X %02X %02X %02X\n",
        i * 4 + IC_START,
        word & 0xFF, /*shift and mask to get correct value*/
        (word >> 8) & 0xFF,
        (word >> 16) & 0xFF,
        (word >> 24) & 0xFF);
    }
    /* data section: bytes stored at data_image[0..DCF-1],
       but printed at output addresses icf .. icf+dcf-1                      */
    for(i = 0; i < dcf; i+=4){
        fprintf(f, "%04d ", icf + i);
        for(j = 0; j < 4 && i + j < dcf; j++){
            fprintf(f, "%02X", (unsigned char)data_image[i + j]);
            if(i + j + 1 < dcf && j < 3) fputc(' ', f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

/* .ent file: one line per entry symbol — only written if entries exist      */
static void write_ent(char *basename, symbol *symboltab){
    int has_entry = 0;
    symbol *s = symboltab;
    char filename[MAXLINE];
    FILE *f;

    while(s){
        if(strstr(s->attribute, "entry") != NULL){ has_entry = 1; break; }
        s = s->next;
    }
    if(!has_entry) return;

    sprintf(filename, "%s.ent", basename);
    f = fopen(filename, "w");
    if(!f){ fprintf(stderr, "error: cannot open %s\n", filename); return; }

    s = symboltab;
    while(s){
        if(strstr(s->attribute, "entry") != NULL)
            fprintf(f, "%s %04d\n", s->name, s->value);
        s = s->next;
    }
    fclose(f);
}
/* .ext file: one line per external reference — only written if refs exist   */
static void write_ext(char *basename, ExtRef *ext_refs){
    char filename[MAXLINE];
    FILE *f;
    ExtRef *e;

    if(!ext_refs) return;

    sprintf(filename, "%s.ext", basename);
    f = fopen(filename, "w");
    if(!f){ fprintf(stderr, "error: cannot open %s\n", filename); return; }

    e = ext_refs;
    while(e){
        fprintf(f, "%s %04d\n", e->name, e->address);
        e = e->next;
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
   SECOND PASS — 10 steps from the spec
   input    — pre-assembled FILE* (rewound internally to start)
   basename — filename base for output files (.ob / .ent / .ext)
   returns 1 on success, 0 if any error found
═══════════════════════════════════════════════════════════════════════════ */
int second_pass(FILE *input, char *basename, int *code_image, int icf, int dcf, symbol *symboltab, char *data_image){
    char line[MAXLINE];
    char word[MAXLINE];
    char sym[MAXLINE];
    int  lp;
    int  len;
    int  error  = 0;
    int  ic     = IC_START; /* mirrors first-pass IC — tracks current instruction address */
    int  ip     = 0;        /*instuction pointer - same as in first pass*/
    int  reg;
    int  immed;
    int lc      = 0;
    char type;
    symbol *s;
    ExtRef *ext_refs = NULL;

    rewind(input); /* step 1 setup: go back to beginning of pre-assembled file */

    while(fgets(line, MAXLINE, input) != NULL){ /* step 1: read next line     */
        lc++;
        lp = 0;

        /* step 2: skip comment lines */
        if(line[0] == ';') continue;

        /* step 2: skip empty lines */
        len = getword(word, line, &lp);
        if(len == 0) continue;

        /* step 3: skip label if present (ends with ':')                      */
        if(word[strlen(word) - 1] == ':'){
            len = getword(word, line, &lp);
            if(len == 0) continue; /* label-only line                         */
        }

        /* steps 4-6: .entry — add "entry" to the symbol's attributes        */
        if(strcmp(word, ".entry") == 0){
            if(getword(sym, line, &lp) == 0){
                err("no symbol given to .entry");
                error = 1;
                continue;
            }
            s = lookup_symbol(sym, symboltab);
            if(!s){
                fprintf(stderr, "error in line %d: .entry symbol '%s' not defined\n",lc, sym);
                error = 1;
                continue;
            }
            /* step 6: append "entry" to existing attribute (e.g. "data" → "data, entry") */
            if(!add_entry_attr(s, sym, lc)){
                error = 1;
            }
            continue;
        }

        /* step 4: skip all other directives — handled entirely in first pass */
        if(word[0] == '.') continue;

        /* step 7: instruction line — complete encoding left unfinished in pass 1 */
        if(!gettype(word, &type)) continue;

        /* ── I-type conditional (beq/bne/blt/bgt) ──────────────────────────
           first pass left immed = 0.
           compute: immed = target_address - current_ic  (relative offset)   */
        if(type == 'i' && iscond(word)){
            getparam(line, &lp, sym, &immed, lc); /* skip $rs                     */
            getparam(line, &lp, sym, &immed, lc); /* skip $rt                     */
            reg = getparam(line, &lp, sym, &immed, lc); /* get label              */
            if(reg == SYM){
                s = lookup_symbol(sym, symboltab);
                if(!s){
                        fprintf(stderr, "error in line %d: label '%s' not found\n",lc, sym);
                        
                    error = 1;
                } else {
                    I_BF ibf;
                    memcpy(&ibf, &code_image[ip], sizeof(ibf));
                    ibf.immed = (unsigned short)(s->value - ic); /* relative offset in bytes */
                    memcpy(&code_image[ip], &ibf, sizeof(ibf));
                }
            }
        }

        /* ── J-type (jmp/la/call — not hlt) ────────────────────────────────
           first pass left address = 0 for label operands.
           fill in the absolute address of the target symbol.                */
        if(type == 'j' && strcmp(word, "hlt") != 0){
            reg = getparam(line, &lp, sym, &immed, lc);
            if(reg == SYM){
                symbol *s = lookup_symbol(sym, symboltab);
                if(!s){
                    fprintf(stderr, "error on line %d: label '%s' not found\n",lc ,sym);
                    error = 1;
                } else {
                    J_BF jbf;
                    memcpy(&jbf, &code_image[ip], sizeof(jbf));
                    jbf.address = (unsigned int)s->value;
                    memcpy(&code_image[ip], &jbf, sizeof(jbf));
                    /* step 8: if external, record the reference              */
                    if(strcmp(s->attribute, "external") == 0)
                        add_ext_ref(sym, ic, lc, ext_refs);
                }
            }
            /* reg > 0: register operand (jmp $N) — already encoded in pass 1 */
        }

        ic += 4; /* every instruction is 4 bytes                              */
        ip += 1;
    }

    /* step 9: stop if errors — no output files                               */
    if(error){
        fprintf(stderr,"errors detected in second pass. no output files will be created\n");
        free_ext_refs(ext_refs);
        free_symbols(symboltab);
        return 0;
    }

    /* step 10: write output files                                             */
    write_ob(basename, code_image, icf, dcf, data_image);
    write_ent(basename, symboltab);
    write_ext(basename, ext_refs);

    free_ext_refs(ext_refs);
    free_symbols(symboltab);
    return 1;
}