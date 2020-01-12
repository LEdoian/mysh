/* Functions to run commands
 */

#ifndef RUN_H
#define RUN_H

#include "parse.h"

void run_pipeline(struct command *pl);
void run_command(struct command *cmd);

#endif
