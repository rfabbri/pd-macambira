# created by Hans-Christoph Steiner <hans@eds.org> to generate MacOS X packages
# Hopefully, others will be coming soon.

current: darwin_package

PDDP_VERSION = $(shell date +20%y.%m.%d)
PACKAGE_PREFIX = pd-pddp
PACKAGE_NAME = $(PACKAGE_PREFIX)-$(PDDP_VERSION)

clean:
	-rm -Rf installroot *~

darwin_pkg_license:
  # generate HTML version of License
	echo "<HTML><BODY><FONT SIZE="-1">" > License.html
	cat LICENSE.txt | sed -e 's/^$$/\<P\>/g' >> License.html	
	echo "</FONT></BODY></HTML>" >> License.html

darwin_pkg_welcome:
# generate Welcome.html from ../README.txt

darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PACKAGE_PREFIX)*.pkg/
	-rm -f $(PACKAGE_PREFIX)-*.info 1 License.html Welcome.???*

# install into MSP's default: /usr/local/lib

darwin_pkg: darwin_pkg_clean darwin_pkg_license darwin_pkg_welcome
# set up installroot dir
	test -d installroot/pd/doc/5.reference/ || mkdir -p installroot/pd/doc/5.reference/
	install -m644 --group=staff *.pd installroot/pd/doc/5.reference/
	cp -f pd-pddp.info $(PACKAGE_NAME).info
# delete cruft
	-find installroot -name .DS_Store -delete
	-rm -f 1
# set proper permissions
	sudo chown -R root:staff installroot
	package installroot $(PACKAGE_NAME).info -d . -ignoreDSStore
# install pkg docs
	install -m 644 License.html $(PACKAGE_NAME).pkg/Contents/Resources
	sudo chown -R root:staff $(PACKAGE_NAME).pkg/Contents/Resources

# install into MacOS X style path: /Library/Pd

darwin_altpkg:  darwin_pkg_clean darwin_pkg_license darwin_pkg_welcome
	test -d installroot/Help || mkdir -p installroot/Help
	install -m644 --group=staff *.pd installroot/Help
	sed -e 's/\/usr\/local\/lib/\/Library\/Pd/' pd-pddp.info \
		 | sed -e 's/MSP standard paths/MacOS X-style Paths/' \
		 > $(PACKAGE_NAME)-alt.info
 # delete cruft
	-find installroot -name .DS_Store -delete
	-rm -f 1
 # set proper permissions
	sudo chown -R root:staff installroot
	package installroot $(PACKAGE_NAME)-alt.info -d . -ignoreDSStore
 # install pkg docs
	install -m 644 License.html $(PACKAGE_NAME)-alt.pkg/Contents/Resources
	sudo chown -R root:staff $(PACKAGE_NAME)-alt.pkg/Contents/Resources
