
prefix=$(DESTDIR)/usr/local/pd

EXTERNALS = $(shell ls )

all: link.stamp $(EXTERNALS:.c=.pd_darwin)

.SUFFIXES: .pd_darwin

PDEXECUTABLE = /usr/local/pd/bin/pd
# or ../../../pd/bin/pd

MACOSXCFLAGS = -DPD -Os  -DUNIX -DMACOSX -Dunix \
    -Wall -W  \
    -Wno-unused -Wno-parentheses -Wno-switch -Wno-shadow 

MACOSXINCLUDE = -I. -I.. -I../include

MACOSXLINKFLAGS = -bundle  -bundle_loader $(PDEXECUTABLE) 

%.pd_darwin: ../src/%.c
	$(CC) $(MACOSXCFLAGS) $(MACOSXINCLUDE) -o "$*.o" -c "../src/$*.c"
	$(CC) $(MACOSXLINKFLAGS) -o "$*.pd_darwin" "$*.o" -lc -lm \
#	`test -f $*.libs && cat $*.libs`
	chmod a-x "$*.pd_darwin"
	rm -f "$*.o" 

clean:
	-rm *.pd_darwin *~ *.c
	-rm -rf root Package.pkg
	-rm link.stamp

install-doc:
	test -d $(prefix)/doc/5.reference || mkdir -p $(prefix)/doc/5.reference
	cp -r ../doc/* $(prefix)/doc/5.reference

install: install-doc
	test -d $(prefix)/extra || mkdir -p $(prefix)/extra
	install -m644 *.pd_darwin $(prefix)/extra

package:
	test -d root/doc/5.reference || mkdir -p root/doc/5.reference
	cp -r ../doc/* root/doc/5.reference
	test -d root/extra || mkdir -p root/extra
	install -m644 *.pd_darwin root/extra
	echo Now opening PackageMaker. Interact with that program now. \
	Choose the Files tab, and correct the path to point to the newly-made ./root \
	directory. Then choose File -\> Create Package.
	open Package.pmsp

altpackage:
	test -d root/Help || mkdir -p root/Help
	cp -r ../doc/* root/Help
	test -d root/Externals || mkdir -p root/Externals
	install -m644 *.pd_darwin root/Externals
	echo Now opening PackageMaker. Interact with that program now. \
	Choose the Files tab, and correct the path to point to the newly-made ./root \
	directory. Then choose File -\> Create Package.
	open Altpackage.pmsp

link: link.stamp

link.stamp:
	cp ../src/*.c .
#	rm vst.c 
# MacOSX exceptions that don't work just yet
	rm ogg* ossmixer.c plugin~.c cdplayer.c promiscous~.c serialctl.c 
	rm serial_bird.c serial_ms.c proc.c streamin~.c streamout~.c rhythm_*
	touch link.stamp
	make
