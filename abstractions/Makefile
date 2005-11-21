#==============================================================================#
#
# Centralized build system for "abstractions".  
#
# see for instructions: http://puredata.org/docs/developer/build
#  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

# these are designed to be overridden by the packages/Makefile
SRC_ROOT_DIR = $(CWD)/..
INSTALL_PREFIX = build
BUILDLAYOUT_DIR = $(CWD)

# default target
install: objects applications
	@echo " "
	@echo "abstractions install succeeded!"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# OVERARCHING BUILD TARGETS
#
#==============================================================================#

# clean up after everything is installed
final_setup:
	chmod -R ugo-w $(INSTALL_PREFIX)

objects: $(OBJECTS_DEST) $(HELP_DEST) $(MANUALS_DEST)
objects: objects_gyre objects_keyboardkeys objects_la-kitchen objects_memento
objects: objects_pddp objects_nusmuk objects_nqpoly objects_rradical

applications: $(APPLICATIONS_DEST) $(MANUALS_DEST)
applications: applications_rradical applications_keyboardkeys applications_gyre


#==============================================================================#
#
# PROJECT TARGETS
#
#==============================================================================#

#-------------------------------------------------------------------------------
# GYRE
GYRE_NAME = gyre
objects_gyre:
	install -d $(OBJECTS_DEST)/$(GYRE_NAME)
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.*.pd \
		 $(OBJECTS_DEST)/$(GYRE_NAME)
	install -d $(MANUALS_DEST)/$(GYRE_NAME)
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.html \
		 $(MANUALS_DEST)/$(GYRE_NAME)

applications_gyre:
	install -d $(APPLICATIONS_DEST)/$(GYRE_NAME)
	install -p $(ABSTRACTIONS_SRC)/audionerd/GYRE/gyre.pd \
		 $(APPLICATIONS_DEST)/$(GYRE_NAME)


