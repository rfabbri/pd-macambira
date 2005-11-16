#
# Centralized build system for "abstractions".  
#
# see README for instructions  <hans@at.or.at>
#

# these are setup to be overridden by the packages/*/Makefiles
SRC_ROOT_DIR := $(shell pwd)/..
INSTALL_PREFIX = build
DOCS_BASE = $(INSTALL_PREFIX)/doc

# default target
all: objects applications

include Makefile.dirs

#------------------------------------------------------------------------------#
# OVERARCHING BUILD TARGETS
#------------------------------------------------------------------------------#

final_setup:
	chmod -R ugo-w $(INSTALL_PREFIX)

objects: $(OBJECTS_DST) $(DOCS_DST) $(MANUALS_DST)
objects: objects_memento objects_nusmuk objects_rradical objects_keyboardkeys
objects: objects_gyre objects_la-kitchen

applications: $(APPLICATIONS_DST) $(MANUALS_DST)
applications: applications_rradical applications_keyboardkeys applications_gyre


#------------------------------------------------------------------------------#
# PROJECT TARGETS
#------------------------------------------------------------------------------#
#------------------------------
# GYRE
objects_gyre:
	install -d $(OBJECTS_DST)/gyre
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.*.pd \
		 $(OBJECTS_DST)/gyre
	install -d $(MANUALS_DST)/gyre
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.html \
		 $(MANUALS_DST)/gyre

applications_gyre:
	install -d $(APPLICATIONS_DST)/gyre
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.pd \
		 $(APPLICATIONS_DST)/GYRE


#------------------------------
# keyboardkeys
objects_keyboardkeys:
	install -d $(OBJECTS_DST)/keyboardkeys
	install -p $(ABSTRACTIONS_SRC)/keyboardkeys/keyboardkeys/*.pd \
		 $(OBJECTS_DST)/keyboardkeys
	install -d $(DOCS_DST)/keyboardkeys
	install -p $(ABSTRACTIONS_SRC)/keyboardkeys/doc/*-help.pd \
		 $(DOCS_DST)/keyboardkeys

applications_keyboardkeys:
	install -d $(APPLICATIONS_DST)/keyboardkeys
	install -p $(ABSTRACTIONS_SRC)/keyboardkeys/keyboard_main.pd \
		 $(APPLICATIONS_DST)/keyboardkeys


#------------------------------
# la-kitchen
objects_la-kitchen:
	install -d $(OBJECTS_DST)/la-kitchen
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/La-kitchen/*.pd | \
		grep -v '\-help.pd')    $(OBJECTS_DST)/la-kitchen
	install -d $(DOCS_DST)/la-kitchen
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/*-help.pd \
		 $(DOCS_DST)/la-kitchen
	install -d $(MANUALS_DST)/la-kitchen
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/readme.txt \
		 $(MANUALS_DST)/la-kitchen


#------------------------------
# memento
objects_memento:
	install -d $(OBJECTS_DST)/memento
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/rradical/memento/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DST)/memento
	install -d $(DOCS_DST)/memento
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/*-help.pd $(DOCS_DST)/memento
	install -d $(MANUALS_DST)/memento
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/tutorial/*.* \
		$(MANUALS_DST)/memento
	install -d $(APPLICATIONS_DST)/memento_tutorial
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/examples/*.* \
		$(APPLICATIONS_DST)/memento_tutorial

#------------------------------
# nusmuk
objects_nusmuk:
	install -d $(OBJECTS_DST)/nusmuk
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nusmuk/*.* | \
		grep -v '\-help.pd')  $(OBJECTS_DST)/nusmuk
	install -d $(DOCS_DST)/nusmuk
	install -p $(ABSTRACTIONS_SRC)/nusmuk/*-help.pd $(DOCS_DST)/nusmuk


#------------------------------
# RRADical
objects_rradical:
	install -d $(OBJECTS_DST)/rradical

applications_rradical:
	install -d $(APPLICATIONS_DST)/RRADical
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/*.* $(APPLICATIONS_DST)/RRADical
	cp -rp $(ABSTRACTIONS_SRC)/rradical/usecases/showcase $(APPLICATIONS_DST)/RRADical/

#------------------------------------------------------------------------------#
# DEVELOPER'S TARGETS
#------------------------------------------------------------------------------#

# make the symlinks necessary to simulate the installed environment
devsymlinks: devsymlinks_keyboardkeys


devsymlinks_keyboardkeys:
	ln -s abs \
		$(ABSTRACTIONS_SRC)/keyboardkeys/keyboardkeys
	ln -s ../keyboardkeys \
		$(ABSTRACTIONS_SRC)/keyboardkeys/doc/keyboardkeys

#------------------------------------------------------------------------------#
# CLEAN TARGETS
#------------------------------------------------------------------------------#
objects_clean:
	-rm -rf $(OBJECTS_DST) $(DOCS_DST)

applications_clean:
	-rm -rf $(APPLICATIONS_DST)


clean: applications_clean objects_clean
	-rm -f *~
	rm -rf $(MANUALS_DST)
	rmdir $(DOCS_BASE) $(INSTALL_PREFIX)


#------------------------------------------------------------------------------#
# LEGACY TARGETS
#------------------------------------------------------------------------------#
# this is a legacy clean target to get rid of cruft
darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PKG_PREFIX)*.pkg/
	-rm -f $(PKG_NAME).info 1 
