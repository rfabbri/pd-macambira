include Makefile.config

current: 
	make -C system
	make -C modules
	make -C modules++

	rm -f $(LIBNAME)
	$(CXX) $(LIBFLAGS) -o $(LIBNAME) system/*.o modules/*.o modules++/*.o -lm

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

mrproper: clean tagsclean
	rm -rf Makefile.config config.status config.log configure autom4te.cache


