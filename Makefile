# This is cringeworthy, I hope to make it much nicer
# Still, it's just a bunch of files, so maybe not worth the trouble

HEADERS:=grow.h utils.h parse.h parse.tab.h constants.h
OBJS:=grow.o main.o parse.tab.o lex.yy.o utils.o

%.o: %.c $(HEADERS)
	gcc -Wall -Wextra -c $<

%.tab.h: %.y
	bison -d $<

# Probably redundant, but does not break anything
%.tab.c: %y
	bison -d $<

lex.yy.c: parse.l parse.tab.h
	flex $<
