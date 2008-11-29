TARGETS = mod1test libmod1a.so libmod1b.so
KNOWN_TARGETS = mod1test libmod1a.so libmod1b.so

CC = gcc
#CC = ccmalloc --no-wrapper gcc
LD = $(CC)

CPPFLAGS = $(shell pkg-config --cflags-only-I gmodule-2.0)
#CFLAGS   ?= -O2 -pipe
#CFLAGS   ?= -Wall -g
#CFLAGS   += -Wall -g
CFLAGS    ?= -g
CFLAGS    += -Wall -fPIC -DPIC

LDFLAGS  = $(shell pkg-config --libs-only-L gmodule-2.0)
LIBS     = $(shell pkg-config --libs-only-l gmodule-2.0)  -lm

LDFLAGS_MODULE = -shared $(LDFLAGS)

all: $(TARGETS)

##-- keep intermediate files
.SECONDARY:

##-- PATTERN: .c -> .o
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $< 

##-- clean
clean:
	rm -f *.o *.lo *.tab.[ch] *.lex.[ch] $(KNOWN_TARGETS)

##-- modules
lib%.so: %.o
	$(LD) $(LDFLAGS_MODULE) -o $@ $^ $(LIBS)

##-- Executables
%test: %test.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
