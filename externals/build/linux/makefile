
prefix=$(DESTDIR)/usr

EXTERNALS = $(shell ls ../src)
STRIP = strip --strip-unneeded -R .note -R .comment


all: $(EXTERNALS:.c=.pd_linux)

.SUFFIXES: .pd_linux

LINUXCFLAGS = -fPIC -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE = -I. -I.. -I../include

%.pd_linux: ../src/%.c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o "$*.o" -c "../src/$*.c"
	gcc -Wl,-export_dynamic  -shared -o "$*.pd_linux" "$*.o" -lc -lm \
	`test -f $*.libs && cat $*.libs`
	chmod a-x "$*.pd_linux"
	$(STRIP) $*.pd_linux
	rm -f "$*.o" 

clean:
	-rm *.pd_linux *~ *.c
	-rm link.stamp

install-doc:
	@test -d $(prefix)/lib/pd/doc/5.reference || mkdir -p $(prefix)/lib/pd/doc/5.reference
	cd ../doc && make all
	cp -r ../doc/* $(prefix)/lib/pd/doc/5.reference

install: install-doc
	@test -d $(prefix)/lib/pd/extra || mkdir -p $(prefix)/lib/pd/extra
	install -m644 *.pd_linux $(prefix)/lib/pd/extra


deb:
	cd ../.. && dpkg-buildpackage -b
