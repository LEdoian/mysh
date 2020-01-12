%{
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "parse.tab.h"
%}

%option noyywrap nodefault yylineno

/* Letters that are not special in any way*/
LETTER	([^;| \t\n<>])
WHITESPACE	([ \t])

COMMENT	(#.*)

%%

{WHITESPACE}*	{ /* Ignore whitespace */ }


	/* End of commands */
	/* Note that every comment ends a command */
;{WHITESPACE}*	|
;?{WHITESPACE}*{COMMENT}?\n	{ return END; }

{LETTER}+	{ return WORD; }

(<|>|>>){WHITESPACE}*{LETTER}+	{return REDIRECT;}
"|"	{ return PIPE; }

. { yyerror("Bad character, somehow"); }

%%
