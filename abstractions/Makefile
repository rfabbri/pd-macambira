#
# Centralized build system for "abstractions".  
#
# see README for instructions  <hans@at.or.at>
#

# these are setup to be overridden by the packages/*/Makefiles
SRC_ROOT_DIR := $(shell pwd)/..
INSTALL_PREFIX = build
DOCS_BASE = $(INSTALL_PREFIX)/doc

# sources
ABSTRACTIONS_SRC = $(SRC_ROOT_DIR)/abstractions

# destinations
APPLICATIONS_DST = $(DOCS_BASE)/examples
OBJECTS_DST = $(INSTALL_PREFIX)/extra
DOCS_DST = $(DOCS_BASE)/5.reference
MANUALS_DST = $(DOCS_BASE)/manuals

#------------------------------------------------------------------------------#
# DIRECTORY STRUCTURE TARGETS
#------------------------------------------------------------------------------#
# first make sure that the directory structure is setup
$(INSTALL_PREFIX):
	install -d $(INSTALL_PREFIX)

$(APPLICATIONS_DST):  $(INSTALL_PREFIX)
	install -d $(APPLICATIONS_DST)

$(OBJECTS_DST): $(INSTALL_PREFIX)
	install -d $(OBJECTS_DST)

$(DOCS_DST): $(INSTALL_PREFIX)
	install -d $(DOCS_DST)

$(MANUALS_DST): $(INSTALL_PREFIX)
	install -d $(MANUALS_DST)


#------------------------------------------------------------------------------#
# OVERARCHING BUILD TARGETS
#------------------------------------------------------------------------------#

all: objects applications

final_setup:
	chmod -R ugo-w $(INSTALL_PREFIX)

objects: $(OBJECTS_DST) $(DOCS_DST) $(MANUALS_DST)
objects: objects_rradical objects_memento

applications: $(APPLICATIONS_DST) $(MANUALS_DST)
applications: applications_rradical


#------------------------------------------------------------------------------#
# PROJECT TARGETS
#------------------------------------------------------------------------------#

#------------------------------
# RRADICAL
objects_rradical:
	install -d $(OBJECTS_DST)/memento

applications_rradical:
	install -d $(APPLICATIONS_DST)/RRADical
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/*.* $(APPLICATIONS_DST)/RRADical
	cp -rp $(ABSTRACTIONS_SRC)/rradical/usecases/showcase $(APPLICATIONS_DST)/RRADical/

#------------------------------
# MEMENTO
objects_memento:
	install -d $(OBJECTS_DST)/memento
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/rradical/memento/*.pd | \
		grep -v -e '-help.pd$$')  $(OBJECTS_DST)/memento
	install -d $(DOCS_DST)/memento
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/*-help.pd $(DOCS_DST)/memento
	install -d $(MANUALS_DST)/memento
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/tutorial/*.* \
		$(MANUALS_DST)/memento
	install -d $(APPLICATIONS_DST)/memento_tutorial
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/examples/*.* \
		$(APPLICATIONS_DST)/memento_tutorial

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
