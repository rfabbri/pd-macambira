HELP = ../darwinroot/Library/Pd/Help
EXTERNS = ../darwinroot/Library/Pd/Externals
ADDITION = ../darwinroot/Library/Pd/AdditionalDocumentation

all:
	rm -r ../darwinroot
	mkdir -p $(EXTERNS)
	mkdir -p $(HELP)
	mkdir -p $(ADDITION)
# flext
	make -C ../../flext -f makefile.pd-darwin
	make -C ../../flext -f makefile.pd-darwin install \
		INSTDIR=../build/darwinroot/usr/local/pd/flext
# pool
	make -C ../../pool -f makefile.pd-darwin install \
		INSTPATH=../build/darwinroot/Library/Pd/Externals
	cp ../../pool/pool.pd $(HELP)
	mkdir $(ADDITION)/pool
	cp ../../pool/gpl.txt ../../pool/readme.txt ../../pool/license.txt \
		$(ADDITION)/pool
# xsample
	make -C ../../xsample -f makefile.pd-darwin install \
		INSTDIR=../build/darwinroot/Library/Pd/Externals \
		# HELPPATH=../build/darwinroot/Library/Pd/Help
#     ramp is an abstraction... should we make it available to all?
	cp ../../xsample/pd/ramp.pd $(EXTERNS)
	cp ../../xsample/pd/*.pd $(HELP)
	mkdir $(ADDITION)/xsample
	cp ../../xsample/gpl.txt ../../xsample/readme.txt ../../xsample/license.txt \
		$(ADDITION)/xsample
# fftease
	make -C ../../fftease -f makefile.pd-darwin install \
		INSTPATH=../build/darwinroot/Library/Pd/Externals 
	cp ../../fftease/pd/*.pd $(HELP)
	mkdir $(ADDITION)/fftease
	cp ../../fftease/gpl.txt ../../fftease/readme.txt ../../fftease/license.txt \
		$(ADDITION)/fftease
	cp  -r ../../fftease/media $(ADDITION)/fftease
# vasp
	mkdir -p ../../vasp/pd-darwin # fails without the dir
	make -C ../../vasp -f makefile.pd-darwin install \
		INSTDIR=../build/darwinroot/Library/Pd/Externals 
	cp ../../vasp/pd-help/*.pd $(HELP)
	mkdir $(ADDITION)/vasp
	cp ../../vasp/gpl.txt ../../vasp/readme.txt ../../vasp/license.txt \
		../../vasp/changes.txt ../../vasp/todo.txt  $(ADDITION)/vasp
	cp -r ../../vasp/pd-ex $(ADDITION)/vasp
# py/pyext
	make -C ../../py -f makefile.pd-darwin install \
		INSTPATH=../build/darwinroot/Library/Pd/Externals
	mkdir -p $(ADDITION)/py/examples
	mkdir -p $(ADDITION)/py/scripts
	cp ../../py/pd/*.pd $(ADDITION)/py/examples
	cp ../../py/scripts/*.py $(ADDITION)/py/scripts
	cp ../../py/gpl.txt ../../py/readme.txt ../../py/license.txt \
		$(ADDITION)/py
	mkdir -p ../darwinroot/Library/Frameworks
	cp -R /Library/Frameworks/Python.framework ../darwinroot/Library/Frameworks
# finish it off...
	open flext.pmsp
