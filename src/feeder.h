
#ifndef DEF_FEEDER
#define DEF_FEEDER

#include <stdbool.h>
#include <stdlib.h>

/* This iterator allows going from one line to another one. */
typedef struct _feeder_iterator_t {
    /* The id of the line it is refering to. */
    size_t id;
    /* Is the iterator valid. */
    bool valid;
} feeder_iterator_t;

/* Init and free the feeder. */
bool feeder_init();
void feeder_quit();

/* Set the feeding command : clear any previous content. */
bool feeder_set(const char* command);

/* Get the fd to watch. */
int feeder_fd();

/* Read data from the feeder and add it to the list. */
void feeder_update();

/* Get the id of the selection. */
/* Deprecated : use iterators. */
size_t feeder_get_id();

/* Get the number of lines. */
/* Deprecated : use iterators. */
size_t feeder_get_lines();

/* Get the name of a line. Returns NULL if id is invalid. */
/* Deprecated : use iterators. */
const char* feeder_get_name(size_t id);

/* Get the text of a line. Returns NULL if id is invalid. */
/* Deprecated : use iterators. */
const char* feeder_get_text(size_t id);

/* Get the iterator to the first element. Returns an invalid iterator if there
 * is no lines.
 */
feeder_iterator_t feeder_begin();

/* Get the iterator to the last element. It is invalid and its id is the number
 * of lines.
 */
feeder_iterator_t feeder_end();

/* Increment the iterator n times. If it goes after the end, it will be set
 * invalid.
 */
feeder_iterator_t feeder_next(feeder_iterator_t* it, size_t n);

/* Get the text of the line pointed by an iterator. Returns NULL if it is
 * invalid
 */
const char* feeder_get_it_text(feeder_iterator_t it);

/* Get the name of the line pointed by an iterator. Returns NULL if it is
 * invalid.
 */
const char* feeder_get_it_name(feeder_iterator_t it);

/* Compare two iterators. The semantics are the same as strcmp. */
int feeder_it_cmp(feeder_iterator_t it1, feeder_iterator_t it2);

#endif

