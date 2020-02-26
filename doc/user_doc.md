Mysh
====
A simple shell, created partly as a solution to assignment in NSWI015.

Instalation
-----------
`make`, then copy `mysh` whereever you want. 

### Dependencies

For running: POSIXly compatible libc with BSD extensions (we use `err.h` for errors and warnings)

For compilation: `make`, `gcc`, `flex`, `bison`, `curl` and `tar` (GNU versions tested)

Usage
-----
Mysh supports three modes of running:

- script mode: invoked as `mysh path/to/a/script`, runs commands in the script in order
- one-off mode: invoked as `mysh -c command`, runs a single command
	+ The "single command" may have parameters and can also be a sequence of commands (i.e. `echo Hello; echo world`), provided you can pass that as a single argument containing semicolon, which is impossible to do from mysh. (See [Not implemented features] bellow)
- interactive mode: run as `mysh`
	+ It tries to be at least a bit comfortable, so arrows work, there is a bit of history (from current session only), `^C` gives a new prompt, `^D` exits, tab completes filenames.

Features
--------

- Commands can be run both from `$PATH` and with path specified
- Command sequences using semicolons: `ls;ls;` and `ls;ls` work. (A single semicolon by itself is not a valid command.)
- Comments: Wherever `#` appears, the rest of line is ignored
- `cd` builtin:
	+ Can be used to change to home directory with plain `cd`
	+ Can go back to previous directory with `cd -` (it prints the new directory to stdout)
	+ Can actually change directories: `cd /etc` (and it sets `OLDPWD` correctly)
	+ Current directory is shown in prompt in interactive mode
- `exit` builtin just exits the shell, returning the return value of last command run (or a special value in case of failure to run command)
- Killed processes generate a message to stderr
- Return values of failures:
	+ 3 for any errors with running commands
	+ 254 for syntax errors
	+ 2 for failures in `cd` (e.g. non-existent directories)
	+ 127 for non-existent commands
	+ 126 for commands not allowed to be run
	+ 128 + signal number for commands killed by a signal
- Stdin and stdout can be redirected to files with `>`, `>>`, `<` (no support for other redirects, not even stderr)
	+ Only last redirect of each stream counts, so `ls > /tmp/out1 >> /tmp/out2` will only create `/tmp/out2`
- Pipelining of commands (i.e. `sort /etc/passwd | grep -B1 root`)
	+ Redirects take precedence, so `ls > /tmp/out | wc` will show empty output from ls

Not implemented features
------------------------
Generally, whatever was not mentioned above is probably not implemented, which means:

- No control flow (`if`, `case`, functions and similar)
- No builtins except for `cd` and `exit`
- No support for variables, single and double quoting, wildcards, escape sequence parsing, multi-line input, ...
- The prompt cannot be customized
- Process groups are not used
- Only stdin and stdout can be redirected, using only `>`, `>>` and `<` redirections.

Mysh treats other special characters as normal characters, so `echo "I've   been at $OLDPWD"` just prints "I've been at $OLDPWD" (including the double-quotes).

Caveats
-------
It is not safe to use as a replacement for `/bin/sh` for two reasons:

1. Many scripts make use of variables, file descriptor redirections, quoting, etc. and would break in mysh
2. We use the `execvp` syscall for invoking the actual programs, which fallbacks to running `sh` in case of some errors, and mysh could loop infinitely on that.

Also, it is probably not a good idea to use mysh to run scripts, since there is no `set -e` or `&&`, so error handling is probably near to impossible.

`cd` changes directory even in pipelines, since subshells are not implemented.

Known bugs
----------

When `cd` is put at the end of a pipeline, its return value may not be the return value of the whole pipeline, since it does not behave like a process and is not waited upon. Seriously, don't put `cd` in a pipeline, it's stupid.
