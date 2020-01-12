/* Generic growing (non-shrinking) array implementation
 */

#include "grow.h"

#include <err.h>

#include "utils.h"

struct grow *grow_init(bool dealloc)
{
	struct grow *g = safe_alloc(sizeof(struct grow));
	// We know that safe_alloc zeroes the memory, so we only need to set the interesting parts.
	g->dealloc = dealloc;
	return g;
}

// Adds value val to the end of array g
void grow_push(void *val, struct grow *g)
{
	if (g == NULL) {
		errx(3, "grow: bad array");
	}
	if (g->alloc <= g->elems) {
		uint64_t new_size = g->alloc == 0 ? 1 : 2 * (g->alloc);
		void **new_arr = realloc(g->arr, new_size * sizeof(void *));
		if (new_arr == NULL) {
			err(3, "grow: realloc");
		}

		g->arr = new_arr;
		g->alloc = new_size;
	}
	g->arr[g->elems] = val;
	g->elems++;
}

// Pops the last element of the array
void *grow_pop(struct grow *g)
{
	if (g == NULL) {
		errx(3, "grow: bad array");
	}
	if (g->elems == 0) {
		return NULL;
	} else {
		g->elems--;
		return g->arr[g->elems + 1];
	}
}

// Deallocates the array, optionally also deallocating all its elements
void grow_drop(struct grow *g)
{
	if (g == NULL) {
		errx(3, "grow: bad array");
	}
	if (g->dealloc) {
		for (uint64_t i = 0; i < g->elems; i++) {
			free(g->arr[i]);
		}
	}
	free(g->arr);
	g->arr = NULL;		// Let it fail and not use free'd memory
	free(g);
	return;
}
