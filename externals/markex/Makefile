current:
	echo make pd_darwin, pd_linux


SOURCES=$(sort $(filter %.c, $(wildcard *.c)))

OTARGETS = $(SOURCES:.c=.o)

# ----------------------- DARWIN -----------------------

TARGETS = $(SOURCES:.c=.pd_darwin)

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DMACOSX -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

DARWININCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src -I /usr/local/include

$(TARGETS): %.pd_darwin : %.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -o $*.o -c $*.c
	$(CC) -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o -lc -lm
	rm $*.o

pd_darwin: $(TARGETS)


# ----------------------- LINUX -----------------------

TARGETS = $(SOURCES:.c=.pd_linux)

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src

$(TARGETS): %.pd_linux : %.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	$(LD) -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

pd_linux: $(TARGETS)

# ----------------------- CLEAN -----------------------

clean: ; rm -f *.pd_* *.o *~

