include Makefile.config

PDP_DISTRO = $(PDP_DIR)/../pdp-$(PDP_VERSION)
PDP_TARBALL = $(PDP_DISTRO).tar.gz
PDP_WWWDIR = /net/zwizwa/www/zwizwa.fartit.com/pd/pdp
PDP_WWWTESTDIR = /net/zwizwa/www/zwizwa.fartit.com/pd/pdp/test

all: pdp.pd_linux

include Makefile.$(PDP_TARGET)

pdp_all:
	make -C system
	make -C modules

buildclean:
	make -C include clean
	make -C system clean
	make -C modules clean

clean: buildclean
	rm -f pdp.pd_linux
	rm -f *~

distroclean: buildclean
	make -C scaf clean

tags:
	etags --language=auto include/*.h system/mmx/*.s system/*.c modules/*.c

tagsclean:
	rm -f TAGS



distro: distroclean
	rm -rf $(PDP_DISTRO)
	mkdir $(PDP_DISTRO)
	cp -av $(PDP_DIR)/*  $(PDP_DISTRO)
	#strip --strip-unneeded $(PDP_DISTRO)/pdp.pd_linux
	rm -rf $(PDP_DISTRO)/CVS
	rm -rf $(PDP_DISTRO)/*/CVS
	rm -rf $(PDP_DISTRO)/*/*/CVS
	rm -rf $(PDP_DISTRO)/*/*.o
	rm -rf $(PDP_DISTRO)/*/TAGS
	cd $(PDP_DISTRO)/.. && tar vczf pdp-$(PDP_VERSION).tar.gz pdp-$(PDP_VERSION)
	rm -rf $(PDP_DISTRO)

www:	$(PDP_TARBALL) 
	cp -av  $(PDP_TARBALL) $(PDP_WWWDIR)
	cp -av  $(PDP_DIR)/README $(PDP_WWWDIR)/README.txt
	cp -av  $(PDP_DIR)/doc/reference.txt $(PDP_WWWDIR)/REFERENCE.txt
	cp -av  $(PDP_DIR)/CHANGES.LOG $(PDP_WWWDIR)/CHANGELOG.txt
	

www-test:$(PDP_TARBALL) 
	cp -av  $(PDP_TARBALL) $(PDP_WWWTESTDIR)
