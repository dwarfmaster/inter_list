
#include "strformat.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define STRFORMAT_BUFFER_LENGTH 512

/* The basic elem in which the string parsed are decomposed. It is an internal
 * structure, which mustn't be manipulated par the users.
 */
struct _strformat_elem_t {
    /* The type of the elem :
     *  - FMT_TEXT means it is a static text.
     *  - FMT_SYMB means it must be replaced by a value.
     */
    enum {
        FMT_TEXT,
        FMT_SYMB,
    } type;
    /* The value, must be interpreted depending on type :
     *  - if it is FMT_TEXT, value is a (const char*) which value is the text
     *    to print. It must be free'd.
     *  - if it is FMT_SYMB, value is a (const char**) pointing to the value of
     *    the symbol. It mustn't be free'd, because it is a memory managed by
     *    the strformat_symb_t object.
     */
    void* value;
};

struct _strformat_t {
    /* The buffer for the string returned by strformat_get. */
    char buffer[STRFORMAT_BUFFER_LENGTH];
    /* The array of the elems of the parsed string. */
    struct _strformat_elem_t* elems;
    /* The number of elems. */
    size_t nbelems;
};

strformat_symbs_t* strformat_symbols(char* symbols)
{
    strformat_symbs_t* smb;
    size_t i;

    smb = malloc(sizeof(strformat_symbs_t));
    if(!smb)
        return NULL;

    smb->symbols  = strdup(symbols);
    if(!smb->symbols) {
        free(smb);
        return NULL;
    }

    smb->contents = malloc(sizeof(char*) * strlen(symbols));
    if(!smb->contents) {
        free(smb->symbols);
        free(smb);
        return NULL;
    }
    for(i = 0; i < strlen(symbols); ++i)
        smb->contents[i] = strdup("");

    return smb;
}

void strformat_symbols_destroy(strformat_symbs_t* sbs)
{
    size_t i;
    if(!sbs)
        return;

    for(i = 0; i < strlen(sbs->symbols); ++i)
        free(sbs->contents[i]);
    free(sbs->contents);
    free(sbs->symbols);
    free(sbs);
}

void strformat_set(strformat_symbs_t* sbs, char symbol, const char* value)
{
    size_t i;
    char* ch;
    if(!sbs)
        return;

    ch = strchr(sbs->symbols, symbol);
    if(!ch)
        return;
    i = ch - sbs->symbols;

    if(sbs->contents[i])
        free(sbs->contents[i]);
    if(value)
        sbs->contents[i] = strdup(value);
    else
        sbs->contents[i] = strdup("");
}

/* Return the new capacity. Returns 0 and free sbs->elems if an error happened.
 * TODO free all of the elems in case of error.
 */
static size_t add_elem(strformat_t* fmt,
        struct _strformat_elem_t elem,
        size_t capacity)
{
    if(fmt->nbelems >= capacity) {
        capacity += 10;
        fmt->elems = realloc(fmt->elems, capacity);
        if(!fmt->elems)
            return 0;
    }

    fmt->elems[fmt->nbelems] = elem;
    ++fmt->nbelems;

    return capacity;
}

strformat_t* strformat_parse(strformat_symbs_t* sbs, const char* str)
{
    strformat_t* fmt;
    struct _strformat_elem_t elem;
    size_t capacity;
    size_t i, size;
    const char* tbg;
    bool insymb;
    char c;

    if(!sbs || !str)
        return NULL;

    /* Initialisation and allocation. */
    fmt = malloc(sizeof(strformat_t));
    if(!fmt)
        return NULL;
    fmt->elems = malloc(sizeof(struct _strformat_elem_t) * 10);
    if(!fmt->elems) {
        free(fmt);
        return NULL;
    }
    capacity = 10;
    fmt->nbelems = 0;

    /* Parsing. */
    insymb = false;
    tbg = str;
    for(i = 0; i < strlen(str); ++i) {
        c = str[i];

        if(insymb) {
            insymb = false;
            elem.type = FMT_SYMB;
            tbg = strchr(sbs->symbols, c);
            if(!tbg)
                continue;
            elem.value = &(sbs->contents[tbg - sbs->symbols]);

            capacity = add_elem(fmt, elem, capacity);
            if(capacity == 0) {
                free(fmt);
                return NULL;
            }

            tbg = str + i + 1;
        }

        else if(c == '%') {
            insymb = true;
            if(tbg == (str + i))
                continue;

            size = str + i - tbg;
            elem.type  = FMT_TEXT;
            elem.value = malloc(size + 1);
            memcpy(elem.value, tbg, size);
            ((char*)elem.value)[size] = '\0';

            capacity = add_elem(fmt, elem, capacity);
            if(capacity == 0) {
                free(fmt);
                return NULL;
            }
        }
    }

    if(tbg < (str + i)) {
        size = str + i - tbg;
        elem.type  = FMT_TEXT;
        elem.value = malloc(size + 1);
        memcpy(elem.value, tbg, size);
        ((char*)elem.value)[size] = '\0';

        capacity = add_elem(fmt, elem, capacity);
        if(capacity == 0) {
            free(fmt);
            return NULL;
        }
    }

    return fmt;
}

void strformat_destroy(strformat_t* fmt)
{
    size_t i;
    if(!fmt)
        return;

    for(i = 0; i < fmt->nbelems; ++i) {
        if(fmt->elems[i].type == FMT_TEXT)
            free(fmt->elems[i].value);
    }
    free(fmt->elems);
    free(fmt);
}

const char* strformat_get(strformat_t* fmt)
{
    size_t length;
    char* str;
    const char* txt;
    size_t i;
    if(!fmt)
        return "";

    str = fmt->buffer;
    str[0] = '\0';
    length = STRFORMAT_BUFFER_LENGTH - 1;
    for(i = 0; i < fmt->nbelems; ++i) {
        if(fmt->elems[i].type == FMT_TEXT)
            txt = fmt->elems[i].value;
        else
            txt = *((char**)fmt->elems[i].value);
        strncat(str, txt, length);
        length -= strlen(txt);
    }

    return str;
}

