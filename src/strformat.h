
#ifndef DEF_STRFORMAT
#define DEF_STRFORMAT

typedef struct _strformat_symbs_t {
    char*  symbols;
    char** contents;
} strformat_symbs_t;

struct _strformat_t;
typedef struct _strformat_t strformat_t;

/* You must provide the list of symbols to be parsed. Each symbol is a single
 * letter, which, when preceded in a string by a %, will be replaced by another
 * value. symbols mustn't include any deduplicated letters : it will result in
 * an undefined behaviour.
 */
strformat_symbs_t* strformat_symbols(char* symbols);

/* Destroy an strformat symbols object. */
void strformat_symbols_destroy(strformat_symbs_t* sbs);

/* Set a value for a symbol. */
void strformat_set(strformat_symbs_t* sbs, char symbol, const char* value);

/* Parse a string to prepare the symbols to be placed on it. The
 * strformat_symbs_t object will be attached to it, so it must be destroyed
 * after the strformat_t object is destroyed.
 */
strformat_t* strformat_parse(strformat_symbs_t* sbs, const char* str);

/* Destroy an strformat object. Won't destroy the strformat_symbs_t object
 * attached to it.
 */
void strformat_destroy(strformat_t* fmt);

/* Get the string with the symbols replaced from an strformat_t object. The
 * string belong to the object, so it mustn't be free'd. It will remain valid
 * until the next call to this function or until the destruction of the object.
 * It will return an empty but valid string in cse of errors.
 */
const char* strformat_get(strformat_t* fmt);

#endif

