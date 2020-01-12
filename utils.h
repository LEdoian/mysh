#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#include "parse.h"

void *safe_alloc(size_t nbytes);
void destroy_pipeline (struct command *pl);
void destroy_command (struct command *cmd);

#endif
