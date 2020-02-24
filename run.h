/* Functions to run commands
 */

#ifndef RUN_H
#define RUN_H

#include "parse.h"

void run_pipeline(struct grow *pl);
pid_t run_command(struct command *cmd, int infd, int outfd);

#endif
