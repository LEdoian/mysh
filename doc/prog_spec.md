Mysh -- programmer's documentation
=================================

Installation procedure
----------------------

1. libtecla gets pulled and compiled
	- The requirement is that it compiles sucessfully in the lab at MS, which does not have libtecla installed.
2. parser is generated from `parse.l` and `parse.y`
3. everything is compiled
4. object files are linked into final executable mysh
	- libtecla is linked statically in order not to have to hack `LD_LIBRARY_PATH` when running mysh.

Overview
--------
There are several files that handle different aspects of the shell:

- `main.c` contains the REPL for interactive mode and runs the parser in the right mode (one-off/interactive/script)
- `parse.l` contains the lexer (in Flex) and feeds it in the right manner using either given line, or whole script
- `parse.y` contains the parser (in Bison), which populates structures defining redirects, commands and pipelines
- `parse.h` has the definition of structures for redirects, commands and pipelines and declaration of global state (since it is mostly parser-related)
- `run.c` implements the creation of pipelines, passing arguments, redirecting stdio and changing directories; `run.h` is its header file.
- `utils.c` has utilities for deallocating structures (`destroy_*`) and safe allocation (`safe_alloc`, returns cleared memory or tears down whole shell); `utils.h` is its header file.
- `constants.c` are constants used in the program. Also it serves as a ~~configuration~~ customization file for REPL and various return values. Again, has a header file.
- `grow.c` (and coresp. header) is an implementation of growing (non-shrinking) array which is used for everything.


Growing-array
-------------

The only dynamically-sized data structure in the shell (apart from the input library) is a growing array of void pointers, implemented by `grow.c`.

It is initialized by `grow_init` which is told whether the deallocator, `grow_drop`, should also automatically call `free()` on the elements (for convenience).

There are "methods" to add an element to the end of the array (`grow_push`) and to remove last element (`grow_pop`). The array itself is the `arr` field of the `struct grow` (representing the whole growing array), with current element count in `elems` and the current allocated size in `alloc`. `dealloc` flag is used to tell `grow_drop` whether to deallocate elements when destroying the array.

There are no safety guards/type checks on the array. It is assumed that the array is either used only locally, or that everyone knows what is be inside (which is a reasonable assumption for a project this small).

REPL
----

We rely on libtecla doing great job of providing a bit of user comfort. Maximum line and history lengths are set in `constants.c`. In order to support `^C` killing current line, we need to ignore SIGINT, so that only libtecla's handler is run.

Lexer
-----

The main job of the lexer is stripping comments and whitespace from the input, and recognising individual words, semicolons, redirection marks and pipes.

The lexer treats everything except implemented special characters (i.e. `#;|<>`) and (standard ASCII) whitespace as a regular character.

The `yyerror` function is written so it passes automated checks for NSWI015.

Since we need to know information from the lexer in the parser and it is written in Flex, it creates automatically global variable `yylineno`, which we use. Also, we call `yyerror` to report parsing errors both from the lexer and parser.

The `end_reached` flag signalises the feeder functions (`parse_and_run_*`) that whole input is read, so that they return the right value.

Parser
------

The parser in Bison recognises and runs "chunks" -- complete pipelines. Pipeline itself is a growing array of commands created by chaining commands one at a time (wherever separated by `|`). Commands are described by a growing array of their arguments (`args`) and two fields describing redirects of input and output (`in` and `out`), redirects have a filename (`file`) and type (`redirtype`, one of `IN`, `OUT` and `APPEND`) and are distinguished from command arguments by preceding redirection mark.

The functions in `parse.y` help create the structures mentioned above.

Running commands
----------------

This is the effective code of the shell.

The parser invokes the `run_pipeline` function, passing the array of commands to chain and run. This function creates pipes between each consecutive process and generates an array of all the file descriptors in the order of pipeline (i.e. stdin, write1, read1, write2, ....., read_n, stdout). Next, it calls `run_command` to actually run the command, given the right input and output file descriptors. After that, it waits for all the children.

The `run_command` function decides to either execute a builtin, or spawn an actual process. The `exit` builtin just calls `exit()` from libc, leaving all the cleanup on the OS; the `cd` is implemented by `change_directory()`, see below. For spawning processes, it forks, resets the SIGINT handler, first connects the given descriptors to stdio, followed by possibly overriding these descriptors with redirects (this way, redirects have precedence over pipelines), and finally runs the program. The parent just returns the child's PID, so it can be waited upon and its state inspected by `run_pipeline`.

To implement returning last commands return value as mysh's own return value, we keep this value in a global variable `last_retval`. (It's a global variable, since the bad has already been done by using Flex and the feeder function needs to get to know the last return value somehow, so they can return it after reading EOF.) Therefore, `run_pipeline` waits for all the processes in the pipeline and inspects the return value of the last one, setting the `last_retval`.

### Changing directories

To make things worse, there is one non-trivial builtin, namely `cd`, which also needs to have arguments, return value and write stuff to filedescriptors. Therefore, the `change_directory()` function gets the whole command structure and file descriptors, and has to close them and set last_retval before returning. Apart from that, it looks at its arguments and decides what to do, then executes the wanted action -- typically by resolving an environment variable, changing the directory, and writing the old into OLDPWD.


Final notes
-----------

I hope not to have upset Mr. Stallman that I say just Bison and not GNU Bison.

Also, I consider the header files as a part of this programmer's documentation, because they are readable and it would be useless to duplicate signatures here.
