include Makefile.config


all: $(PDP_TARGET)

pdp_all:
	make -C system
	make -C modules

darwin:	pdp_all
	rm -f pdp.pd_darwin
	$(CC)  -o pdp.pd_darwin modules/*/*.o system/*/*.o $(PDP_LIBS) -bundle -undefined error -twolevel_namespace -bundle_loader $(PD_EXECUTABLE) 


linux: pdp_all
	rm -f pdp.pd_linux
	$(CC) -export_dynamic -shared -o pdp.pd_linux modules/*/*.o system/*/*.o $(PDP_LIBS)

linux_mmx: pdp_all
	rm -f pdp.pd_linux
	$(CC) -export_dynamic -shared -o pdp.pd_linux modules/*/*.o system/*/*.o $(PDP_LIBS)

buildclean:
	make -C include clean
	make -C system clean
	make -C modules clean

clean: buildclean
	rm -f pdp.pd_linux
	rm -f *~

distroclean: buildclean
	make -C scaf clean
	make -C opengl clean
	make -C guile clean

mrproper: clean
	make -C scaf mrproper
	rm -rf configure
	rm -rf config.log
	rm -rf config.status
	rm -rf autom4te.cache
	#this needs to stay in to keep the makefiles working
	#rm -rf Makefile.config

tags:
	etags --language=auto include/*.h system/mmx/*.s system/*/*.c \
	modules/*/*.c scaf/*/*.c scaf/*/*.s opengl/*/*.c

tagsclean:
	rm -f TAGS


install: all
	#check if pd is installed. if this fails make install will stop here.
	install -m 755 $(PDP_LIBRARY_NAME) $(prefix)/lib/pd/externs
	install -m 755 -d $(prefix)/include/pdp
	install -m 644 include/*.h $(prefix)/include/pdp
	install -m 644 abstractions/*.pd $(prefix)/lib/pd/externs
	install -m 644 doc/objects/*.pd $(prefix)/lib/pd/doc/5.reference
	install -m 755 -d $(prefix)/lib/pd/doc/pdp
	install -m 755 -d $(prefix)/lib/pd/doc/pdp/introduction
	install -m 755 -d $(prefix)/lib/pd/doc/pdp/examples
	install -m 644 doc/reference.txt $(prefix)/lib/pd/doc/pdp
	install -m 644 doc/introduction/*.pd $(prefix)/lib/pd/doc/pdp/introduction
	install -m 644 doc/examples/*.pd $(prefix)/lib/pd/doc/pdp/examples
	install -m 755 bin/pdp-config $(prefix)/bin
