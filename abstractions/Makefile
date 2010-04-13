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
cvs_root_dir := $(shell cd $(CWD)/.. && pwd)
DESTDIR = $(CWD)/build/
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# OVERARCHING BUILD TARGETS
#
#==============================================================================#

# if your library isn't included in LIB_TARGETS, it won't be built with
# Pd-extended.  
#
# WARNING!  this MUST be all on one line because the automatic package
# building scripts rely on it being that way.
LIB_TARGETS = controctopus jmmmp keyboardkeys la-kitchen list-abs memento nusmuk nqpoly nsend parazit pixeltango purepd rradical rtc-lib timestretch memento-p sfruit pd-wavelet 

# in case anything needs to be compiled or processed somehow before installing
all:

# clean up after everything is installed
final_setup:
	chmod -R ugo-w $(pddocdir)

install: all $(objectsdir) $(helpdir) $(manualsdir) $(examplesdir) \
$(patsubst %, %_install,$(LIB_TARGETS))
	@echo " "
	@echo "abstractions install succeeded!"

#==============================================================================#
#
# PROJECT TARGETS
#
#==============================================================================#


#-------------------------------------------------------------------------------
# controctopus
CONTROCTOPUS_NAME = controctopus
controctopus_install:
	install -d $(DESTDIR)$(objectsdir)/$(CONTROCTOPUS_NAME)
	install -p $(abstractions_src)/sfruit/controctopus/*.pd \
		$(DESTDIR)$(objectsdir)/$(CONTROCTOPUS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(CONTROCTOPUS_NAME) \
		--author "Luke Iannini" \
		--license "GNU GPL" \
		--description "objects for learning and mapping midi and OSC controllers to pdpatches with scaling and parameter discovery - requires memento-p"

#-------------------------------------------------------------------------------
# ds-abs
ds-abs_NAME = ds-abs
ds-abs_install:
	install -d $(DESTDIR)$(objectsdir)/$(ds-abs_NAME)
	install -p $(abstractions_src)/sfruit/ds-abs/*.pd $(DESTDIR)$(objectsdir)/$(ds-abs_NAME)
	install -p $(abstractions_src)/sfruit/ds-abs/help/*-help.pd \
		 $(DESTDIR)$(objectsdir)/$(ds-abs_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(ds-abs_NAME) \
		--author "Luke Iannini" \
		--description "" \
		--license "GNU GPL" \
		--version "a comprehensive collection of objects for working with data structures" 

#-------------------------------------------------------------------------------
# GYRE
GYRE_NAME = gyre
gyre_install:
	install -d $(DESTDIR)$(objectsdir)/$(GYRE_NAME)
	install -p $(abstractions_src)/audionerd/GYRE/gyre.*.pd \
		 $(DESTDIR)$(objectsdir)/$(GYRE_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(GYRE_NAME)
	install -p $(abstractions_src)/audionerd/GYRE/gyre.html \
		 $(DESTDIR)$(manualsdir)/$(GYRE_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(GYRE_NAME)
	install -p $(abstractions_src)/audionerd/GYRE/gyre.pd \
		 $(DESTDIR)$(examplesdir)/$(GYRE_NAME)


#-------------------------------------------------------------------------------
# JMMMP
JMMMP_NAME = jmmmp
jmmmp_install:
	install -d $(DESTDIR)$(objectsdir)/$(JMMMP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(JMMMP_NAME) \
		--author "Joao Miguel Pais <jmmmpais@googlemail.com>" \
		--description "" \
		--license "BSD" \
		--version "" 
	install -p $(abstractions_src)/jmmmp/*.pd \
		 $(DESTDIR)$(objectsdir)/$(JMMMP_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(JMMMP_NAME)
#	install -p $(abstractions_src)/jmmmp/*.html \
#		 $(DESTDIR)$(manualsdir)/$(JMMMP_NAME)
#	install -d $(DESTDIR)$(examplesdir)/$(JMMMP_NAME)
#	install -p $(abstractions_src)/jmmmp/examples/jmmmp.pd \
#		 $(DESTDIR)$(examplesdir)/$(JMMMP_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(abstractions_src)/jmmmp/jmmmp-ext.txt \
		$(DESTDIR)$(readmesdir)/$(JMMMP_NAME).txt
	install -d $(DESTDIR)$(objectsdir)/$(JMMMP_NAME)/manual
	install -p $(abstractions_src)/jmmmp/jmmmp-ext.txt \
		$(DESTDIR)$(objectsdir)/$(JMMMP_NAME)/manual/



#-------------------------------------------------------------------------------
# keyboardkeys
KEYBOARDKEYS_NAME = keyboardkeys
keyboardkeys_install:
	install -d $(DESTDIR)$(objectsdir)/$(KEYBOARDKEYS_NAME)
	install -p $(abstractions_src)/$(KEYBOARDKEYS_NAME)/abs/*.pd \
		 $(DESTDIR)$(objectsdir)/$(KEYBOARDKEYS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(KEYBOARDKEYS_NAME) \
		--author "Andrey Savitsky" \
		--description "objects for using keyboard keys for scrolling and selecting"
	install -p $(abstractions_src)/$(KEYBOARDKEYS_NAME)/doc/*-help.pd \
		 $(DESTDIR)$(objectsdir)/$(KEYBOARDKEYS_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(KEYBOARDKEYS_NAME)/examples
	install -p $(abstractions_src)/$(KEYBOARDKEYS_NAME)/keyboard_main.pd \
		 $(DESTDIR)$(objectsdir)/$(KEYBOARDKEYS_NAME)/examples

#-------------------------------------------------------------------------------
# list-abs
LIST-ABS_NAME = list-abs
list-abs_install:
	install -d $(DESTDIR)$(objectsdir)/$(LIST-ABS_NAME)
	install -p $(abstractions_src)/footils/list-abs/*.pd \
		$(DESTDIR)$(objectsdir)/$(LIST-ABS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(LIST-ABS_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL" \
		--description "a collection of objects for manipulating lists. Requires pd>=0.39"
	install -p $(abstractions_src)/footils/list-abs/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(LIST-ABS_NAME)



#-------------------------------------------------------------------------------
# la-kitchen
LA-KITCHEN_NAME = la-kitchen
la-kitchen_install:
	install -d $(DESTDIR)$(objectsdir)/$(LA-KITCHEN_NAME)
	install -p $(abstractions_src)/La-kitchen/*.pd \
		$(DESTDIR)$(objectsdir)/$(LA-KITCHEN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(LA-KITCHEN_NAME) \
		--author "Charles Verron" \
		--description "a collection of objects working with sensors"
	install -p $(abstractions_src)/La-kitchen/*-help.pd \
		 $(DESTDIR)$(objectsdir)/$(LA-KITCHEN_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(LA-KITCHEN_NAME)
#	install -p $(abstractions_src)/La-kitchen/doc/*.* \
#		 $(DESTDIR)$(manualsdir)/$(LA-KITCHEN_NAME)
	install -p $(abstractions_src)/La-kitchen/readme.txt \
		$(DESTDIR)$(objectsdir)/$(LA_KITCHEN_NAME)/README.txt


#-------------------------------------------------------------------------------
# memento
MEMENTO_NAME = memento
memento_install:
	install -d $(DESTDIR)$(objectsdir)/$(MEMENTO_NAME)
	install -p $(abstractions_src)/rradical/memento/*.pd \
		$(DESTDIR)$(objectsdir)/$(MEMENTO_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MEMENTO_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL" \
		--description "a collection of objects for managing state saving"
	install -d $(DESTDIR)$(manualsdir)/$(MEMENTO_NAME)
	install -p $(abstractions_src)/rradical/memento/tutorial/*.* \
		$(DESTDIR)$(manualsdir)/$(MEMENTO_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(MEMENTO_NAME)/manual
	install -p $(abstractions_src)/rradical/memento/tutorial/*.* \
		$(DESTDIR)$(objectsdir)/$(MEMENTO_NAME)/manual
	install -d $(DESTDIR)$(examplesdir)/$(MEMENTO_NAME)
	install -p $(abstractions_src)/rradical/memento/examples/*.* \
		$(DESTDIR)$(examplesdir)/$(MEMENTO_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(MEMENTO_NAME)/examples
	install -p $(abstractions_src)/rradical/memento/examples/*.* \
		$(DESTDIR)$(objectsdir)/$(MEMENTO_NAME)/examples

#-------------------------------------------------------------------------------
# memento-p
MEMENTO-P_NAME = memento-p
memento-p_install:
	install -d $(DESTDIR)$(objectsdir)/$(MEMENTO-P_NAME)
	install -p $(abstractions_src)/sfruit/memento-p/*.pd \
		$(DESTDIR)$(objectsdir)/$(MEMENTO-P_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MEMENTO-P_NAME) \
		--author "Luke Iannini" \
		--license "GNU GPL" \
		--description "an addon for Frank Barknecht's Memento adding local-state saving and support for Controctopus (a controller lib)"

#-------------------------------------------------------------------------------
# nqpoly
nqpoly_install: nqpoly_install~ nqpoly_install4


NQPOLY_NAME = nqpoly~
nqpoly_install~:
	install -d $(DESTDIR)$(objectsdir)/$(NQPOLY_NAME)
	install -p $(abstractions_src)/nqpoly/nqpoly~/*.pd \
		$(DESTDIR)$(objectsdir)/$(NQPOLY_NAME)
	install -p $(abstractions_src)/nqpoly/nqpoly~/*.pd \
		$(DESTDIR)$(objectsdir)/$(NQPOLY_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(NQPOLY_NAME)/manual
	install -p $(abstractions_src)/nqpoly/nqpoly~/readme.txt	\
		$(DESTDIR)$(objectsdir)/$(NQPOLY_NAME)/manual
	install -p $(abstractions_src)/nqpoly/*.html \
		$(DESTDIR)$(objectsdir)/$(NQPOLY_NAME)/manual


NQPOLY4_NAME = nqpoly4
nqpoly_install4:
	install -d $(DESTDIR)$(objectsdir)/$(NQPOLY4_NAME)
	install -p $(abstractions_src)/nqpoly/nqpoly4/*.pd \
		$(DESTDIR)$(objectsdir)/$(NQPOLY4_NAME)
	install -p $(abstractions_src)/nqpoly/nqpoly4/*.pd \
		$(DESTDIR)$(objectsdir)/$(NQPOLY4_NAME)
	install -p $(abstractions_src)/nqpoly/*/*-help.pd $(DESTDIR)$(helpdir)
	install -d $(DESTDIR)$(objectsdir)/$(NQPOLY4_NAME)/manual
	install -p $(abstractions_src)/nqpoly/*.html \
		$(DESTDIR)$(objectsdir)/$(NQPOLY4_NAME)/manual

#-------------------------------------------------------------------------------
# nsend
NSEND_NAME = nsend
nsend_install:
	install -d $(DESTDIR)$(objectsdir)/$(NSEND_NAME)
	install -p $(shell ls -1 $(abstractions_src)/sfruit/nsend/*.pd | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(NSEND_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(NSEND_NAME) \
		--author "Luke Iannini" \
		--license "GNU GPL" \
		--description "multiple-inlet/outlet send and receive"

#-------------------------------------------------------------------------------
# nusmuk
NUSMUK_NAME = nusmuk
nusmuk_install:
	install -d $(DESTDIR)$(objectsdir)/$(NUSMUK_NAME)
	install -p $(abstractions_src)/nusmuk/*.pd \
		$(DESTDIR)$(objectsdir)/$(NUSMUK_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(NUSMUK_NAME) \
		--author "Cyrille Henry" \
		--description "a collection of objects for physical modelling"
	install -p $(abstractions_src)/nusmuk/*.wav \
		$(DESTDIR)$(objectsdir)/$(NUSMUK_NAME)



#-------------------------------------------------------------------------------
# parazit
parazit_install:
	install -d $(DESTDIR)$(objectsdir)
	install -p $(abstractions_src)/parazit/parazit.pd $(DESTDIR)$(objectsdir)
	install -p $(abstractions_src)/parazit/parazit-help.pd $(DESTDIR)$(objectsdir)



#-------------------------------------------------------------------------------
# mtl
MTL_NAME = mtl
mtl_install:
	install -d $(DESTDIR)$(objectsdir)/$(MTL_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MTL_NAME) \
		--author "Thomas O Fredericks and 11h11" \
		--license "If you use and die, it's not our fault" \
		--description "(Pure Data) Montreal Abstractions"
	install -p $(shell ls -1 $(abstractions_src)/$(MTL_NAME)/*.pd | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(MTL_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MTL_NAME)
	install -p $(abstractions_src)/$(MTL_NAME)/*-help.pd \
		$(DESTDIR)$(helpdir)/$(MTL_NAME)
	install -p $(abstractions_src)/$(MTL_NAME)/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(MTL_NAME)
	install -p $(abstractions_src)/$(MTL_NAME)/browser.pd \
		$(DESTDIR)$(helpdir)/$(MTL_NAME)
#	install -p $(abstractions_src)/$(MTL_NAME)/1.mtlBrowser.pd \
#		$(DESTDIR)$(helpdir)/$(MTL_NAME)
	install -p $(abstractions_src)/$(MTL_NAME)/*.txt \
		$(DESTDIR)$(objectsdir)/$(MTL_NAME)		
	install -p $(abstractions_src)/$(MTL_NAME)/*.txt \
		$(DESTDIR)$(helpdir)/$(MTL_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(MTL_NAME)
	install -p $(abstractions_src)/$(MTL_NAME)/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(MTL_NAME)


#-------------------------------------------------------------------------------
# pdsvn
PDSVN_NAME = pdsvn
pdsvn_install:
	install -d $(DESTDIR)$(objectsdir)/$(PDSVN_NAME)
	install -p $(shell ls -1 $(abstractions_src)/sfruit/pdsvn/*.pd | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(PDSVN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDSVN_NAME) \
		--author "Luke Iannini" \
		--license "GNU GPL" \
		--description "objects for interacting with Subversion from within Pd patches (experimental, but usable)"
	install -d $(DESTDIR)$(helpdir)/$(PDSVN_NAME)
	install -p $(abstractions_src)/sfruit/pdsvn/*-help.pd \
		$(DESTDIR)$(helpdir)/$(PDSVN_NAME)

#-------------------------------------------------------------------------------
# pixeltango
PIXELTANGO_NAME = pixeltango
pixeltango_install:
	install -d $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/abstractions/*.pd \
		$(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/fx
	install -p $(abstractions_src)/pixelTANGO/abstractions/fx/*.pd \
		$(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/fx
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PIXELTANGO_NAME) \
		--author "Ben Bogart" \
		--license "GNU GPL" \
		--version "0.3.4" \
		--description "objects for creating visuals in a live performance setting"
	install -p $(abstractions_src)/pixelTANGO/help/*.* \
		$(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/Example-Patches/*.* \
		$(DESTDIR)$(examplesdir)/$(PIXELTANGO_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/*.txt \
		$(DESTDIR)$(manualsdir)/$(PIXELTANGO_NAME)
	install -p $(abstractions_src)/pixelTANGO/dev-notes/*.txt \
		$(DESTDIR)$(manualsdir)/$(PIXELTANGO_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/manual
	install -p $(abstractions_src)/pixelTANGO/*.txt \
		$(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/manual
	install -p $(abstractions_src)/pixelTANGO/dev-notes/*.txt \
		$(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/manual
	install -d $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/examples
	install -p $(abstractions_src)/pixelTANGO/Example-Patches/*.* \
		$(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/examples

pixeltango_clean:
	-rm -f -- $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/fx/*.*
	-rmdir -- $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/fx
	-rm -f -- $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)/*.*
	-rmdir -- $(DESTDIR)$(objectsdir)/$(PIXELTANGO_NAME)
	-rm -f -- $(DESTDIR)$(examplesdir)/$(PIXELTANGO_NAME)/*.*
	-rmdir -- $(DESTDIR)$(examplesdir)/$(PIXELTANGO_NAME)
	-rm -f -- $(DESTDIR)$(manualsdir)/$(PIXELTANGO_NAME)/*.*
	-rmdir -- $(DESTDIR)$(manualsdir)/$(PIXELTANGO_NAME)


#------------------------------------------------------------------------------#
# PUREPD
purepd:
	make -C $(abstractions_src)/purepd

purepd_install:
	make -C $(abstractions_src)/purepd DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

purepd_clean:
	make -C $(abstractions_src)/purepd clean


#-------------------------------------------------------------------------------
# RRADical
RRADICAL_NAME = rradical
rradical_install:
	install -d $(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(RRADICAL_NAME) \
		--author "Frank Barknecht" \
		--license "GNU GPL"
	install -d $(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/control/*.* | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/effects/*.* | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/instruments/*.* | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	install -p $(shell ls -1 $(abstractions_src)/rradical/stuff/*.* | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	install -p $(abstractions_src)/rradical/*/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(RRADICAL_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)
	install -p $(abstractions_src)/rradical/usecases/*.* \
		$(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)/showcase
	install -p $(abstractions_src)/rradical/usecases/showcase/*.* \
		$(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)/showcase
	install -d $(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)/showcase/impl
	install -p $(abstractions_src)/rradical/usecases/showcase/impl/*.* \
		$(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)/showcase/impl
	install -d $(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)/showcase/songs
	install -p $(abstractions_src)/rradical/usecases/showcase/songs/*.* \
		$(DESTDIR)$(examplesdir)/$(RRADICAL_NAME)/showcase/songs

#-------------------------------------------------------------------------------
# rtc-lib
RTC-LIB_NAME = rtc
rtc-lib_install:
	install -d $(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)
	install -p $(abstractions_src)/footils/rtc-lib/rtc/*.pd \
		$(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(RTC-LIB_NAME) \
		--author "Karlheinz Essl and Frank Barknecht" \
		--license "Pd license" \
		--description "Realtime Composition Library for Max ported to Pd"
	install -p $(abstractions_src)/footils/rtc-lib/rtc/*-help.pd \
		$(abstractions_src)/footils/rtc-lib/rtc/RTC-Overview.pd \
		$(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)/rtc-help
	install -p $(abstractions_src)/footils/rtc-lib/rtc/rtc-help/*.pd \
		$(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)/rtc-help
	install -d $(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)/examples
	install -p $(abstractions_src)/footils/rtc-lib/rtc/rtc-help/*.pd \
		$(DESTDIR)$(objectsdir)/$(RTC-LIB_NAME)/examples


#-------------------------------------------------------------------------------
# PUREMEASUREMENT
PUREMEASURE_NAME = puremeasurement
puremeasurement_install:
	install -d $(DESTDIR)$(objectsdir)/$(PUREMEASURE_NAME)
	install -p $(abstractions_src)/puremeasurement/applications/*.pd $(DESTDIR)$(objectsdir)/$(PUREMEASURE_NAME)
	install -p $(abstractions_src)/puremeasurement/tools/*.pd $(DESTDIR)$(objectsdir)/$(PUREMEASURE_NAME)
	install -p $(abstractions_src)/puremeasurement/testsig/*.wav $(DESTDIR)$(objectsdir)/$(PUREMEASURE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PUREMEASURE_NAME) \
		--author "Matthias Blau" \
		--description "patches for acoustical measurements" \
		--license "GNU GPL v2" \
		--version "v28II06"
	install -d $(DESTDIR)$(examplesdir)/$(PUREMEASURE_NAME)
	install -p $(abstractions_src)/puremeasurement/applications/*.pd \
		 $(DESTDIR)$(examplesdir)/$(PUREMEASURE_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(abstractions_src)/puremeasurement/readme \
		$(DESTDIR)$(readmesdir)/$(PUREMEASURE_NAME).txt


#-------------------------------------------------------------------------------
# sfruit
SFRUIT_NAME = sfruit
sfruit_install:
	install -d $(DESTDIR)$(objectsdir)/$(SFRUIT_NAME)
	install -p $(shell ls -1 $(abstractions_src)/sfruit/sfruit/*.pd | \
		grep -v '\-help.pd')  $(DESTDIR)$(objectsdir)/$(SFRUIT_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(SFRUIT_NAME) \
		--author "Luke Iannini" \
		--license "GNU GPL" \
		--description "a large collection of assorted handy utilites"
	install -d $(DESTDIR)$(helpdir)/$(SFRUIT_NAME)
	install -p $(abstractions_src)/sfruit/sfruit/*-help.pd \
		$(DESTDIR)$(helpdir)/$(SFRUIT_NAME)
	install -p $(abstractions_src)/sfruit/sfruit/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(SFRUIT_NAME)

#-------------------------------------------------------------------------------
# TIMESTRETCH
TIMESTRETCH_NAME = timestretch
timestretch_install:
	install -d $(DESTDIR)$(objectsdir)/$(TIMESTRETCH_NAME)
	install -p $(abstractions_src)/timestretch/*.pd $(DESTDIR)$(objectsdir)/$(TIMESTRETCH_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(TIMESTRETCH_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version "" 
#	install -d $(DESTDIR)$(manualsdir)/$(TIMESTRETCH_NAME)
#	install -p $(abstractions_src)/timestretch/doc/*.txt \
#		 $(DESTDIR)$(manualsdir)/$(TIMESTRETCH_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(abstractions_src)/timestretch/README \
#		$(DESTDIR)$(readmesdir)/$(TIMESTRETCH_NAME).txt


#-------------------------------------------------------------------------------
# SSSAD
SSSAD_NAME = sssad
sssad_install:
	install -d $(DESTDIR)$(objectsdir)/$(SSSAD_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(SSSAD_NAME)/_sssad
	install -p $(abstractions_src)/footils/sssad/*.pd $(DESTDIR)$(objectsdir)/$(SSSAD_NAME)
	install -p $(abstractions_src)/footils/sssad/_sssad/*.pd $(DESTDIR)$(objectsdir)/$(SSSAD_NAME)/_sssad
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(SSSAD_NAME) \
		--author "Frank Barknecht" \
		--description "Stupid-super-simplistic State-saving for PD" \
		--license "same as Pd's license" \
		--version "" 
	install -d $(DESTDIR)$(helpdir)/$(SSSAD_NAME)
	install -p $(abstractions_src)/footils/sssad/*-help.pd \
		 $(DESTDIR)$(helpdir)/$(SSSAD_NAME)
	install -p $(abstractions_src)/footils/sssad/_sssad/*-help.pd \
		 $(DESTDIR)$(helpdir)/$(SSSAD_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(SSSAD_NAME)
	install -p $(abstractions_src)/footils/sssad/sssad-example.pd \
		 $(DESTDIR)$(examplesdir)/$(SSSAD_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(abstractions_src)/footils/sssad/README.txt \
		$(DESTDIR)$(readmesdir)/$(SSSAD_NAME).txt


#-------------------------------------------------------------------------------
# PDWAVELET
PDWAVELET_NAME = pd-wavelet
pd-wavelet_install:
	install -d $(DESTDIR)$(objectsdir)/$(PDWAVELET_NAME)
	install -p $(abstractions_src)/pd-wavelet/*.pd \
		$(DESTDIR)$(objectsdir)/$(PDWAVELET_NAME)
	install -p $(abstractions_src)/pd-wavelet/abs/*.pd \
		$(DESTDIR)$(objectsdir)/$(PDWAVELET_NAME)
	install -p $(abstractions_src)/pd-wavelet/main.pd \
		 $(DESTDIR)$(objectsdir)/$(PDWAVELET_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDWAVELET_NAME) \
		--author "Raul Diaz Poblete" \
		--description "manipulation of audio in the wavelet domain" \
		--license "GNU GPL v2" \
		--version "" 
	install -d $(DESTDIR)$(readmesdir)
	install -p $(abstractions_src)/pd-wavelet/README.txt \
		$(DESTDIR)$(readmesdir)/$(PDWAVELET_NAME).txt


#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks:
# make symlinks for help patches
	test -e $(abstractions_src)/../2.control.examples || \
		ln -s $(pd_src)/doc/2.control.examples \
			$(abstractions_src)/../2.control.examples
	test -e $(abstractions_src)/../3.audio.examples || \
		ln -s $(pd_src)/doc/3.audio.examples \
			$(abstractions_src)/../3.audio.examples
# pd <= 0.38-4 has 4.fft.examples
	test -e $(abstractions_src)/../4.fft.examples || \
		ln -s $(pd_src)/doc/4.fft.examples $(abstractions_src)/../4.fft.examples
# pd > 0.38-4 has 4.data.structures
	test -e $(abstractions_src)/../4.data.structures || \
		ln -s $(pd_src)/doc/4.data.structures \
			$(abstractions_src)/../4.data.structures
	test -e $(abstractions_src)/../7.stuff || \
		ln -s $(pd_src)/doc/7.stuff $(abstractions_src)/../7.stuff
	test -e $(abstractions_src)/../sound || \
		ln -s $(pd_src)/doc/sound $(abstractions_src)/../sound

# hmm, seems broken and unneeded
#devsymlinks_keyboardkeys:
#	test -e abs || \
#		ln -s abs $(abstractions_src)/keyboardkeys/keyboardkeys
#	test -e ../keyboardkeys || \
#		ln -s ../keyboardkeys $(abstractions_src)/keyboardkeys/doc/keyboardkeys

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#

# the destination-specific clean targets are in Makefile.buildlayout
clean: install_clean

distclean: clean cruft_clean




test_locations:
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_VERSION: $(PACKAGE_VERSION)"
	@echo "CWD $(CWD)"
	@echo "DESTDIR $(DESTDIR)"
	@echo "PREFIX $(prefix)"
	@echo "BINDIR  $(bindir)"
	@echo "LIBDIR  $(libdir)"
	@echo "OBJECTSDIR  $(objectsdir)"
	@echo "PDDOCDIR  $(pddocdir)"
	@echo "LIBPDDIR  $(libpddir)"
	@echo "LIBPDBINDIR  $(libpdbindir)"
	@echo "HELPDIR  $(helpdir)"
	@echo "MANUALSDIR  $(manualsdir)"
	@echo "EXAMPLESDIR  $(examplesdir)"
