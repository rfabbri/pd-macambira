
#
# makefile for copying all of the help docs into this folder for 
# inclusion into the externals packages
#

CWD := $(shell pwd)

# these are setup to be overridden by the packages/Makefile
SRC_ROOT_DIR := $(CWD)/../../..
INSTALL_PREFIX = .
BUILDLAYOUT_DIR = $(CWD)/..

all: install
	echo "Completed externals/doc build!"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# this is here to retain compatibility to the way that this makefile copied
# files before it was added to the unified build system
HELP_DEST = .

install: $(HELP_DEST)
#----------------------------------------------------------------------------
# all standard objs' help files
#    it had to be broken up because the list is soo long
	install -p \
			$(EXTERNALS_SRC)/OSCx/doc/*.pd \
			$(EXTERNALS_SRC)/control/*/*.pd \
			$(EXTERNALS_SRC)/signal/*/*.pd \
			$(EXTERNALS_SRC)/aenv~/*.pd \
			$(EXTERNALS_SRC)/arraysize/*.pd \
			$(EXTERNALS_SRC)/beatpipe/*.pd \
			$(EXTERNALS_SRC)/build/*.pd \
			$(EXTERNALS_SRC)/bbogart/chaos/tools/*.pd \
			$(EXTERNALS_SRC)/bbogart/*/*.pd \
			$(EXTERNALS_SRC)/creb/doc/*.pd \
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/creb/doc/examples/*.pd \
			$(EXTERNALS_SRC)/cxc/reference/*.pd \
			$(EXTERNALS_SRC)/dfx/*/*.pd \
			$(EXTERNALS_SRC)/ext13/doc/*.pd \
			$(EXTERNALS_SRC)/ggee/*/*-help.pd \
			$(EXTERNALS_SRC)/gem2pdp/*.pd \
			$(EXTERNALS_SRC)/ff/*.pd \
			$(EXTERNALS_SRC)/freeverb~/*.pd \
			$(EXTERNALS_SRC)/hcs/*-help.pd \
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/hcs/*/doc/*.pd \
			$(EXTERNALS_SRC)/hcs/hid/examples/*.pd \
			$(EXTERNALS_SRC)/hcs/pan/*-help.pd \
			$(EXTERNALS_SRC)/iem/*/*/*-help.pd \
			$(EXTERNALS_SRC)/markex/*.pd \
			$(EXTERNALS_SRC)/maxlib/help/*.* \
			$(EXTERNALS_SRC)/mjlib/doc/*.pd \
			$(EXTERNALS_SRC)/motex/*.pd \
			$(EXTERNALS_SRC)/nusmuk/line3/*.pd\
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/pdogg/*/*.pd \
			$(EXTERNALS_SRC)/plugin~/*.pd \
			$(EXTERNALS_SRC)/pmpd/help/*.pd\
			$(EXTERNALS_SRC)/rhythm_estimator/*.p? \
			$(EXTERNALS_SRC)/sprinkler/sprinkler-help.pd \
			$(EXTERNALS_SRC)/susloop~/*.pd \
			$(EXTERNALS_SRC)/svf~/*.pd \
			$(EXTERNALS_SRC)/vbap/*.pd \
			$(EXTERNALS_SRC)/vst/*.pd \
			$(EXTERNALS_SRC)/zhzxh~/*.pd \
				$(HELP_DEST)
#----------------------------------------------------------------------------
# IEMlib uses its own dir and since its not maintained in CVS
# its best not to change the objects
	install -d $(HELP_DEST)/iemhelp
	install -p $(EXTERNALS_SRC)/iemlib/iemhelp/*.* $(HELP_DEST)/iemhelp
#----------------------------------------------------------------------------
# PMDP examples
	install -d  $(HELP_DEST)/pmpd
# pmpd exemples files has to go in an other directory
	install -p $(EXTERNALS_SRC)/pmpd/exemples/*.pd $(HELP_DEST)/pmpd
# pmpd documentation
	cp -Rp $(EXTERNALS_SRC)/pmpd/doc $(HELP_DEST)/pmpd/doc
#----------------------------------------------------------------------------
# zexy uses its own dir
	install -d $(HELP_DEST)/zexy
	install -p $(EXTERNALS_SRC)/zexy/zexy.pd  $(HELP_DEST)
	install -p $(EXTERNALS_SRC)/zexy/examples/*.* $(HELP_DEST)/zexy
#----------------------------------------------------------------------------
# iemmatrix uses its own dir
	install -d $(HELP_DEST)/iemmatrix
	install -p $(EXTERNALS_SRC)/iem/iemmatrix/doc/*.pd  $(HELP_DEST)/iemmatrix


#----------------------------------------------------------------------------
# remove help files for objects that are not included due to conflicts, etc.
#
#  mjlib [prob] conflicts with cyclone's [prob]
	rm $(HELP_DEST)/prob-help.pd
#  maxlib conflicts with cyclone
	rm $(HELP_DEST)/split-help.pd


#----------------------------------------------------------------------------
# this is mildly dangerous, since it just deletes everything
clean:
	cd $(HELP_DEST) && rm -f -- *.pd *.ps *.txt test.som *~
	rm -Rf -- $(HELP_DEST)/iemhelp $(HELP_DEST)/zexy \
		$(HELP_DEST)/pmpd $(HELP_DEST)/iemmatrix

