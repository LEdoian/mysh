%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <err.h>

#include "constants.h"
#include "parse.h"
#include "run.h"
#include "utils.h"

//static struct grow *push_pipeline(struct command *cmd, struct grow *pl);
static struct command *push_arg(char *word, struct command *cmd);
static struct redirect *newredirect(enum redirtype type, char *filename);

bool end_reached = false;
int last_retval = 0;
%}

%union {
	struct redirect *redirect;
	struct command *cmd;
	char *word;
}

%token <word> WORD
%token REDIRAPPEND REDIROUT REDIRIN
%token SEMICOLON NEWLINE
%token EOI 0
%token PIPE

/* TODO: As of now, pipeline is the same as command */
%type <cmd> command pipeline
%type <redirect> redirect

%destructor { destroy_command($$); } <cmd>
%destructor { destroy_redirect($$); } <redirect>

%%
chunk: chunk pipeline end	{
		run_pipeline($2);
		destroy_pipeline($2);
		}
	| /**/
	;

end: NEWLINE
	| SEMICOLON
	| SEMICOLON NEWLINE
	| EOI
	;

pipeline: command	{ $$ = $1; }
	| pipeline PIPE command	{ 
		yyerror("Pipelines are not yet implemented");
		destroy_pipeline($1);
		// We don't need to care for the command, since we have only one token lokahead.
		YYABORT;
		$$ = NULL;
		(void) $3;
		}
redirect: REDIRAPPEND WORD	{$$ = newredirect(APPEND, $2);}
	| REDIRIN WORD	{$$ = newredirect(IN, $2);}
	| REDIROUT WORD	{$$ = newredirect(OUT, $2);}
	;

command: WORD { $$ = push_arg($1, NULL); }
	| command WORD { $$ = push_arg($2, $1); }
	| redirect { yyerror("Redirects are not yet implemented"); YYABORT; $$ = NULL; (void) $1; }
	| command redirect { yyerror("Redirects are not yet implemented"); destroy_command($1); YYABORT; $$ = NULL; (void) $2; }
	;

%%

/* To be uncommented
static struct grow *push_pipeline(struct command *cmd, struct grow *pl) {
	struct grow *result;
	if (pl == NULL) {
		result = grow_init(false);
	} else {
		result = pl;
	}
	grow_push(cmd, result);
	return result;
}*/

static struct command *push_arg(char *word, struct command *cmd) {
	struct command *result;
	if (cmd == NULL) {
		result = safe_alloc(sizeof(struct command));
		result->args = grow_init(true);
	} else {
		result = cmd;
	}
	// yytext is recycled, so we need to save the value.
	char *new_word = safe_alloc(strlen(word)+1);
	strcpy(new_word, word);
	grow_push(new_word, result->args);
	return result;
}

static struct redirect *newredirect(enum redirtype type, char *filename) {
	struct redirect *result = safe_alloc(sizeof(struct redirect));
	result->redirtype = type;
	result->file = filename;
	return result;
}
