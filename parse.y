%{
#include <string.h>
#include <unistd.h>

#include <err.h>

#include "parse.h"
#include "utils.h"

static struct grow *push_pipeline(struct command *cmd, struct grow *pl);
static struct command *push_arg(char *word, struct command *cmd);

%}

%union {
	struct command *cmd;
	struct grow *list;
	char *word;
}

%token <word> WORD REDIRECT
%token END
%token PIPE

/* TODO: As of now, pipeline is the same as command */
%type <cmd> command pipeline
%type <list> pipeline_list

%%

pipeline_list: END /* Probably can be nothing here */	{ $$ = NULL; }
	| pipeline_list pipeline END	{ $$ = push_pipeline($2, $1); }
	;

pipeline: command	{ $$ = $1; }
	| pipeline PIPE command	{ 
		yyerror("Pipelines are not yet implemented");
		}
	;

command: WORD { $$ = push_arg($1, NULL); }
	| command WORD { $$ = push_arg($2, $1); }
	| REDIRECT { yyerror("Redirects are not yet implemented"); }
	| command REDIRECT { yyerror("Redirects are not yet implemented"); }
	;

%%

static struct grow *push_pipeline(struct command *cmd, struct grow *pl) {
	struct grow *result;
	if (pl == NULL) {
		result = grow_init(false);
	} else {
		result = pl;
	}
	grow_push(cmd, result);
	return result;
}

static struct command *push_arg(char *word, struct command *cmd) {
	struct command *result;
	if (cmd == NULL) {
		result = safe_alloc(sizeof(struct command));
		result->args = grow_init(true);
	} else {
		result = cmd;
	}
	grow_push(word, cmd->args);
	return result;
}
