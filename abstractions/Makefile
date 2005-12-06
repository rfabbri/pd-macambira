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
cvs_root_dir = $(CWD)/..
DESTDIR = $(CWD)/build/
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# OVERARCHING BUILD TARGETS
#
#==============================================================================#

# in case anything needs to be compiled or processed somehow before installing
all:

# clean up after everything is installed
final_setup:
	chmod -R ugo-w $(pddocdir)

install: all $(objectsdir) $(helpdir) $(manualsdir) $(examplesdir) \
gyre_install keyboardkeys_install la_install-kitchen list-abs_install memento_install \
nusmuk_install nqpoly_install parazit_install pddp_install pixeltango_install \
rradical_install
	@echo " "
	@echo "abstractions install succeeded!"

#==============================================================================#
#
# PROJECT TARGETS
#
#==============================================================================#

#-------------------------------------------------------------------------------
# GYRE
GYRE_NAME = gyre
gyre_install:
	install -d $(objectsdir)/$(GYRE_NAME)
	install -p $(abstractions_src)/audionerd/GYRE/gyre.*.pd \
		 $(objectsdir)/$(GYRE_NAME)
	install -d $(manualsdir)/$(GYRE_NAME)
	install -p $(abstractions_src)/audionerd/GYRE/gyre.html \
		 $(manualsdir)/$(GYRE_NAME)
	install -d $(examplesdir)/$(GYRE_NAME)
	install -p $(abstractions_src)/audionerd/GYRE/gyre.pd \
		 $(examplesdir)/$(GYRE_NAME)


