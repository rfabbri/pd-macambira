

EXTERNALS = $(shell ls ../src)

all: $(EXTERNALS:.c=.pd_linux)

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I.. -I../../../pd/src

%.pd_linux: ../src/%.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c ../src/$*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	chmod a-x $*.pd_linux
#	strip --strip-unneeded $*.pd_linux
	rm -f $*.o 

clean:
	-rm *.pd_linux *~

install:
	install -m644 *.pd_linux $(prefix)/lib/pd/extra

