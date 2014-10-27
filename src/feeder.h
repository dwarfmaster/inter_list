
#ifndef DEF_FEEDER
#define DEF_FEEDER

#include <stdbool.h>
#include <stdlib.h>

/* This iterator allows going from one line to another one. */
typedef struct _feeder_iterator_t {
    /* The id of the line it is refering to. */
    size_t id;
    /* The virtual id of the line it is refering to (same as id for now). */
    size_t vid;
    /* Is the iterator valid. */
    bool valid;
} feeder_iterator_t;

/* Init and free the feeder. */
bool feeder_init();
void feeder_quit();

/* Set the feeding command : clear any previous content. */
bool feeder_set(const char* command);

/* Get the feeding command previously setted. */
const char* feeder_get();

/* Get the fd to watch. */
int feeder_fd();

/* Read data from the feeder and add it to the list. */
void feeder_update();

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

/* Decrement the iterator n times. If it goes before the beggining, it will be
 * set invalid.
 */
feeder_iterator_t feeder_prev(feeder_iterator_t* it, size_t n);

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

/* Hide/unhide lines in [id1,id2]. */
void feeder_hide(bool hide, size_t id1, size_t id2);
void feeder_hide_toggle(size_t id1, size_t id2);

#endif