#-------------------------------------------------------------------------------
# keyboardkeys
KEYBOARDKEYS_NAME = keyboardkeys
objects_keyboardkeys:
	install -d $(OBJECTS_DEST)/$(KEYBOARDKEYS_NAME)
	install -p $(ABSTRACTIONS_SRC)/$(KEYBOARDKEYS_NAME)/abs/*.pd \
		 $(OBJECTS_DEST)/$(KEYBOARDKEYS_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(KEYBOARDKEYS_NAME) \
		--author "Andrey Savitsky" \
		--description "objects for using keyboard keys for scrolling and selecting"
	install -d $(HELP_DEST)/$(KEYBOARDKEYS_NAME)
	install -p $(ABSTRACTIONS_SRC)/$(KEYBOARDKEYS_NAME)/doc/*-help.pd \
		 $(HELP_DEST)/$(KEYBOARDKEYS_NAME)

applications_keyboardkeys:
	install -d $(APPLICATIONS_DEST)/$(KEYBOARDKEYS_NAME)
	install -p $(ABSTRACTIONS_SRC)/$(KEYBOARDKEYS_NAME)/keyboard_main.pd \
		 $(APPLICATIONS_DEST)/$(KEYBOARDKEYS_NAME)


#-------------------------------------------------------------------------------
# la-kitchen
LA-KITCHEN_NAME = la-kitchen
objects_la-kitchen:
	install -d $(OBJECTS_DEST)/$(LA-KITCHEN_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/La-kitchen/*.pd | \
		grep -v '\-help.pd')    $(OBJECTS_DEST)/$(LA-KITCHEN_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(LA-KITCHEN_NAME) \
		--description "a collection of objects working with sensors"
	install -d $(HELP_DEST)/$(LA-KITCHEN_NAME)
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/*-help.pd \
		 $(HELP_DEST)/$(LA-KITCHEN_NAME)
	install -d $(MANUALS_DEST)/$(LA-KITCHEN_NAME)
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/readme.txt \
		 $(MANUALS_DEST)/$(LA-KITCHEN_NAME)


#-------------------------------------------------------------------------------
# memento
MEMENTO_NAME = memento
objects_memento:
	install -d $(OBJECTS_DEST)/$(MEMENTO_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/rradical/memento/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/$(MEMENTO_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(MEMENTO_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL" \
		--description "a collection of objects for managing state saving"
	install -d $(HELP_DEST)/$(MEMENTO_NAME)
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/*-help.pd \
		$(HELP_DEST)/$(MEMENTO_NAME)
	install -d $(MANUALS_DEST)/$(MEMENTO_NAME)
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/tutorial/*.* \
		$(MANUALS_DEST)/$(MEMENTO_NAME)
	install -d $(APPLICATIONS_DEST)/$(MEMENTO_NAME)
	install -p $(ABSTRACTIONS_SRC)/rradical/memento/examples/*.* \
		$(APPLICATIONS_DEST)/$(MEMENTO_NAME)


#-------------------------------------------------------------------------------
# nqpoly
objects_nqpoly: objects_nqpoly~ objects_nqpoly4
# make a common target for the manuals so they are in one place
	install -d $(MANUALS_DEST)/nqpoly
	install -p $(ABSTRACTIONS_SRC)/nqpoly/*.html	$(MANUALS_DEST)/nqpoly
	install -p $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/readme.txt	\
		$(MANUALS_DEST)/nqpoly/nqpoly~.txt

NQPOLY_NAME = nqpoly~
objects_nqpoly~:
	install -d $(OBJECTS_DEST)/$(NQPOLY_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/$(NQPOLY_NAME)
	install -d $(HELP_DEST)/$(NQPOLY_NAME) 
# all of the patches should be installed into help since [nqpoly~] needs to be
# in the same directory as the patches its manipulating
	install -p $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/*.pd \
		$(HELP_DEST)/$(NQPOLY_NAME)
	install -d $(APPLICATIONS_DEST)/$(NQPOLY_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly~/*.pd | \
		grep -v '\-help.pd')  $(APPLICATIONS_DEST)/$(NQPOLY_NAME)


NQPOLY4_NAME = nqpoly4
objects_nqpoly4:
	install -d $(OBJECTS_DEST)/$(NQPOLY4_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly4/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/$(NQPOLY4_NAME)
	install -d $(HELP_DEST)/$(NQPOLY4_NAME)
# all of the patches should be installed into help since [nqpoly4] needs to be
# in the same directory as the patches its manipulating
	install -p $(ABSTRACTIONS_SRC)/nqpoly/nqpoly4/*.pd $(HELP_DEST)/$(NQPOLY4_NAME)
	install -d $(APPLICATIONS_DEST)/$(NQPOLY4_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nqpoly/nqpoly4/*.pd | \
		grep -v '\-help.pd')  $(APPLICATIONS_DEST)/$(NQPOLY4_NAME)

#-------------------------------------------------------------------------------
# nusmuk
NUSMUK_NAME = nusmuk
objects_nusmuk:
	install -d $(OBJECTS_DEST)/$(NUSMUK_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/nusmuk/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/$(NUSMUK_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(NUSMUK_NAME) \
		--author "Cyrille Henry" \
		--description "a collection of objects for physical modelling"
	install -d $(HELP_DEST)/$(NUSMUK_NAME)
	install -p $(ABSTRACTIONS_SRC)/nusmuk/*-help.pd \
		$(HELP_DEST)/$(NUSMUK_NAME)
	install -p $(ABSTRACTIONS_SRC)/nusmuk/*.wav \
		$(HELP_DEST)/$(NUSMUK_NAME)


#-------------------------------------------------------------------------------
# parazit
	install -d $(OBJECTS_DEST)
	install -p $(ABSTRACTIONS_SRC)/parazit/parazit.pd $(OBJECTS_DEST)



#-------------------------------------------------------------------------------
# PDDP support lib
PDDP_NAME = pddp
objects_pddp:
	install -d $(OBJECTS_DEST)/$(PDDP_NAME)
	install -p $(shell ls -1 $(ABSTRACTIONS_SRC)/pddp/*.pd | \
		grep -v '\-help.pd')  $(OBJECTS_DEST)/$(PDDP_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(PDDP_NAME) \
		--author "Hans-Christoph Steiner" \
		--license "GNU GPL" \
		--description "support objects for the Pure Data Documentation Project"
	install -d $(HELP_DEST)/$(PDDP_NAME)
	install -p $(ABSTRACTIONS_SRC)/pddp/*-help.pd \
		$(HELP_DEST)/$(PDDP_NAME)
#	install -d $(MANUALS_DEST)/$(PDDP_NAME)
#	install -d $(APPLICATIONS_DEST)/$(PDDP_NAME)

#-------------------------------------------------------------------------------
# RRADical
RRADICAL_NAME = rradical
objects_rradical:
	install -d $(OBJECTS_DEST)/$(RRADICAL_NAME)

applications_rradical:
	install -d $(APPLICATIONS_DEST)/$(RRADICAL_NAME)
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/*.* \
		$(APPLICATIONS_DEST)/$(RRADICAL_NAME)
	install -d $(APPLICATIONS_DEST)/$(RRADICAL_NAME)/showcase
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/showcase/*.* \
		$(APPLICATIONS_DEST)/$(RRADICAL_NAME)/showcase
	install -d $(APPLICATIONS_DEST)/$(RRADICAL_NAME)/showcase/impl
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/showcase/impl/*.* \
		$(APPLICATIONS_DEST)/$(RRADICAL_NAME)/showcase/impl
	install -d $(APPLICATIONS_DEST)/$(RRADICAL_NAME)/showcase/songs
	install -p $(ABSTRACTIONS_SRC)/rradical/usecases/showcase/songs/*.* \
		$(APPLICATIONS_DEST)/$(RRADICAL_NAME)/showcase/songs

#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks: devsymlinks_keyboardkeys


devsymlinks_keyboardkeys:
	ln -s abs \
		$(ABSTRACTIONS_SRC)/keyboardkeys/keyboardkeys
	ln -s ../keyboardkeys \
		$(ABSTRACTIONS_SRC)/keyboardkeys/doc/keyboardkeys

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#

# the destination-specific clean targets are in Makefile.buildlayout
clean: install_clean

distclean: clean cruft_clean
