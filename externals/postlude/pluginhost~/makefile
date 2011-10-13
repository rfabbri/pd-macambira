NAME=pluginhost~
CSYM=pluginhost~

LIBDIR=/usr/local/lib
PDDIR=$(LIBDIR)/pd
INSTALLPATH=$(PDDIR)/extra/
ARCHITECTURE=i386
DEBUG=0

current: pd_linux


# ----------------------- Linux -----------------------

pd_linux: src/$(NAME).pd_linux

.SUFFIXES: .pd_linux

# Debug
LINUXCFLAGS = -ggdb -g -DPD -O0 -fPIC -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch -DDEBUG=$(DEBUG)

LINUXINCLUDE =  -I/usr/include -I./include

.c.pd_linux:
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/jsearch.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/jload.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/pluginhost~.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/handlers_osc.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/handlers_pd.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/ph_common.c
	gcc --export-dynamic  -shared -o $(NAME).pd_linux pluginhost~.o jload.o jsearch.o handlers_osc.o handlers_pd.o ph_common.o -lc -lm
#	strip --strip-unneeded $(NAME).pd_linux
	cp $(NAME).pd_linux ~/pd-externals/
	rm -f *.o

# ----------------------- Mac OSX -----------------------

pd_darwin: src/$(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -g -ggdb -DPD -arch $(ARCHITECTURE) -O0 -Wall -L/usr/local/lib/ -m32 -DDEBUG=$(DEBUG)

DARWININCLUDE = -I ./ -I ../src -I/usr/local/include/ -I ./include -I/usr/local/include/pluginhost/

.c.pd_darwin:
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -c src/jsearch.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -c src/jload.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -c src/pluginhost~.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -c src/handlers_osc.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -c src/handlers_pd.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -c src/ph_common.c
	$(CC) -m32 -arch $(ARCHITECTURE) -bundle -undefined suppress -flat_namespace  -o $(NAME).pd_darwin pluginhost~.o jload.o jsearch.o handlers_osc.o handlers_pd.o ph_common.o
	#rm -f *.o

# ----------------------- Generic -----------------------

clean:
	rm -f *.o *.pd_* so_locations

install:
	cp pluginhost~.pd_* $(INSTALLPATH)
	install -d  $(PDDIR)/doc/5.reference/pluginhost/
	install -m 644 doc/*-help* $(PDDIR)/doc/5.reference/
	install -m 644 doc/output~.pd $(PDDIR)/doc/5.reference/pluginhost/
