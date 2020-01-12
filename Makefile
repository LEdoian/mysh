# This is cringeworthy, I hope to make it much nicer
# Still, it's just a bunch of files, so maybe not worth the trouble

HEADERS:=grow.h utils.h parse.h parse.tab.h constants.h run.h
OBJS:=grow.o main.o parse.tab.o lex.yy.o utils.o run.o constants.o
LINKFLAGS:= -ltecla

mysh: $(OBJS) Makefile
	gcc -g $(LINKFLAGS) -o mysh $(OBJS)

%.o: %.c $(HEADERS) Makefile
	gcc -g -Wall -Wextra -c $<

%.tab.h: %.y Makefile
	bison -d $<

# Probably redundant, but does not break anything
%.tab.c: %y Makefile
	bison -d $<

lex.yy.c: parse.l parse.tab.h Makefile
	flex $<
