current: darwin_package

clean:
	-rm -Rf installroot *~ */*~ */*/*~ */*/*/*~

darwin_package: 
	chmod a-x */*/*/*.pd
	test -d installroot/pd/doc/7.stuff || mkdir -p installroot/pd/doc/7.stuff
	cp -a tutorials installroot/pd/doc/
	cp -a additional/pd-msg installroot/pd/doc/7.stuff/
	-rm -Rf installroot/pd/doc/*/CVS installroot/pd/doc/*/*/CVS 
	-rm -Rf installroot/pd/doc/*/*/*/CVS
	open darwin_package.pmsp

darwin_altpackage: 
# this needs some figuring out, what goes where...
	chmod a-x */*/*/*.pd
	test -d installroot/Help || mkdir -p installroot/Help
	open darwin_altpackage.pmsp

