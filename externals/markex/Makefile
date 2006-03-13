current:
	@echo make pd_linux, MarkEx.pd_linux, pd_darwin, MarkEx.pd_darwin

MarkEx:
	@echo make MarkEx.pd_{linux,darwin}

SOURCES=$(sort $(filter-out MarkEx.c, $(filter %.c, $(wildcard *.c))))

# ----------------------- DARWIN -----------------------

TARGETS = $(SOURCES:.c=.pd_darwin)
OTARGETS = $(SOURCES:.c=.o_darwin)

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DMACOSX -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

DARWININCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src -I /usr/local/include

$(OTARGETS): %.o_darwin : %.c
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -o $*.o_darwin -c $*.c

$(TARGETS): %.pd_darwin : %.o_darwin
	$(CC) -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o -lc -lm

pd_darwin: $(TARGETS)


MarkEx.pd_darwin: $(OTARGETS)
	$(CC) $(DARWINCFLAGS) $(DARWININCLUDE) -o MarkEx.o -c MarkEx.c
	$(CC)  -bundle -undefined suppress -flat_namespace -o MarkEx.pd_linux *.o_darwin -lc -lm

# ----------------------- LINUX -----------------------

OTARGETS = $(SOURCES:.c=.o_linux)
TARGETS = $(SOURCES:.c=.pd_linux)

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src

$(OTARGETS): %.o_linux : %.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o_linux -c $*.c

$(TARGETS): %.pd_linux : %.o_linux
	$(LD) -export_dynamic  -shared -o $*.pd_linux $*.o_linux -lc -lm
	strip --strip-unneeded $*.pd_linux

pd_linux: $(TARGETS)

MarkEx.pd_linux: $(OTARGETS)
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o MarkEx.o_linux -c MarkEx.c
	$(LD) -export_dynamic  -shared -o MarkEx.pd_linux *.o_linux -lc -lm
	strip --strip-unneeded MarkEx.pd_linux

# ----------------------- CLEAN -----------------------

clean: ; rm -f *.pd_* *.o *.o_* *~

