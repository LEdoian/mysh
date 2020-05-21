#ifndef PARSE_H
#define PARSE_H

#define YY_NO_UNPUT
#define YY_NO_INPUT

#include <stdbool.h>

#include "grow.h"

// Interface to the lexer
extern int yylineno;
void yyerror(char *str);
extern int yylex(void);		//?

// Since the lexer and parser rely on global variables, we use them to extract their status
// (It is also hinted in the book that it often works like this)
extern bool end_reached;
extern int last_retval;

// Possible types of redirect
enum redirtype { IN, OUT, APPEND };

// A struct to describe a redirect
struct redirect {
	enum redirtype redirtype;
	char *file;
};

// A structure that describes a command to run
struct command {
	struct grow *args;
	struct redirect *in;
	struct redirect *out;
};

int parse_and_run_str(char *str);
int parse_and_run_script(char *filename);

#endif
