/* Various utilities
 */

#include "utils.h"

#include <stdlib.h>

#include <err.h>

void *safe_alloc(size_t nbytes)
{
	void *result = calloc(nbytes, 1);
	if (result == NULL) {
		err(3, "safe_alloc");
	}
	return result;
}

void destroy_pipeline(struct grow *pl)
{
	if (pl == NULL) return;
	for(int i = 0; i < pl->elems; i++) {
		destroy_command(pl->arr[i]);
	}
	grow_drop(pl);
}

void destroy_command(struct command *cmd)
{
	if (cmd == NULL) return;
	// As of now, the .in and .out fields are not ever assigned
	// We only need to deallocate the array
	grow_drop(cmd->args);
	free(cmd);
}

void destroy_redirect(struct redirect *redir)
{
	if (redir == NULL) return;
	free(redir->file);
	free(redir);
}
