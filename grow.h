/* Header file for the generic growing array
 */

#ifndef GROW_H
#define GROW_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct grow {
	void **arr;		// The array itself
	uint64_t elems;
	uint64_t alloc;
	bool dealloc;		// Should the elements be deallocated when destroying the array?
};

struct grow *grow_init(bool dealloc);
void grow_push(void *val, struct grow *g);
void *grow_pop(struct grow *g);
void grow_drop(struct grow *g);

#endif
