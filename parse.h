#pragma once

#include <stdbool.h>

#include "grow.h"

// Interface to the lexer
// XXX: I am not really sure that we want these to be public
extern int yylineno;
void yyerror(char *str);

// A structure that describes a command to run
struct command {
	struct grow *args;
	char *in;
	char *out;
	bool out_append;
};

// TODO: Structure to describe a pipeline

int parse_and_run_str(char *str, bool *should_exit);
int parse_and_run_script(char *filename);
