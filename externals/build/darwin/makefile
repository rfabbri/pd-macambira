
prefix=$(DESTDIR)/usr/local/lib/pd

EXTERNALS = $(shell ls )

all: link.stamp $(EXTERNALS:.c=.pd_darwin)

.SUFFIXES: .pd_darwin

PDEXECUTABLE = /usr/local/bin/pd
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
	-rm *.pd_darwin *~ *.c *.o
	-rm -rf root *.pkg
	-rm link.stamp

install-doc:
	test -d $(prefix)/doc/5.reference || mkdir -p $(prefix)/doc/5.reference
	cp -r ../doc/* $(prefix)/doc/5.reference

install: install-doc
	test -d $(prefix)/extra || mkdir -p $(prefix)/extra
	install -m644 *.pd_darwin $(prefix)/extra

link: link.stamp

link.stamp:
	cp ../src/*.c .
#	rm vst.c 
# MacOSX exceptions that don't work just yet
	-rm ogg* ossmixer.c plugin~.c cdplayer.c promiscous~.c serialctl.c 
	-rm serial_bird.c serial_ms.c proc.c streamin~.c streamout~.c rhythm_*
	touch link.stamp
	make


EXTERNALS_VERSION = $(shell date +20%y.%m.%d)
PACKAGE_PREFIX = pd-externals
PACKAGE_NAME = $(PACKAGE_PREFIX)-$(EXTERNALS_VERSION)

darwin_pkg_license:
  # generate HTML version of License
	echo "<HTML><BODY><FONT SIZE="-1">" > License.html
	cat ../../creb/COPYING | sed -e 's/^$$/\<P\>/g' >> License.html	
	echo "</FONT></BODY></HTML>" >> License.html

darwin_pkg_welcome:
# generate Welcome.html from ../README.txt

darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PACKAGE_PREFIX)*.pkg/
	-rm -f $(PACKAGE_PREFIX)-*.info 1 License.html Welcome.???*

# install into MSP's default: /usr/local/lib

darwin_pkg: all darwin_pkg_clean darwin_pkg_license darwin_pkg_welcome
# set up installroot dir
	test -d installroot/pd/doc/5.reference/ || mkdir -p installroot/pd/doc/5.reference/
	cp -R ../doc/* installroot/pd/doc/5.reference
	test -d installroot/pd/extra || mkdir -p installroot/pd/extra
	install -m644 --group=staff *.pd_darwin installroot/pd/extra
	cp -f pd-externals.info $(PACKAGE_NAME).info
# delete cruft
	-find installroot -name .DS_Store -delete
	-sudo rm -Rf installroot/*/*/CVS installroot/*/*/*/CVS installroot/*/*/*/*/CVS 
	-rm -f 1
# set proper permissions
	sudo chown -R root:staff installroot
	package installroot $(PACKAGE_NAME).info -d . -ignoreDSStore
# install pkg docs
	install -m 644 License.html $(PACKAGE_NAME).pkg/Contents/Resources
	sudo chown -R root:staff $(PACKAGE_NAME).pkg/Contents/Resources


# install into MacOS X style path: /Library/Pd

darwin_altpkg: all darwin_pkg_clean darwin_pkg_license darwin_pkg_welcome
	test -d installroot/Help || mkdir -p installroot/Help
	cp -r ../doc/* installroot/Help
	test -d installroot/Externals || mkdir -p installroot/Externals
	install -m644 --group=staff *.pd_darwin installroot/Externals
	sed -e 's/\/usr\/local\/lib/\/Library\/Pd/' pd-externals.info \
		 | sed -e 's/MSP standard paths/MacOS X-style Paths/' \
		 > $(PACKAGE_NAME)-alt.info
 # delete cruft
	-sudo find installroot -name .DS_Store -delete
	-sudo rm -Rf installroot/*/*/CVS installroot/*/*/*/CVS installroot/*/*/*/*/CVS 
	-rm -f 1
 # set proper permissions
	sudo chown -R root:staff installroot
	package installroot $(PACKAGE_NAME)-alt.info -d . -ignoreDSStore
 # install pkg docs
	install -m 644 License.html $(PACKAGE_NAME)-alt.pkg/Contents/Resources
	sudo chown -R root:staff $(PACKAGE_NAME)-alt.pkg/Contents/Resources
