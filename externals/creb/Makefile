include Makefile.config

CREB_DISTRO = $(CREB_DIR)/../creb-$(CREB_VERSION)
CREB_TARBALL = $(CREB_DISTRO).tar.gz
CREB_WWWDIR = /net/zwizwa/www/zwizwa.fartit.com/pd/creb

LIBNAME=creb.pd_linux

current: 
	make -C system
	make -C modules
	make -C modules++

	rm -f $(LIBNAME)
	$(CXX) -export_dynamic -shared -o $(LIBNAME) system/*.o modules/*.o modules++/*.o -lm
	strip --strip-unneeded $(LIBNAME)

clean:
	make -C include clean
	make -C modules clean
	make -C modules++ clean
	make -C system clean
	rm -f $(LIBNAME)
	rm -f *~

tags:
	etags --language=auto include/*.h system/*.c modules/*.c modules++/*.cpp

tagsclean:
	rm -f TAGS


distro: clean
	rm -rf $(CREB_DISTRO)
	mkdir $(CREB_DISTRO)
	cp -av $(CREB_DIR)/*  $(CREB_DISTRO)
	rm -rf $(CREB_DISTRO)/CVS
	rm -rf $(CREB_DISTRO)/*/CVS
	rm -rf $(CREB_DISTRO)/*/*/CVS
	rm -rf $(CREB_DISTRO)/*/*.o
	rm -rf $(CREB_DISTRO)/*/TAGS
	cd $(CREB_DISTRO)/.. && tar vczf creb-$(CREB_VERSION).tar.gz creb-$(CREB_VERSION)
	rm -rf $(CREB_DISTRO)

www:	$(PDP_TARBALL) 
	cp -av  $(CREB_TARBALL) $(CREB_WWWDIR)
	cp -av  $(CREB_DIR)/README $(CREB_WWWDIR)/README.txt
	cp -av  $(CREB_DIR)/doc/reference.txt $(CREB_WWWDIR)

