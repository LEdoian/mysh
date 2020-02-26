A simple shell: it will support pipelining, redirects of stdin and stdout to files, running commands interactively, non-interactively (i.e. sh -c "ls") and scripts (sh something.sh) and the interactive mode will not be dumb (i.e. will support basic editing features)

There won't be variables, wildcards nor control flow statements and possibly no support for single- and double-quoting strings.

It will be written in C with parser generated using Bison and Flex.