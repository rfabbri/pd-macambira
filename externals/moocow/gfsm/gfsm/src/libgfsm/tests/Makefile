TARGETS = priotest

KNOWN_TARGETS = \
	priotest \
	seek3test \
	seek2test \
	offsettest \
	arctab-1test \
	seektest \
	gfsmlabdump \
	iotest \
	structtest \
	pathtest \
	memtest \
	memtest-general \
	ftest \
	flextest \
	calctest \
	flex2test \
	calc2test \
	flex3test \
	compretest \
	comprelex \
	compre2test


CC = gcc
#CC = ccmalloc --no-wrapper gcc
LD = $(CC)

CPPFLAGS = -I. -I.. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
CFLAGS    ?= -O2 -pipe
#CFLAGS   ?= -Wall -g
#CFLAGS   += -Wall -g
#CFLAGS    = -Wall -g
CFLAGS   += -Wall -Winline

LDFLAGS  = -L../libgfsm/.libs -L/usr/local/lib -L/usr/lib
LIBS     =  -lz -lglib-2.0 -lm
GFSMLIBS = ../.libs/libgfsm.a

LEX ?= flex
FLEX ?= flex
FLEXFLAGS ?=
BISON ?= bison
BISONFLAGS ?= --verbose

all: $(TARGETS)

##-- keep intermediate files
.SECONDARY:

seektest.o: seektest.c labprobs.h
seektest: seektest.o

##-- flex
%.lex.c %.lex.h: %.l
	$(FLEX) $(FLEXFLAGS) --outfile="$*.lex.c" --header-file="$*.lex.h" $^

calctest.lex.o: calctest.tab.h

calc2test.lex.o: calc2test.tab.h ../gfsmRegexCompiler.h

compretest.lex.o: compretest.tab.h ../gfsmRegexCompiler.h

comprelex.lex.o: compretest.lex.c compretest.lex.h ../gfsmRegexCompiler.h

##-- gfsmlabdump
gfsmlabdump: gfsmlabdump.o ../.libs/libgfsm.a
	$(LD) $(LDFLAGS) -o $@ $< $(GFSMLIBS) $(LIBS)

##-- bison
%.tab.c %.tab.h: %.y
	$(BISON) $(BISONFLAGS) --defines --file-prefix="$*" --name-prefix="$*_yy" $^

##-- flex+bison
calctest: calctest.lex.o calctest.tab.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

calc2test: calc2test.lex.o calc2test.tab.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

flex2test: flex2test.lex.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

flex3test: flex3test.lex.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

compretest: compretest.lex.o compretest.tab.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

comprelex: comprelex.lex.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

compre2test: compre2test.o $(GFSMLIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

##-- io
iotest: iotest.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

##-- .c -> .o
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $< 

##-- clean
clean:
	rm -f *.o *.lo *.tab.[ch] *.lex.[ch] $(KNOWN_TARGETS)


##-- executables
%test: %test.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

%-general: %-general.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)
