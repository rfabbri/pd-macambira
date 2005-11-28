#
# for making a tar.bz2 that installs using a Makefile
#
all: install

CWD := $(shell pwd)

DESTDIR = $(CWD)/build/
cvs_root_dir = $(CWD)/../..
BUILDLAYOUT_DIR = $(CWD)/..


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				UNAME=$(UNAME)


install: 
	cd $(packages_src) && make $(DEST_PATHS) install
	cd $(packages_src) && make $(DEST_PATHS) doc_format
	@echo " "
	@echo "linux_make install succeeded!"

#==============================================================================#
#
## 
#
#==============================================================================#

PACKAGE_NAME = Pd-$(PD_VERSION)-$(PACKAGE_VERSION)-$(OS_NAME)-$(shell uname -m)
tarbz2:
# it is very hard to escape the $ in shell scripts so delete files
	-rm -f -- $(helpdir)/iemabs/*\$$*.pd $(objectsdir)/iemabs/*\$$*.pd
	-./generate_install_makefile.bash  $(DESTDIR)$(prefix) > tarbz2Makefile
	mv -i tarbz2Makefile $(DESTDIR)$(prefix)/Makefile
	mv $(DESTDIR)$(prefix) $(DESTDIR)$(PACKAGE_NAME)
	cd $(DESTDIR) && \
		tar cjf $(PACKAGE_NAME).tar.bz2  $(PACKAGE_NAME)
	mv $(DESTDIR)$(PACKAGE_NAME) $(DESTDIR)$(prefix)

#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

clean: 	
	cd $(packages_src) && make $(DEST_PATHS) clean


