## -*- Mode: Makefile -*-
##
## File: externals/moocow/extended/Makefile
## Author: Bryan Jurish <moocow@bbaw.de>
## Description: pd-extended makefile for moocow's externals
##

##======================================================================
## Variables

SUBDIRS_DONT = \
	../flite \
	../gfsm

SUBDIRS = \
	../deque \
	../readdir \
	../weightmap \
	../pdstring \
	../../sprinkler

##-- 'pdstring' and 'sprinkler' are already in 'flatspace' ... should they be moved here?

##-- local variables
MOOCOW_DIR    ?=$(shell pwd)
MOOCOW_BUILD  ?=$(MOOCOW_DIR)/build.moo
MOOCOW_MFLAGS ?=DESTDIR=""

CONFIGURE_ARGS=\
	CFLAGS="$(CFLAGS)" \
	--with-pd-include="$(pd_src)/src" \
	--with-pd-dir="$(MOOCOW_BUILD)"

#	--with-pd-extdir="$(MOOCOW_BUILD)/externs"

##-- defaults
CFLAGS ?= -g -O2
pd_src ?= $(CURDIR)/../../../pd


##======================================================================
## Rules: default
all: build.stamp

autogen: $(SUBDIRS:=-autogen)
configure: $(SUBDIRS:=-configure)

build.stamp: $(SUBDIRS:=/build.stamp)
	touch $@

reamde: README.txt
README.txt: README.pod
	pod2text README.pod $@

clean:
	for d in $(SUBDIRS); do $(MAKE) $(MOOCOW_MFLAGS) -C $$d clean || true; rm -f $$d/build.stamp; done;
	rm  -f build.stamp config.log
	rm -rf $(MOOCOW_BUILD)

realclean: clean
	for d in $(SUBDIRS); do $(MAKE) $(MOOCOW_MFLAGS) -C $$d realclean || true; done;

distclean: clean
	for d in $(SUBDIRS); do $(MAKE) $(MOOCOW_MFLAGS) -C $$d distclean || true; done;

cvsclean: clean
	for d in $(SUBDIRS); do $(MAKE) $(MOOCOW_MFLAGS) -C $$d cvsclean || true; done;


##======================================================================
## Templates: subdir

## RULES = $(call subdir_template,$(dir_path),$(configure_args))
define subdir_template
 $(1)-autogen: $(1)-distclean
	(cd $(1); sh ./autogen.sh) || true

 $(1)-configure: $(1)-autogen
	(cd $(1); sh ./configure $(CONFIGURE_ARGS) $(2); make clean) || true

 $(1)/build.stamp: $(1)-configure
	$(MAKE) $(MOOCOW_MFLAGS) -C $(1) all install || true
	touch $$@

 $(1)-clean:
	$(MAKE) -C $(1) clean || true

 $(1)-distclean:
	$(MAKE) -C $(1) distclean || true
endef

##======================================================================
## Rules: subdirectories

##-- flite, gfsm: simulate failed builds
#$(eval $(call subdir_template,../flite,--with-flite-dir=/NOPE))
#$(eval $(call subdir_template,../gfsm,--disable-gfsm))

##-- flite, gfsm: build 'em if you got 'em
$(eval $(call subdir_template,../flite,))
$(eval $(call subdir_template,../gfsm,))

##-- the usual suspects
$(eval $(call subdir_template,../deque,))
$(eval $(call subdir_template,../pdstring,--enable-object-externals))
$(eval $(call subdir_template,../readdir,))
$(eval $(call subdir_template,../../sprinkler,))
$(eval $(call subdir_template,../weightmap,))
