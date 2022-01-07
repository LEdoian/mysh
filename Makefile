# This is cringeworthy, I hope to make it much nicer
# Still, it's just a bunch of files, so maybe not worth the trouble

HEADERS:=grow.h utils.h parse.h parse.tab.h constants.h run.h libtecla/libtecla.h workaround_uncaused_bugs.h
OBJS:=grow.o main.o parse.tab.o lex.yy.o utils.o run.o constants.o libtecla/libtecla.a
TAGTHESE:=grow.h grow.c utils.h utils.c parse.y parse.l constants.h constants.c main.c run.h run.c
LINKFLAGS:= -lcurses
CCFLAGS:= -I./libtecla

mysh: $(OBJS) Makefile
	gcc -g -o mysh $(OBJS) $(LINKFLAGS)

%.o: %.c $(HEADERS) Makefile
	gcc -g -Wall -Wextra $(CCFLAGS) -c $<

%.tab.h: %.y Makefile
	bison -d $<

# Probably redundant, but does not break anything
%.tab.c: %.y Makefile
	bison -d $<

lex.yy.c: parse.l parse.tab.h Makefile
	flex $<

libtecla/libtecla.h: libtecla/libtecla.a

libtecla/libtecla.a:
	{	curl 'https://sites.astro.caltech.edu/~mcs/tecla/libtecla.tar.gz' > libtecla.tar.gz ;\
		tar xavf libtecla.tar.gz ;\
		cd libtecla ;\
		./configure && make TARGET_LIBS=static ;}

tags: $(TAGTHESE)
	ctags $(TAGTHESE)
