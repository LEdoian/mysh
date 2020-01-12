/* Functions to run commands and pipelines
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <signal.h>
#include <sys/wait.h>

#include "parse.h"
#include "constants.h"
#include "grow.h"
#include "run.h"
#include "utils.h"

static void change_directory(struct command *cmd) {
	char wd[2048];	// Fix hard-coded constant
	if (getcwd(wd, 2048) == NULL) {
		err(RETVAL_ERROR, "getcwd");
	}
	char *target;
	switch (cmd->args->elems) {
		case 1:
			target = getenv("HOME");
			if (target == NULL) {
				warnx("HOME not set");
				last_retval = 1;
				return;
			}
			break;
		case 2:
			if (strcmp(cmd->args->arr[1], "-") == 0) {
				target = getenv("OLDPWD");
				if (target == NULL) {
					warnx("OLDPWD not set");
					last_retval = 1;
					return;
				}
			} else target = cmd->args->arr[1];
			break;
		default:
			warnx("Bad usage of cd");
			last_retval = 2;
			return;
	}

	if (chdir(target) == -1) {
		warn("chdir");
		last_retval = 1;
		return;
	}
	// The directory is changed, so we report success (even though environment may be a bit broken)
	last_retval = 0;
	if (setenv("OLDPWD", wd, 1) == -1) warn("set OLDPWD");
	if (setenv("PWD", target, 1) == -1) warn("set PWD");
}

void run_pipeline(struct command *pl) {
	//TODO: implement pipelines
	run_command(pl);
}

void run_command(struct command *cmd) {
	if (cmd == NULL || cmd->args == NULL || cmd->args->elems < 1) errx(RETVAL_ERROR, "bad cmd");
	if (strcmp(cmd->args->arr[0], "exit") == 0) exit(last_retval);
	if (strcmp(cmd->args->arr[0], "cd") == 0) {
		change_directory(cmd);
		return;
	}

	// We want to use cmd->args as argument to execvp
	grow_push(NULL, cmd->args);
	pid_t pid = fork();
	if (pid == -1) {
		warn("fork");
		last_retval = RETVAL_ERROR;
		return;
	}
	if (pid == 0) {
		// Child
		// Reset signals
		sigset_t set;
		sigfillset(&set);
		if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1) err(RETVAL_ERROR, "sigprocmask");
		
		execvp((char *) cmd->args->arr[0], (char **) cmd->args->arr);
		if (errno == ENOENT) {
			fprintf(stderr, "mysh: %s: %s\n", (char *) cmd->args->arr[0], strerror(errno));
			exit(127);
		}
		if (errno == ENOEXEC) {
			fprintf(stderr, "mysh: %s: %s\n", (char *) cmd->args->arr[0], strerror(errno));
			exit(126);
		}
	}
	// Parent
	pid_t result;
	int status;
	do {
		result = waitpid(pid, &status, 0);
	} while (
		(result == -1 && errno == EINTR) ||
		(result != -1 &&
			( WIFSTOPPED(status) || WIFCONTINUED(status))
		)
	);

	if (result == -1) {
		warn("wait");
		last_retval = RETVAL_ERROR;
		return;
	}
	if (WIFEXITED(status)) {
		last_retval = WEXITSTATUS(status);
		return;
	}
	if (WIFSIGNALED(status)) {
		fprintf(stderr, "Killed by signal %d.\n", WTERMSIG(status));
		last_retval = 128 + WTERMSIG(status);
		return;
	}
	warnx("Something strange is happening");
	last_retval = RETVAL_ERROR;
}
