LTIPREFIX = /usr/local
#LTIPREFIX = /home/matju
LIBS = $(LTIPREFIX)/lib/ltilib/libltir.a -lpng -ljpeg
INCS = -I$(LTIPREFIX)/include/ltilib -I/usr/share/swig1.3/std
PINC = -I/usr/include/python
RINC = -I`ruby -rrbconfig -e'h=Config::CONFIG;puts h["rubylibdir"]+"/"+h["arch"]'`

default:: python

python:: _pylti.so
	python -c 'import pylti'
	python testpylti.py fan.jpg

_pylti.so: pylti_wrap.o Makefile
	g++ -shared -o _pylti.so pylti_wrap.o $(LIBS)

pylti_wrap.o: pylti_wrap.cxx
	g++ $(PINC) -Ipatched $(INCS) -c pylti_wrap.cxx

pylti_wrap.cxx: ./swig/pylti.i
	swig -v -c++ -python -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389  -o ./pylti_wrap.cxx ./swig/pylti.i
	#swig -c++ -python -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated_python -I./patched \
	#	$(INCS) -w509,-312,-362,-389 pylti.i

ruby:: rblti.so
	ruby -e 'require "rblti"'

rblti.so: rblti_wrap.o Makefile
	g++ -shared -o rblti.so rblti_wrap.o $(LIBS)

rblti_wrap.o: rblti_wrap.cxx
	g++ $(RINC) -Ipatched $(INCS) -c rblti_wrap.cxx

rblti_wrap.s: rblti_wrap.cxx
	g++ $(RINC) -Ipatched $(INCS) -S rblti_wrap.cxx

rblti_wrap.cxx: rblti.i
	swig -c++ -ruby -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated -I./patched \
		-fcompact -fvirtual $(INCS) -w509,-312,-362,-389,-801,-314 rblti.i

clean::
	rm -f \
		pylti.so pylti_wrap.o pylti_wrap.cxx \
		rblti.so rblti_wrap.o rblti_wrap.cxx


