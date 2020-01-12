#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>

#include "grow.h"

// Interface to the lexer
extern int yylineno;
void yyerror(char *str);
extern int yylex (void); //?

// Since the lexer and parser rely on global variables, we use them to extract their status
// (It is also hinted in the book that it often works like this)
extern bool end_reached;
extern int last_retval;

// A structure that describes a command to run
struct command {
	struct grow *args;
	char *in;
	char *out;
	bool out_append;
};

// TODO: Structure to describe a pipeline

int parse_and_run_str(char *str);
int parse_and_run_script(char *filename);

#endif
