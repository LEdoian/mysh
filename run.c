/* Functions to run commands and pipelines
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <signal.h>
#include <sys/wait.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parse.h"
#include "constants.h"
#include "grow.h"
#include "run.h"
#include "utils.h"

static pid_t run_command(struct command *cmd, int infd, int outfd);

static void change_directory(struct command *cmd, int infd, int outfd)
{
	char wd[2048];		// FIXME: hard-coded constant
	if (getcwd(wd, 2048) == NULL) {
		warn("getcwd");
		last_retval = RETVAL_ERROR;
		goto end;
	}
	char *target;
	switch (cmd->args->elems) {
	case 1:
		target = getenv("HOME");
		if (target == NULL) {
			// Note: this goes to stderr, which (acc. to the assignment) is
			// never redirected, so this needn't be changed.
			warnx("HOME not set");
			last_retval = 1;
			goto end;
		}
		break;
	case 2:
		if (strcmp(cmd->args->arr[1], "-") == 0) {
			target = getenv("OLDPWD");
			if (target == NULL) {
				warnx("OLDPWD not set");
				last_retval = 1;
				goto end;
			}
			dprintf(outfd, "%s\n", target);
		} else
			target = cmd->args->arr[1];
		break;
	default:
		warnx("Bad usage of cd (too many)");
		last_retval = 2;
		goto end;
	}

	if (chdir(target) == -1) {
		warn("chdir");
		last_retval = 1;
		goto end;
	}
	// The directory is changed, so we report success (even though environment
	// may be a bit broken)
	last_retval = 0;
	if (setenv("OLDPWD", wd, 1) == -1)
		warn("set OLDPWD");
	if (setenv("PWD", target, 1) == -1)
		warn("set PWD");

 end:
	// Close all fd's
	if (infd != STDIN_FILENO) {
		if (close(infd) == -1) {
			warn("close in");
			last_retval = RETVAL_ERROR;
		}
	}
	if (outfd != STDOUT_FILENO) {
		if (close(outfd) == -1) {
			warn("close out");
			last_retval = RETVAL_ERROR;
		}
	}
}

// The plan: create all the pipes here and then run the processes with
// run_command, which will close the fd's that should be closed
void run_pipeline(struct grow *pl)
{
	// Array of PIDs of childs, in order
	struct grow *pids = grow_init(true);
	// In and out directions are decided from the program's viewpoint, i.e.
	// out is the program's output, disregarding the fact that it's the
	// input to the pipe. This is consistent with the notion of (shell's)
	// stdin and stdout.
	int current_in = STDIN_FILENO;
	int current_out;	// Will be set at the begining of the loop
	// Spawn the all the processes with run_command, suplying right input and
	// output fd's
	for (uint64_t i = 0; i < pl->elems; i++) {
		int new_in;	// relevant only for pipelines
		// Set current_out
		if (i == pl->elems - 1)
			current_out = STDOUT_FILENO;
		else {
			// Create a pipe for the output
			int pipefds[2];
			if (pipe(pipefds) == -1) {
				warn("pipe failed");
				// Complete bail out: kill the pipeline, close all remaining fds
				for (uint64_t j = 0; j < pids->elems; j++)
					//FIXME: Is SIGPIPE the right signal?
					kill(*(pid_t *) pids->arr[j], SIGPIPE);
				if (current_in != STDIN_FILENO)
					close(current_in);
				if (current_out != STDOUT_FILENO)
					close(current_out);
				last_retval = RETVAL_ERROR;
				goto cleanup;
			}
			current_out = pipefds[1];
			new_in = pipefds[0];
		}
		// run_command makes sure the fd's will be closed, unless stdio.
		pid_t pid = run_command(pl->arr[i], current_in, current_out);
		if (pid == -1) {
			// NOTE: failing execution still closes the fd's, probably killing
			// the running pipeline by SIGPIPE.
			warnx("Command execution failed");
			goto cleanup;
		}
		// Save the PID for waiting
		if (pid == 0)
			continue;	// No PID
		pid_t *pid_p = safe_alloc(sizeof(pid_t));
		*pid_p = pid;
		grow_push(pid_p, pids);

		// Set current_in for next iteration
		current_in = new_in;
	}

	// Wait for all the children in order
	// Reason: the last status gets remembered for postprocessing
	if (pids->elems == 0)
		goto cleanup;
	pid_t result;
	int status;
	for (uint64_t i = 0; i < pids->elems; i++) {
		do {
			result = waitpid(*(pid_t *) pids->arr[i], &status, 0);
		} while ((result == -1 && errno == EINTR) ||
			 (result != -1
			  && (WIFSTOPPED(status) || WIFCONTINUED(status))
			 )
		    );

		if (result == -1) {
			warn("wait");
			last_retval = RETVAL_ERROR;
			continue;
		}
	}

	// Process the final result
	if (WIFEXITED(status)) {
		last_retval = WEXITSTATUS(status);
		goto cleanup;
	}
	if (WIFSIGNALED(status)) {
		fprintf(stderr, "Killed by signal %d.\n", WTERMSIG(status));
		last_retval = 128 + WTERMSIG(status);
		goto cleanup;
	}
	warnx("Something strange is happening");
	last_retval = RETVAL_ERROR;

 cleanup:
	grow_drop(pids);
}

static pid_t run_command(struct command *cmd, int infd, int outfd)
{
	// Hardcoded two builtins and error checking (not nice, sufficient)
	if (cmd == NULL || cmd->args == NULL || cmd->args->elems < 1) {
		warnx("bad cmd");
		if (infd != STDIN_FILENO)
			close(infd);
		if (outfd != STDOUT_FILENO)
			close(outfd);
		last_retval = RETVAL_ERROR;
		return -1;
	}
	if (strcmp((char *)cmd->args->arr[0], "exit") == 0)
		exit(last_retval);
	if (strcmp((char *)cmd->args->arr[0], "cd") == 0) {
		change_directory(cmd, infd, outfd);
		return 0;	//there is no PID
	}
	// We want to use cmd->args as argument to execvp
	grow_push(NULL, cmd->args);
	pid_t pid = fork();
	if (pid == -1) {
		warn("fork");
		if (infd != STDIN_FILENO)
			close(infd);
		if (outfd != STDOUT_FILENO)
			close(outfd);
		last_retval = RETVAL_ERROR;
		return -1;	//Error with getting PID
	}
	if (pid == 0) {
		// Child
		// Reset signals
		struct sigaction sa;
		sa.sa_handler = SIG_DFL;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGINT, &sa, NULL) == -1) {
			err(RETVAL_ERROR, "sigaction");
		}
		// dup2 file descriptors to stdio and close them
		if (dup2(infd, STDIN_FILENO) == -1)
			err(RETVAL_ERROR, "dup2 in");
		if (dup2(outfd, STDOUT_FILENO) == -1)
			err(RETVAL_ERROR, "dup2 out");
		if (infd != STDIN_FILENO) {
			if (close(infd) == -1)
				err(RETVAL_ERROR, "close in");
		}
		if (outfd != STDOUT_FILENO) {
			if (close(outfd) == -1)
				err(RETVAL_ERROR, "close out");
		}

		if (cmd->in != NULL) {
			struct redirect *redir = cmd->in;
			assert(redir->redirtype == IN);
			int fd = open(redir->file, O_RDONLY);
			if (fd == -1)
				err(RETVAL_ERROR, "open");
			if (dup2(fd, STDIN_FILENO) == -1)
				err(RETVAL_ERROR, "dup2");
			if (close(fd) == -1)
				err(RETVAL_ERROR, "close");
		}
		if (cmd->out != NULL) {
			struct redirect *redir = cmd->out;
			assert(redir->redirtype != IN);
			int flags = O_WRONLY | O_CREAT;
			if (redir->redirtype == APPEND) {
				flags |= O_APPEND;
			} else {
				flags |= O_TRUNC;
			}
			int fd = open(redir->file, flags, 0666);	// umask solves the rest
			if (fd == -1)
				err(RETVAL_ERROR, "open");
			if (dup2(fd, STDOUT_FILENO) == -1)
				err(RETVAL_ERROR, "dup2");
			if (close(fd) == -1)
				err(RETVAL_ERROR, "close");
		}
		// Execute
		execvp((char *)cmd->args->arr[0], (char **)cmd->args->arr);
		fprintf(stderr, "mysh: %s: %s\n", (char *)cmd->args->arr[0],
			strerror(errno));
		switch (errno) {
		case ENOENT:
			exit(127);
		case EACCES:
			exit(126);
		default:
			exit(RETVAL_ERROR);
			break;
		}
	}
	// Parent

	// Just close the file descriptors dedicated to inputs and outputs of child
	// and return child's PID
	if (infd != STDIN_FILENO)
		close(infd);
	if (outfd != STDOUT_FILENO)
		close(outfd);
	return pid;
}
