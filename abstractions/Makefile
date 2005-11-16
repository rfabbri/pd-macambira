#
# Centralized build system for "abstractions".  
#
# see README for instructions  <hans@at.or.at>
#

# these are setup to be overridden by the packages/*/Makefiles
SRC_ROOT_DIR := $(shell pwd)/..
INSTALL_PREFIX = build

# default target
all: objects applications

include Makefile.dirs

#------------------------------------------------------------------------------#
# OVERARCHING BUILD TARGETS
#------------------------------------------------------------------------------#

final_setup:
	chmod -R ugo-w $(INSTALL_PREFIX)

objects: $(OBJECTS_DEST) $(DOCS_DEST) $(MANUALS_DEST)
objects: objects_memento objects_nusmuk objects_rradical objects_keyboardkeys
objects: objects_gyre objects_la-kitchen objects_nqpoly

applications: $(APPLICATIONS_DEST) $(MANUALS_DEST)
applications: applications_rradical applications_keyboardkeys applications_gyre


#------------------------------------------------------------------------------#
# PROJECT TARGETS
#------------------------------------------------------------------------------#
#------------------------------
# GYRE
objects_gyre:
	install -d $(OBJECTS_DEST)/gyre
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.*.pd \
		 $(OBJECTS_DEST)/gyre
	install -d $(MANUALS_DEST)/gyre
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.html \
		 $(MANUALS_DEST)/gyre

applications_gyre:
	install -d $(APPLICATIONS_DEST)/gyre
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.pd \
		 $(APPLICATIONS_DEST)/GYRE


#------------------------------
# keyboardkeys
objects_keyboardkeys:
	install -d $(OBJECTS_DEST)/keyboardkeys
	install -p $(ABSTRACTIONS_SRC)/keyboardkeys/keyboardkeys/*.pd \
		 $(OBJECTS_DEST)/keyboardkeys
	install -d $(DOCS_DEST)/keyboardkeys
	install -p $(ABSTRACTIONS_SRC)/keyboardkeys/doc/*-help.pd \
		 $(DOCS_DEST)/keyboardkeys

applications_keyboardkeys:
	install -d $(APPLICATIONS_DEST)/keyboardkeys
	install -p $(ABSTRACTIONS_SRC)/keyboardkeys/keyboard_main.pd \
		 $(APPLICATIONS_DEST)/keyboardkeys


#------------------------------
# la-kitchen
objects_la-kitchen:
	install -d $(OBJECTS_DEST)/la-kitchen
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/La-kitchen/*.pd | \
		grep -v '\-help.pd')    $(OBJECTS_DEST)/la-kitchen
	install -d $(DOCS_DEST)/la-kitchen
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/*-help.pd \
		 $(DOCS_DEST)/la-kitchen
	install -d $(MANUALS_DEST)/la-kitchen
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/readme.txt \
		 $(MANUALS_DEST)/la-kitchen


#------------------------------
# memento
objects_memento:
	install -d $(OBJECTS_DEST)/memento
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/rradical/memento/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/memento
	install -d $(DOCS_DEST)/memento
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/*-help.pd $(DOCS_DEST)/memento
	install -d $(MANUALS_DEST)/memento
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/tutorial/*.* \
		$(MANUALS_DEST)/memento
	install -d $(APPLICATIONS_DEST)/memento_tutorial
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/examples/*.* \
		$(APPLICATIONS_DEST)/memento_tutorial


#------------------------------
# nqpoly
objects_nqpoly:
	install -d $(OBJECTS_DEST)/nqpoly~
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/nqpoly~
	install -d $(OBJECTS_DEST)/nqpoly4
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly4/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/nqpoly4
	install -d $(DOCS_DEST)/nqpoly~
	install -p $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/*.pd $(DOCS_DEST)/nqpoly~
	install -d $(DOCS_DEST)/nqpoly4
	install -p $(ABSTRACTIONS_SRC)/nqpoly/nqpoly4/*.pd $(DOCS_DEST)/nqpoly4
	install -d $(MANUALS_DEST)/nqpoly
	install -p $(ABSTRACTIONS_SRC)/nqpoly/*.html	$(MANUALS_DEST)/nqpoly
	install -p $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/readme.txt	\
		$(MANUALS_DEST)/nqpoly/nqpoly~.txt
	install -d $(APPLICATIONS_DEST)/nqpoly~
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/*.pd | \
		grep -v '\-help.pd')  $(APPLICATIONS_DEST)/nqpoly~
	install -d $(APPLICATIONS_DEST)/nqpoly4
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly4/*.pd | \
		grep -v '\-help.pd')  $(APPLICATIONS_DEST)/nqpoly4


#------------------------------
# nusmuk
objects_nusmuk:
	install -d $(OBJECTS_DEST)/nusmuk
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nusmuk/*.* | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/nusmuk
	install -d $(DOCS_DEST)/nusmuk
	install -p $(ABSTRACTIONS_SRC)/nusmuk/*-help.pd $(DOCS_DEST)/nusmuk


#------------------------------
# parazit
	install -d $(OBJECTS_DEST)
	install -p $(ABSTRACTIONS_SRC)/parazit/parazit.pd $(OBJECTS_DEST)


#------------------------------
# RRADical
objects_rradical:
	install -d $(OBJECTS_DEST)/rradical

applications_rradical:
	install -d $(APPLICATIONS_DEST)/RRADical
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/*.* $(APPLICATIONS_DEST)/RRADical
	cp -rp $(ABSTRACTIONS_SRC)/rradical/usecases/showcase $(APPLICATIONS_DEST)/RRADical/

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
	-rm -rf $(OBJECTS_DEST) $(DOCS_DEST)

applications_clean:
	-rm -rf $(APPLICATIONS_DEST)


clean: applications_clean objects_clean
	-rm -f *~
	rm -rf $(MANUALS_DEST)
	rmdir $(DOCS_BASE) $(INSTALL_PREFIX)


#------------------------------------------------------------------------------#
# LEGACY TARGETS
#------------------------------------------------------------------------------#
# this is a legacy clean target to get rid of cruft
darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PKG_PREFIX)*.pkg/
	-rm -f $(PKG_NAME).info 1 
