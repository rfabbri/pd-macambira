
prefix=$(DESTDIR)/usr

EXTERNALS = $(shell ls )

all: link.stamp $(EXTERNALS:.c=.pd_linux)

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE = -I. -I.. -I../include

%.pd_linux: ../src/%.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o "$*.o" -c "../src/$*.c"
	ld -export_dynamic  -shared -o "$*.pd_linux" "$*.o" -lc -lm \
	`test -f $*.libs && cat $*.libs`
	chmod a-x "$*.pd_linux"
#	strip --strip-unneeded $*.pd_linux
	rm -f "$*.o" 

clean:
	-rm *.pd_linux *~ *.c
	-rm link.stamp

install-doc:
	@test -d $(prefix)/lib/pd/doc/5.reference || mkdir -p $(prefix)/lib/pd/doc/5.reference
	cp -r ../doc/* $(prefix)/lib/pd/doc/5.reference

install: install-doc
	@test -d $(prefix)/lib/pd/extra || mkdir -p $(prefix)/lib/pd/extra
	install -m644 *.pd_linux $(prefix)/lib/pd/extra


link: link.stamp

link.stamp:
	cp ../src/*.c .
	rm vst.c
	touch link.stamp
	make
	
	