#-------------------------------------------------------------------------------
# keyboardkeys
KEYBOARDKEYS_NAME = keyboardkeys
keyboardkeys_install:
	install -d $(objectsdir)/$(KEYBOARDKEYS_NAME)
	install -p $(abstractions_src)/$(KEYBOARDKEYS_NAME)/abs/*.pd \
		 $(objectsdir)/$(KEYBOARDKEYS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(KEYBOARDKEYS_NAME) \
		--author "Andrey Savitsky" \
		--description "objects for using keyboard keys for scrolling and selecting"
	install -d $(helpdir)/$(KEYBOARDKEYS_NAME)
	install -p $(abstractions_src)/$(KEYBOARDKEYS_NAME)/doc/*-help.pd \
		 $(helpdir)/$(KEYBOARDKEYS_NAME)
	install -d $(examplesdir)/$(KEYBOARDKEYS_NAME)
	install -p $(abstractions_src)/$(KEYBOARDKEYS_NAME)/keyboard_main.pd \
		 $(examplesdir)/$(KEYBOARDKEYS_NAME)

#-------------------------------------------------------------------------------
# list-abs
LIST-ABS_NAME = list-abs
list-abs_install:
	install -d $(objectsdir)/$(LIST-ABS_NAME)
	install -p $(shell ls -1 $(abstractions_src)/footils/list-abs/*.pd | \
		grep -v '\-help.pd')  $(objectsdir)/$(LIST-ABS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(LIST-ABS_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL" \
		--description "a collection of objects for manipulating lists. Requires pd>=0.39"
	install -d $(helpdir)/$(LIST-ABS_NAME)
	install -p $(abstractions_src)/footils/list-abs/*-help.pd \
		$(helpdir)/$(LIST-ABS_NAME)



#-------------------------------------------------------------------------------
# la-kitchen
LA-KITCHEN_NAME = la-kitchen
la_install-kitchen:
	install -d $(objectsdir)/$(LA-KITCHEN_NAME)
	install -p $(shell ls -1 $(abstractions_src)/La-kitchen/*.pd | \
		grep -v '\-help.pd')    $(objectsdir)/$(LA-KITCHEN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(LA-KITCHEN_NAME) \
		--author "Charles Verron" \
		--description "a collection of objects working with sensors"
	install -d $(helpdir)/$(LA-KITCHEN_NAME)
	install -p $(abstractions_src)/La-kitchen/*-help.pd \
		 $(helpdir)/$(LA-KITCHEN_NAME)
	install -d $(manualsdir)/$(LA-KITCHEN_NAME)
	install -p $(abstractions_src)/La-kitchen/readme.txt \
		 $(manualsdir)/$(LA-KITCHEN_NAME)


#-------------------------------------------------------------------------------
# memento
MEMENTO_NAME = memento
memento_install:
	install -d $(objectsdir)/$(MEMENTO_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/memento/*.pd | \
		grep -v '\-help.pd')  $(objectsdir)/$(MEMENTO_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MEMENTO_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL" \
		--description "a collection of objects for managing state saving"
	install -d $(helpdir)/$(MEMENTO_NAME)
	install -p $(abstractions_src)/rradical/memento/*-help.pd \
		$(helpdir)/$(MEMENTO_NAME)
	install -d $(manualsdir)/$(MEMENTO_NAME)
	install -p $(abstractions_src)/rradical/memento/tutorial/*.* \
		$(manualsdir)/$(MEMENTO_NAME)
	install -d $(examplesdir)/$(MEMENTO_NAME)
	install -p $(abstractions_src)/rradical/memento/examples/*.* \
		$(examplesdir)/$(MEMENTO_NAME)


#-------------------------------------------------------------------------------
# nqpoly
nqpoly_install: nqpoly_install~ nqpoly_install4
# make a common target for the manuals so they are in one place
	install -d $(manualsdir)/nqpoly
	install -p $(abstractions_src)/nqpoly/*.html	$(manualsdir)/nqpoly
	install -p $(abstractions_src)/nqpoly/nqpoly~/readme.txt	\
		$(manualsdir)/nqpoly/nqpoly~.txt

NQPOLY_NAME = nqpoly~
nqpoly_install~:
	install -d $(objectsdir)/$(NQPOLY_NAME)
	install -p $(shell ls -1 $(abstractions_src)/nqpoly/nqpoly~/*.pd | \
		grep -v '\-help.pd')  $(objectsdir)/$(NQPOLY_NAME)
	install -d $(helpdir)/$(NQPOLY_NAME) 
# all of the patches should be installed into help since [nqpoly~] needs to be
# in the same directory as the patches its manipulating
	install -p $(abstractions_src)/nqpoly/nqpoly~/*.pd \
		$(helpdir)/$(NQPOLY_NAME)
	install -d $(examplesdir)/$(NQPOLY_NAME)
	install -p $(shell ls -1 $(abstractions_src)/nqpoly/nqpoly~/*.pd | \
		grep -v '\-help.pd')  $(examplesdir)/$(NQPOLY_NAME)


NQPOLY4_NAME = nqpoly4
nqpoly_install4:
	install -d $(objectsdir)/$(NQPOLY4_NAME)
	install -p $(shell ls -1 $(abstractions_src)/nqpoly/nqpoly4/*.pd | \
		grep -v '\-help.pd')  $(objectsdir)/$(NQPOLY4_NAME)
	install -d $(helpdir)/$(NQPOLY4_NAME)
# all of the patches should be installed into help since [nqpoly4] needs to be
# in the same directory as the patches its manipulating
	install -p $(abstractions_src)/nqpoly/nqpoly4/*.pd $(helpdir)/$(NQPOLY4_NAME)
	install -d $(examplesdir)/$(NQPOLY4_NAME)
	install -p $(shell ls -1 $(abstractions_src)/nqpoly/nqpoly4/*.pd | \
		grep -v '\-help.pd')  $(examplesdir)/$(NQPOLY4_NAME)

#-------------------------------------------------------------------------------
# nusmuk
NUSMUK_NAME = nusmuk
nusmuk_install:
	install -d $(objectsdir)/$(NUSMUK_NAME)
	install -p $(shell ls -1 $(abstractions_src)/nusmuk/*.pd | \
		grep -v '\-help.pd')  $(objectsdir)/$(NUSMUK_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(NUSMUK_NAME) \
		--author "Cyrille Henry" \
		--description "a collection of objects for physical modelling"
	install -d $(helpdir)/$(NUSMUK_NAME)
	install -p $(abstractions_src)/nusmuk/*-help.pd \
		$(helpdir)/$(NUSMUK_NAME)
	install -p $(abstractions_src)/nusmuk/*.wav \
		$(helpdir)/$(NUSMUK_NAME)


#-------------------------------------------------------------------------------
# parazit
parazit_install:
	install -d $(objectsdir)
	install -p $(abstractions_src)/parazit/parazit.pd $(objectsdir)



#-------------------------------------------------------------------------------
# PDDP support lib
PDDP_NAME = pddp
pddp_install:
	install -d $(objectsdir)/$(PDDP_NAME)
	install -p $(shell ls -1 $(abstractions_src)/pddp/*.pd | \
		grep -v '\-help.pd')  $(objectsdir)/$(PDDP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDDP_NAME) \
		--license "GNU GPL" \
		--description "support objects for the Pure Data Documentation Project"
	install -d $(helpdir)/$(PDDP_NAME)
	install -p $(abstractions_src)/pddp/*-help.pd \
		$(helpdir)/$(PDDP_NAME)
#	install -d $(manualsdir)/$(PDDP_NAME)
#	install -d $(examplesdir)/$(PDDP_NAME)


#-------------------------------------------------------------------------------
# pixeltango
PIXELTANGO_NAME = pixeltango
pixeltango_install:
	install -d $(objectsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/abstractions/*.pd \
		$(objectsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/abstractions/fx/*.pd \
		$(objectsdir)/$(PIXELTANGO_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PIXELTANGO_NAME) \
		--author "Ben Bogart" \
		--license "GNU GPL" \
		--version "0.3.4" \
		--description "objects for creating visuals in a live performance setting"
	install -d $(helpdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/help/*.* \
		$(helpdir)/$(PIXELTANGO_NAME)
	install -d $(examplesdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/Example-Patches/*.* \
		$(examplesdir)/$(PIXELTANGO_NAME)
	install -d $(manualsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/*.txt \
		$(manualsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/dev-notes/*.txt \
		$(manualsdir)/$(PIXELTANGO_NAME)



#-------------------------------------------------------------------------------
# RRADical
RRADICAL_NAME = rradical
rradical_install:
	install -d $(objectsdir)/$(RRADICAL_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(RRADICAL_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL"
	install -d $(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/control/*.* | \
		grep -v '\-help.pd')  $(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/effects/*.* | \
		grep -v '\-help.pd')  $(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/instruments/*.* | \
		grep -v '\-help.pd')  $(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/stuff/*.* | \
		grep -v '\-help.pd')  $(objectsdir)/$(RRADICAL_NAME)
	install -d $(examplesdir)/$(RRADICAL_NAME)
	install -p $(abstractions_src)/rradical/usecases/*.* \
		$(examplesdir)/$(RRADICAL_NAME)
	install -d $(examplesdir)/$(RRADICAL_NAME)/showcase
	install -p $(abstractions_src)/rradical/usecases/showcase/*.* \
		$(examplesdir)/$(RRADICAL_NAME)/showcase
	install -d $(examplesdir)/$(RRADICAL_NAME)/showcase/impl
	install -p $(abstractions_src)/rradical/usecases/showcase/impl/*.* \
		$(examplesdir)/$(RRADICAL_NAME)/showcase/impl
	install -d $(examplesdir)/$(RRADICAL_NAME)/showcase/songs
	install -p $(abstractions_src)/rradical/usecases/showcase/songs/*.* \
		$(examplesdir)/$(RRADICAL_NAME)/showcase/songs

#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks: devsymlinks_keyboardkeys


devsymlinks_keyboardkeys:
	ln -s abs \
		$(abstractions_src)/keyboardkeys/keyboardkeys
	ln -s ../keyboardkeys \
		$(abstractions_src)/keyboardkeys/doc/keyboardkeys

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#

# the destination-specific clean targets are in Makefile.buildlayout
clean: install_clean

distclean: clean cruft_clean
