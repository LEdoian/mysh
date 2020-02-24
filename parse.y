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

static struct grow *push_pipeline(struct command *cmd, struct grow *pl);
static struct command *push_arg(char *word, struct command *cmd);
static struct redirect *newredirect(enum redirtype type, char *filename);
static struct command *apply_redirect(struct command *cmd, struct redirect *redir);

bool end_reached = false;
int last_retval = 0;
%}

%union {
	struct redirect *redirect;
	struct grow *pipeline;
	struct command *cmd;
	char *word;
}

%token <word> WORD
%token REDIRAPPEND REDIROUT REDIRIN
%token SEMICOLON NEWLINE
%token EOI 0
%token PIPE

%type <cmd> command
%type <pipeline> pipeline
%type <redirect> redirect

%destructor { destroy_command($$); } <cmd>
%destructor { destroy_pipeline($$); } <pipeline>
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

pipeline: command	{ $$ = push_pipeline($1, NULL); }
	| pipeline PIPE command	{ $$ = push_pipeline($3, $1); }
	;

redirect: REDIRAPPEND WORD	{$$ = newredirect(APPEND, $2);}
	| REDIRIN WORD	{$$ = newredirect(IN, $2);}
	| REDIROUT WORD	{$$ = newredirect(OUT, $2);}
	;

command: WORD { $$ = push_arg($1, NULL); }
	| command WORD { $$ = push_arg($2, $1); }
	| redirect { $$ = apply_redirect(NULL, $1); }
	| command redirect { $$ = apply_redirect($1, $2); }
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
	// yytext is recycled, so we need to save the value.
	char *new_word = safe_alloc(strlen(word)+1);
	strcpy(new_word, word);
	grow_push(new_word, result->args);
	return result;
}

static struct redirect *newredirect(enum redirtype type, char *filename) {
	struct redirect *result = safe_alloc(sizeof(struct redirect));
	result->redirtype = type;
	char *newfn = safe_alloc(strlen(filename)+1);
	strcpy(newfn, filename);
	result->file = newfn;
	return result;
}

// Deallocate old redirect and put the new one in its place
static void redirect_clean_assign(struct redirect **old, struct redirect *new) {
	if (old == NULL) {
		warnx("redirect_clean_assign: Bad usage, target is NULL!");
		return;
	}
	destroy_redirect(*old);
	*old = new;
}

static struct command *apply_redirect(struct command *cmd, struct redirect *redir) {
	if (redir == NULL) return cmd;
	struct command *result;
	if (cmd == NULL) {
		result = (struct command *) safe_alloc(sizeof(struct command));
		// We have to perform basic initialization of the command
		result->args = grow_init(true);
	} else {
		result = cmd;
	}
	
	if (redir->redirtype == IN) {
		redirect_clean_assign(&(result->in), redir);
	} else /* We are changing output */ {
		redirect_clean_assign(&(result->out), redir);
	}

	return result;
}
