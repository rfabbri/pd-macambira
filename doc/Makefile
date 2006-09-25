#==============================================================================#
#
# Centralized build system for "doc".  
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

# default target
all:
	@echo "this currently does nothing"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# OVERARCHING BUILD TARGETS
#
#==============================================================================#

DOC_TARGETS = externals-howto media messageoddness pd_fileformat pd-msg pddp \
tutorials

# clean up after everything is installed
final_setup:
	chmod -R ugo-w $(pddocdir)

install: $(objectsdir) $(helpdir) $(manualsdir) $(examplesdir) \
$(patsubst %, %_install,$(DOC_TARGETS))
	@echo " "
	@echo "doc install succeeded!"


#==============================================================================#
#
# PROJECT-SPECIFIC TARGETS
#
#==============================================================================#


#------------------------------------------------------------------------------#
# TEMPLATE
TEMPLATE_NAME = template
template_install: $(manualsdir)
	install -d $(manualsdir)/$(TEMPLATE_NAME)
	install -p $(doc_src)/template/*.* \
		$(manualsdir)/$(TEMPLATE_NAME) 

template_clean:
	-rm -f -- $(manualsdir)/$(TEMPLATE_NAME)/*.*
	-rmdir --	$(manualsdir)/$(TEMPLATE_NAME) 




#------------------------------------------------------------------------------#
# EXTERNALS-HOWTO
EXTERNALS-HOWTO_NAME = Externals-HOWTO
externals-howto_install: $(manualsdir)
	install -d $(manualsdir)/$(EXTERNALS-HOWTO_NAME)
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/pd-externals-HOWTO.pdf > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/pd-externals-HOWTO.pdf
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/HOWTO-externals-en.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/HOWTO-externals-en.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node1.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node1.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node2.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node2.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node3.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node3.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node4.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node4.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node5.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node5.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node6.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node6.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node7.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node7.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node8.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node8.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node9.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node9.html
	-curl http://iem.kug.ac.at/pd/externals-HOWTO/node10.html > \
		$(manualsdir)/$(EXTERNALS-HOWTO_NAME)/node10.html

externals-howto_clean:




#------------------------------------------------------------------------------#
# MEDIA
MEDIA_NAME = media
media_install: $(manualsdir)
	install -d $(manualsdir)/$(MEDIA_NAME)
	install -p $(doc_src)/media/*.* \
		$(manualsdir)/$(MEDIA_NAME) 

media_clean:
	-rm -f -- $(manualsdir)/$(MEDIA_NAME)/*.*
	-rmdir --	$(manualsdir)/$(MEDIA_NAME) 




#------------------------------------------------------------------------------#
# MESSAGEODDNESS
MESSAGEODDNESS_NAME = MessageOddness
messageoddness_install: $(manualsdir)
	install -d $(manualsdir)/$(MESSAGEODDNESS_NAME)
	install -p $(doc_src)/additional/messageoddness/*.* \
		$(manualsdir)/$(MESSAGEODDNESS_NAME) 

messageoddness_clean:
	-rm -f -- $(manualsdir)/$(MESSAGEODDNESS_NAME)/*.*
	-rmdir --	$(manualsdir)/$(MESSAGEODDNESS_NAME) 




#------------------------------------------------------------------------------#
# PD-MSG
PD-MSG_NAME = pd-msg
pd-msg_install: $(manualsdir)
	install -d $(manualsdir)/$(PD-MSG_NAME)
	install -p $(doc_src)/additional/pd-msg/*.txt \
		$(manualsdir)/$(PD-MSG_NAME) 
	for dir in $(shell cd  $(doc_src)/additional/pd-msg && ls -d [1-5].*); do \
		echo "Including $$dir in $(PD-MSG_NAME)"; \
		install -d $(manualsdir)/$(PD-MSG_NAME)/$$dir; \
		install -p $(doc_src)/additional/pd-msg/$$dir/*.* \
			$(manualsdir)/$(PD-MSG_NAME)/$$dir; \
	done

pd-msg_clean:


#------------------------------------------------------------------------------#
# PD_FILEFORMAT
PD_FILEFORMAT_NAME = Pd
pd_fileformat_install: $(manualsdir)
	install -d $(manualsdir)/$(PD_FILEFORMAT_NAME)
	install -p $(doc_src)/additional/pd-fileformat.html \
		$(manualsdir)/$(PD_FILEFORMAT_NAME)/Pd_File_Format.html

pd_fileformat_clean:


#------------------------------------------------------------------------------#
# PDDP
pddp_install: $(helpdir)
	install -p $(doc_src)/pddp/*.pd $(helpdir)
# this file is used in key-help.pd
	install -p $(doc_src)/pddp/keyboard_fun.txt $(helpdir)

pddp_clean:




#------------------------------------------------------------------------------#
# TUTORIALS
tutorials_install: $(manualsdir)
# pddrums
	install -d $(manualsdir)/PdDrums
	install -p $(doc_src)/tutorials/footils/pddrums/*.* $(manualsdir)/PdDrums
# playnow
	install -d $(manualsdir)/PlayNow
	install -p $(doc_src)/tutorials/playnow/*.* $(manualsdir)/PlayNow
# intro
	install -d $(manualsdir)/0.Intro/Intro
	install -p $(doc_src)/tutorials/intro/*.* $(manualsdir)/0.Intro/Intro
	install -p $(abstractions_src)/pddp/0-pager.pd $(manualsdir)/0.Intro/Intro
# sound
	install -d $(manualsdir)/0.Intro/Sound
	install -p $(doc_src)/tutorials/sound/*.* $(manualsdir)/0.Intro/Sound
	install -p $(abstractions_src)/pddp/0-pager.pd $(manualsdir)/0.Intro/Sound
# visual
	install -d $(manualsdir)/0.Intro/Visual
	install -p $(doc_src)/tutorials/visual/*.* $(manualsdir)/0.Intro/Visual
	install -p $(abstractions_src)/pddp/0-pager.pd $(manualsdir)/0.Intro/Visual
# networking
	install -d $(manualsdir)/0.Intro/Networking
	install -p $(doc_src)/tutorials/networking/*.* $(manualsdir)/0.Intro/Networking
	install -p $(abstractions_src)/pddp/0-pager.pd $(manualsdir)/0.Intro/Networking
# physical
	install -d $(manualsdir)/0.Intro/Physical
	install -p $(doc_src)/tutorials/physical/*.* $(manualsdir)/0.Intro/Physical
	install -p $(abstractions_src)/pddp/0-pager.pd $(manualsdir)/0.Intro/Physical

tutorials_clean:
	-rm -f -- $(manualsdir)/PdDrums/*.*
	-rmdir -- $(manualsdir)/PdDrums
	-rm -f -- $(manualsdir)/PlayNow/*.*
	-rmdir -- $(manualsdir)/PlayNow
	-rm -f -- $(manualsdir)/0.Intro/Intro/*.*
	-rmdir -- $(manualsdir)/0.Intro/Intro
	-rm -f -- $(manualsdir)/0.Intro/Sound/*.*
	-rmdir -- $(manualsdir)/0.Intro/Sound
	-rm -f -- $(manualsdir)/0.Intro/Visual/*.*
	-rmdir -- $(manualsdir)/0.Intro/Visual
	-rm -f -- $(manualsdir)/0.Intro/Networking/*.*
	-rmdir -- $(manualsdir)/0.Intro/Networking
	-rm -f -- $(manualsdir)/0.Intro/Physical/*.*
	-rmdir -- $(manualsdir)/0.Intro/Physical


#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks:
	test -e $(doc_src)/2.control.examples || \
		ln -s $(pd_src)/doc/2.control.examples $(doc_src)/2.control.examples
	test -e $(doc_src)/3.audio.examples || \
		ln -s $(pd_src)/doc/3.audio.examples $(doc_src)/3.audio.examples
# pd <= 0.38-4 has 4.fft.examples
	test -e $(doc_src)/4.fft.examples || \
		ln -s $(pd_src)/doc/4.fft.examples $(doc_src)/4.fft.examples
# pd > 0.38-4 has 4.data.structures
	test -e $(doc_src)/4.data.structures || \
		ln -s $(pd_src)/doc/4.data.structures $(doc_src)/4.data.structures
	test -e $(doc_src)/7.stuff || \
		ln -s $(pd_src)/doc/7.stuff $(doc_src)/7.stuff
	test -e $(doc_src)/sound || \
		ln -s $(pd_src)/doc/sound $(doc_src)/sound
# put the pddp support objects in the path
	test -e $(doc_src)/pddp/pddp || \
		ln -s $(abstractions_src)/pddp $(doc_src)/pddp/pddp

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#

# the destination-specific clean targets are in Makefile.buildlayout
clean: $(patsubst %, %_clean,$(DOC_TARGETS))

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
