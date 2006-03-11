NAME=dssi~
CSYM=dssi~

LIBDIR=/usr/local/lib
PDDIR=$(LIBDIR)/pd
INSTALLPATH=$(PDDIR)/extra/

current: pd_linux


# ----------------------- Linux -----------------------

pd_linux: src/$(NAME).pd_linux

.SUFFIXES: .pd_linux

#LINUXCFLAGS = -DPD -O3 -fPIC -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

# Debug
LINUXCFLAGS = -ggdb -g -DPD -O0 -fPIC -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I/usr/include -I./include

.c.pd_linux:
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/jsearch.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/jload.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/dssi~.c
	ld -export_dynamic  -shared -o $(NAME).pd_linux dssi~.o jload.o jsearch.o -lc -lm -llo
#	strip --strip-unneeded $(NAME).pd_linux
	rm -f *.o

# ----------------------- Mac OSX -----------------------

pd_darwin: src/$(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -O3 -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch -L/usr/local/lib/

DARWININCLUDE = -I ./ -I ../src -I/usr/local/include/ -I ./include -I/usr/local/include/dssi/

.c.pd_darwin:
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -o $(NAME).o -c $*.c
	$(CC) -bundle -undefined suppress -flat_namespace -llo -o $(NAME).pd_darwin $(NAME).o
	rm -f *.o

# ----------------------- Generic -----------------------

clean:
	rm -f *.o *.pd_* so_locations

install:
	cp dssi~.pd_linux $(INSTALLPATH)
	install -d  $(PDDIR)/doc/5.reference/dssi/
	install -m 644 doc/help-* $(PDDIR)/doc/5.reference/
	install -m 644 doc/output~.pd $(PDDIR)/doc/5.reference/dssi/
