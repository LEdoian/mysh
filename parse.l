%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <err.h>

#include "parse.tab.h"
#include "constants.h"
#include "parse.h"
#include "run.h"
#include "utils.h"
%}

%option noyywrap nodefault yylineno

/* Letters that are not special in any way*/
LETTER	([^;| \t\n<>])
WHITESPACE	([ \t])

COMMENT	(#[^\n]*)

%%

{WHITESPACE}*	{ /* Ignore whitespace */  }
^{WHITESPACE}*{COMMENT}?\n	{ /* Ignore empty line */  }

	/* End of commands */
	/* Note that every comment ends a command */
;	|
;?{WHITESPACE}*{COMMENT}?\n	{ return END; }

{LETTER}+	{ yylval.word = yytext; return WORD; }

(<|>|>>){WHITESPACE}*{LETTER}+	{ yylval.word = yytext; return REDIRECT;}
"|"	{  return PIPE; }

<<EOF>>	{ end_reached = true; yyterminate(); }
. { yyerror("Bad character"); }

%%

void yyerror(char *msg) {
	fprintf(stderr, "error:%d: %s near unexpected token '%s'\n", yylineno, msg, yytext);
}

// Return value from last command is in the global last_retval
// The global boolean end_reached notifies us that whole input is parsed
int parse_and_run_str(char *str) {
	yylineno = 1;
	YY_BUFFER_STATE bs = yy_scan_string(str);
	yy_switch_to_buffer(bs);
	
	end_reached = false;
	while (end_reached == false) {
		int status = yyparse();
		if (status == 1) {
			last_retval = RETVAL_BAD_SYNTAX;
			break;
		}
	}
	return last_retval;
}

int parse_and_run_script(char *fn) {
	yylineno = 1;
	FILE *f = fopen(fn, "r");
	if (f == NULL) errx(3, "fopen");
	yyin = f;

	last_retval = 0;
	end_reached = false;
	while (end_reached == false) {
		int status = yyparse();
		if (status == 1) {
			last_retval = RETVAL_BAD_SYNTAX;
			break;
		}
	}
	return last_retval;
}
