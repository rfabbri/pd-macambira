# to use this Makefile for your project, you should only need to add your
# source files to the SOURCES variable.  For objects that only build on
# certain platforms, add those to the SOURCES line for the right platforms.

SOURCES = catch13~.c ext13.c filesize.c ftos.c kalashnikov.c mandelbrot~.c mandelbrot.c messages.c openpatch.c ossmixer.c piperead~.c pipewrite~.c receive13~.c receive13.c scramble~.c send13~.c send13.c sfread.c sfwrite13~.c streamin13~.c streamout13~.c strippath.c throw13~.c wavinfo.c 

LIBRARY_NAME = $(shell basename $(PWD))

CFLAGS = -DPD -I../../pd/src -Wall -W -g
LDFLAGS =  
LIBS = -lm

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  SOURCES += 
  EXTENSION = pd_darwin
  OS = macosx
  CFLAGS += -DUNIX -fPIC
  OPT_CFLAGS = -ftree-vectorize -ftree-vectorizer-verbose=2 -fast
  LDFLAGS += -bundle -undefined dynamic_lookup
  LIBS += -lc 
  STRIP = strip -x
 endif
ifeq ($(UNAME),Linux)
  SOURCES += cdplayer.c ossmixer.c promiscous~.c
  EXTENSION = pd_linux
  OS = linux
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  CFLAGS += -DUNIX -fPIC
  LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  SOURCES += 
  EXTENSION = dll
  OS = windows
  OPT_CFLAGS = -03 -funroll-loops -fomit-frame-pointer
  WINDOWS_HACKS = -D'O_NONBLOCK=1'
  CFLAGS += -mms-bitfields $(WINDOWS_HACKS)
  LDFLAGS += -shared
  LIBS += -L$(pd_src)/bin -L$(pd_src)/obj -lpd -lwsock32 -lkernel32 -luser32 -lgdi32
  STRIP = strip --strip-unneeded -R .note -R .comment
endif

CFLAGS += $(OPT_CFLAGS)



all: $(SOURCES:.c=.$(EXTENSION))

%.o: %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

%.$(EXTENSION): %.o
	$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o"  $(LIBS)
	chmod a-x "$*.$(EXTENSION)"
	$(STRIP) $*.$(EXTENSION)
	rm -f -- $*.o


clean:
	-rm -f -- $(SOURCES:.c=.o)
	-rm -f -- $(SOURCES:.c=.$(EXTENSION))


dist:
	cd .. && tar cjpf $(LIBRARY_NAME)-$(OS).tar.bz2 $(LIBRARY_NAME)

