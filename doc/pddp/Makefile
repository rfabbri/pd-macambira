current: darwin_package

clean:
	-rm -Rf installroot *~

darwin_altpackage: 
	test -d installroot/Help || mkdir -p installroot/Help
	cp *.pd installroot/Help
	open darwin_altpackage.pmsp

darwin_package: 
	test -d installroot/pd/doc/5.reference/ || mkdir -p installroot/pd/doc/5.reference/
	cp *.pd installroot/pd/doc/5.reference/
	open darwin_package.pmsp
