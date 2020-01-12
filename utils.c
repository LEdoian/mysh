/* Various utilities
 */

#include "utils.h"

#include <stdlib.h>

#include <err.h>

void *safe_alloc(size_t nbytes) {
	void *result = calloc(nbytes, 1);
	if (result == NULL) {
		err(3, "safe_alloc");
	}
	return result;
}

// To-be-implemented
void destroy_pipeline (struct command *pl) {
	destroy_command(pl);
}

void destroy_command (struct command *cmd) {

}
