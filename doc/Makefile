# Created by Hans-Christoph Steiner <hans@eds.org> to generate MacOS X packages
# This Makefile does not generate a PDDP pkg, PDDP has its own Makefile

current: darwin_pkg

PACKAGE_VERSION = $(shell date +20%y.%m.%d)
PACKAGE_PREFIX = pd-doc
PACKAGE_NAME = $(PACKAGE_PREFIX)-$(PACKAGE_VERSION)

darwin_pkg_license:
  # generate HTML version of License
# 	echo "<HTML><BODY><FONT SIZE="-1">" > License.html
# 	cat ../../creb/COPYING | sed -e 's/^$$/\<P\>/g' >> License.html	
# 	echo "</FONT></BODY></HTML>" >> License.html

darwin_pkg_welcome:
# generate Welcome.html from ../README.txt

darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PACKAGE_PREFIX)*.pkg/
	-rm -f $(PACKAGE_PREFIX)-*.info 1 License.html Welcome.???*

# install into MSP's default: /usr/local/lib

darwin_pkg: darwin_pkg_clean darwin_pkg_license darwin_pkg_welcome
# set up installroot dir
	-chmod a-x */*/*.pd */*/*/*.pd */*/*/*/*.pd
	test -d installroot/pd/doc/7.stuff || mkdir -p installroot/pd/doc/7.stuff
	cp -R tutorials installroot/pd/doc/
	cp -R additional/pd-msg installroot/pd/doc/7.stuff/
	cp -f pd-doc.info $(PACKAGE_NAME).info
# delete cruft
	-find installroot -name .DS_Store -delete
	-sudo rm -Rf installroot/*/*/*/CVS installroot/*/*/*/*/CVS  installroot/*/*/*/*/*/CVS
	-rm -f 1
# set proper permissions
	sudo chown -R root:staff installroot
	package installroot $(PACKAGE_NAME).info -d . -ignoreDSStore
# install pkg docs
#	install -m 644 License.html $(PACKAGE_NAME).pkg/Contents/Resources
	sudo chown -R root:staff $(PACKAGE_NAME).pkg/Contents/Resources

