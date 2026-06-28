#include "../main/assembler.h"
#include "../lib/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── externs from first-pass.c ─────────────────────────────────────────────
   REQUIRED CHANGES in first-pass.c before this compiles:
   1. remove 'static' from:  static Symble *symbletab
   2. split declaration and remove 'static' from ICF, DCF:
        static int cp=0, lp=0;
        int IC=IC_START, DC=0, ICF, DCF;                                     */
extern Symble *symbletab;
extern int     data_image[];
extern int     ICF;
extern int     DCF;

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

static ExtRef *ext_refs = NULL;

static void add_ext_ref(char *name, int address){
    ExtRef *e = (ExtRef *)malloc(sizeof(ExtRef));
    if(!e){ err("error: malloc failed in add_ext_ref"); return; }
    e->name    = strdup(name);
    e->address = address;
    e->next    = ext_refs;
    ext_refs   = e;
}

static void free_ext_refs(void){
    ExtRef *e = ext_refs;
    while(e){
        ExtRef *next = e->next;
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
static int add_entry_attr(Symble *s, char *sym_name){
    if(strcmp(s->attribute, "external") == 0){
        fprintf(stderr, "error: symbol '%s' is external and cannot be .entry\n", sym_name);
        return 0;
    }
    /* already marked as entry — do nothing */
    if(strstr(s->attribute, "entry") != NULL) return 1;

    /* build "original, entry" string */
    char *combined = (char *)malloc(strlen(s->attribute) + 9); /* ", entry\0" = 8 chars */
    if(!combined){ err("error: malloc failed in add_entry_attr"); return 0; }
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
static void write_ob(char *basename){
    char filename[MAXLINE];
    snprintf(filename, sizeof(filename), "%s.ob", basename);
    FILE *f = fopen(filename, "w");
    if(!f){ fprintf(stderr, "error: cannot open %s\n", filename); return; }

    /* header: number of instructions, number of data bytes */
    fprintf(f, "   %d %d\n", (ICF - IC_START) / 4, DCF);

    int i;
    /* code section: one int per instruction at data_image[IC_START], [IC_START+4] ... */
    for(i = IC_START; i < ICF; i += 4){
        unsigned int word = (unsigned int)data_image[i];
        fprintf(f, "%04d %08X\n", i, word);
    }
    /* data section: bytes stored at data_image[0..DCF-1],
       but printed at output addresses ICF .. ICF+DCF-1                      */
    for(i = 0; i < DCF; i++){
        fprintf(f, "%04d %02X\n", ICF + i, (unsigned char)data_image[i]);
    }
    fclose(f);
}

/* .ent file: one line per entry symbol — only written if entries exist      */
static void write_ent(char *basename){
    int has_entry = 0;
    Symble *s = symbletab;
    while(s){
        if(strstr(s->attribute, "entry") != NULL){ has_entry = 1; break; }
        s = s->next;
    }
    if(!has_entry) return;

    char filename[MAXLINE];
    snprintf(filename, sizeof(filename), "%s.ent", basename);
    FILE *f = fopen(filename, "w");
    if(!f){ fprintf(stderr, "error: cannot open %s\n", filename); return; }

    s = symbletab;
    while(s){
        if(strstr(s->attribute, "entry") != NULL)
            fprintf(f, "%s %04d\n", s->name, s->value);
        s = s->next;
    }
    fclose(f);
}

/* .ext file: one line per external reference — only written if refs exist   */
static void write_ext(char *basename){
    if(!ext_refs) return;

    char filename[MAXLINE];
    snprintf(filename, sizeof(filename), "%s.ext", basename);
    FILE *f = fopen(filename, "w");
    if(!f){ fprintf(stderr, "error: cannot open %s\n", filename); return; }

    ExtRef *e = ext_refs;
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
int second_pass(FILE *input, char *basename){
    char line[MAXLINE];
    char word[MAXLINE];
    char sym[MAXLINE];
    int  lp;
    int  len;
    int  error  = 0;
    int  ic     = IC_START; /* mirrors first-pass IC — tracks current instruction address */
    int  reg;
    int  immed;
    char type;

    rewind(input); /* step 1 setup: go back to beginning of pre-assembled file */

    while(fgets(line, MAXLINE, input) != NULL){ /* step 1: read next line     */

        lp = 0;

        /* step 2: skip comment lines */
        if(line[0] == ';') continue;

        /* step 2: skip empty lines */
        len = getword(word, line, &lp);
        if(len == 0) continue;

        /* step 3: skip label if present (ends with ':')                      */
        if(word[strlen(word) - 1] == ':'){
            lp = 0;
            len = getword(word, line, &lp);
            if(len == 0) continue; /* label-only line                         */
        }

        /* steps 4-6: .entry — add "entry" to the symbol's attributes        */
        if(strcmp(word, ".entry") == 0){
            lp = 0;
            if(getword(sym, line, &lp) == 0){
                err("error: no symbol given to .entry");
                error = 1;
                continue;
            }
            Symble *s = lookup_symble(sym, symbletab);
            if(!s){
                fprintf(stderr, "error: .entry symbol '%s' not defined\n", sym);
                error = 1;
                continue;
            }
            /* step 6: append "entry" to existing attribute (e.g. "data" → "data, entry") */
            if(!add_entry_attr(s, sym)){
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
            lp = 0;
            getparam(line, &lp, sym, &immed); /* skip $rs                     */
            getparam(line, &lp, sym, &immed); /* skip $rt                     */
            reg = getparam(line, &lp, sym, &immed); /* get label              */
            if(reg == SYM){
                Symble *s = lookup_symble(sym, symbletab);
                if(!s){
                    fprintf(stderr, "error: label '%s' not found\n", sym);
                    error = 1;
                } else {
                    I_BF ibf;
                    memcpy(&ibf, &data_image[ic], sizeof(ibf));
                    ibf.immed = (unsigned short)(s->value - ic); /* relative offset */
                    memcpy(&data_image[ic], &ibf, sizeof(ibf));
                }
            }
        }

        /* ── J-type (jmp/la/call — not hlt) ────────────────────────────────
           first pass left address = 0 for label operands.
           fill in the absolute address of the target symbol.                */
        if(type == 'j' && strcmp(word, "hlt") != 0){
            lp = 0;
            reg = getparam(line, &lp, sym, &immed);
            if(reg == SYM){
                Symble *s = lookup_symble(sym, symbletab);
                if(!s){
                    fprintf(stderr, "error: label '%s' not found\n", sym);
                    error = 1;
                } else {
                    J_BF jbf;
                    memcpy(&jbf, &data_image[ic], sizeof(jbf));
                    jbf.address = (unsigned int)s->value;
                    memcpy(&data_image[ic], &jbf, sizeof(jbf));
                    /* step 8: if external, record the reference              */
                    if(strcmp(s->attribute, "external") == 0)
                        add_ext_ref(sym, ic);
                }
            }
            /* reg > 0: register operand (jmp $N) — already encoded in pass 1 */
        }

        ic += 4; /* every instruction is 4 bytes                              */
    }

    /* step 9: stop if errors — no output files                               */
    if(error){
        free_ext_refs();
        return 0;
    }

    /* step 10: write output files                                             */
    write_ob(basename);
    write_ent(basename);
    write_ext(basename);

    free_ext_refs();
    return 1;
}