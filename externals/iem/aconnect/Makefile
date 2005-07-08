LIBNAME    =aconnect

PREFIX     =/usr/local/lib/pd

INSTALL_BIN=$(PREFIX)/extra
INSTALL_DOC=$(PREFIX)/extra/help-$(LIBNAME)

EXT = pd_linux 
DEFS = -DHAVE_ALSA 
IFLAGS = -I.  $(INCLUDES)

CC = gcc
LD = ld
STRIP = strip --strip-unneeded

AFLAGS = 
LFLAGS = -export_dynamic  -shared
WFLAGS =

MAKEDEP_FLAGS = -MM

TARNAME =  $(LIBNAME).tgz

.SUFFIXES: .$(EXT)

CFLAGS = $(DEFS) $(IFLAGS) $(WFLAGS) -fPIC -g -O2 

LIBS = -lasound -lm -lc 

SOURCES=$(sort $(filter %.c, $(wildcard *.c)))
TARGETS = $(SOURCES:.c=.o)


all: $(LIBNAME)
	cp $(LIBNAME).$(EXT) ..

clean:
	-rm -f *.$(EXT) *.o 

distclean: clean
	-rm -f *~ _* config.* *.d
	-rm -rf autom4te.cache

install: install-bin install-doc

install-bin:
	-install -d $(INSTALL_BIN)
	-install -m 644 $(LIBNAME).$(EXT) $(INSTALL_BIN)

install-doc:
	-install -d $(INSTALL_DOC)
	-install -m 644 ../doc/*.pd $(INSTALL_DOC)

dist: all distclean
	(cd ..;tar czvf $(TARNAME) $(LIBNAME))

everything: clean all install distclean


$(LIBNAME): $(TARGETS)
	$(LD) $(LFLAGS) -o $(LIBNAME).$(EXT) *.o $(LIBS)
	$(STRIP) $(LIBNAME).$(EXT)


## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
-include $(SOURCES:.c=.d)
%.d: %.c
	@set -e; rm -f $@; \
	 $(CC) $(MAKEDEP_FLAGS) $(CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

configure: configure.ac
	autoconf

Makefile: Makefile.in configure
	./configure
