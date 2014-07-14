
#include "strformat.h"
#include <string.h>

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
}

void strformat_symbols_destroy(strformat_symbs_t* sbs)
{
}

void strformat_set(strformat_symbs_t* sbs, char symbol, char* value)
{
}

strformat_t* strformat_parse(strformat_symbs_t* sbs, const char* str)
{
}

void strformat_destroy(strformat_t* fmt)
{
}

const char* strformat_get(strformat_t* fmt)
{
}

