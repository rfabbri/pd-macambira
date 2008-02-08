#
# automake template
# added by tim blechmann
#

EXTRA_DIST = bonk~ choice expr~ fiddle~ loop~ lrshift~ pique 

MAINTAINERCLEANFILES = Makefile.in \
	Makefile 

EXTERNS = ./*/*.$(EXT)

all:
	cd ./bonk~ && make @EXT@
	cd ./choice && make @EXT@
	cd ./expr~ && make @EXT@
	cd ./fiddle~ && make @EXT@
	cd ./loop~ && make @EXT@
	cd ./lrshift~ && make @EXT@
	cd ./pique && make @EXT@

install:
	install -m 644 $(EXTERNS) $(prefix)/lib/pd/extra

dist-hook:
	rm -rf `find $(distdir) -name CVS`	