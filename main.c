#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <locale.h>
#include <unistd.h>

#include <libtecla.h>
#include <err.h>

#include "constants.h"
#include "utils.h"
#include "parse.h"

// FIXME: have "parse and run" here?

static char *gen_prompt(void)
{
	// We can obtain maximum path length from pathconf
	long path_max = pathconf("/", _PC_PATH_MAX);
	if (path_max == -1)
		path_max = 1024;	// Hopefully reasonable upper bound
	char path[path_max];
	if (getcwd(path, path_max) == NULL) {
		strcpy(path, "(broken getcwd)");	// We assume that path_max is not unreasonably short
	}
	char *prompt = safe_alloc(path_max + 10);
	sprintf(prompt, "mysh:%s$ ", path);	// Same as in the assignment
	return prompt;
}

static int repl(void)
{
	GetLine *gl = new_GetLine(MAX_LINE_LENGTH, MAX_HISTORY_LENGTH);
	last_retval = 0;

	// This loop is terminated by EOF
	// The "exit" command calls exit() on its own
	while (true) {
		// Read
		char *prompt = gen_prompt();

		// Ignore SIGINT (libtecla handles that itself)
		struct sigaction sa;
		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGINT, &sa, NULL) == -1) {
			err(3, "sigaction");
		}
		// The read itself
		char *line = gl_get_line(gl, prompt, NULL, -1);
		if (line == NULL) {
			GlReturnStatus status = gl_return_status(gl);
			if (status == GLR_SIGNAL)
				continue;	// This is only SIGINT, other signals kill us by default and we do not catch them
			if (status == GLR_EOF)
				break;
			if (status == GLR_ERROR)
				errx(3, "gl_read_line: %s",
				     gl_error_message(gl, NULL, 0));
		}
		if (strchr(line, '\0') == NULL) {
			// The line was too long
			// Warn the user and ask again
			// We don't care that the user cannot do what they want
			printf
			    ("WARNING: The line was too long, so nothing happened.\n");
			continue;
		}
		free(prompt);

		// Execute
		parse_and_run_str(line);
	}
	del_GetLine(gl);
	exit(last_retval);
}

int main(int argc, char **argv)
{
	setlocale(LC_CTYPE, "");
	// Just switch between possible methods of running mysh
	// It would be possible to use getopt, but it's too heavyweight for mysh
	if (argc <= 1)
		return repl();
	if (argc == 3 && strcmp(argv[1], "-c") == 0)
		return parse_and_run_str(argv[2]);
	if (argc == 2 && strcmp(argv[1], "") > 0)
		return parse_and_run_script(argv[1]);

	// If nothing holds, mysh was run in a strange way
	fprintf(stderr, "Bad usage\n"
		"Usage: \"%s [-c command]\" or \"%s scriptfile\"\n",
		argv[0], argv[0]);
	return BAD_USAGE_RETURN_VALUE;
}
