## -*- Mode: Makefile -*-
##
## File: externals/moocow/extended/Makefile
## Author: Bryan Jurish <moocow@bbaw.de>
## Description: pd-extended makefile for moocow's externals
##

##======================================================================
## Variables

SUBDIRS = \
	flite \
	gfsm \
	deque \
	readdir \
	weightmap \
	pdstring \
	sprinkler

##-- 'pdstring' and 'sprinkler' are already in 'flatspace' ... should they be moved here?
##    -> 2008-08: removed any2string,string2any,pdstring from flatspace

##-- local variables
MOOCOW_DIR    ?=$(shell pwd)
MOOCOW_BUILD  ?=$(MOOCOW_DIR)/build.moo
MOOCOW_MFLAGS ?=DESTDIR=""

CONFIGURE_ARGS=\
	CFLAGS="$(CFLAGS)" \
	--with-pd-include="$(pd_src)/src" \
	--with-pd-dir="$(MOOCOW_BUILD)" \
	--disable-dependency-tracking

#	--with-pd-extdir="$(MOOCOW_BUILD)/externs"

##-- defaults
CFLAGS ?= -g -O2
pd_src ?= $(CURDIR)/../../../pd


##======================================================================
## Rules: default
all: build.stamp

##======================================================================
## Rules: subdirectory: dummy

DUMMY_DIR            ?=../dummy
DUMMY_CONFIGURE_ARGS ?=--disable-shared
dummy.autogen_stamp: 
	(cd $(DUMMY_DIR); sh ./autogen.sh) || true
	touch $@

dummy.configure_stamp: dummy.autogen_stamp
	(cd $(DUMMY_DIR); sh ./configure $(CONFIGURE_ARGS) $(DUMMY_CONFIGURE_ARGS); make clean) || true
	touch $@

dummy.build_stamp: dummy.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(DUMMY_DIR) all install || true
	touch $@

dummy.extclean:
	rm -f dummy.autogen_stamp dummy.configure_stamp dummy.build_stamp

dummy.clean: dummy.extclean
	$(MAKE) -C $(DUMMY_DIR) clean || true

dummy.distclean: dummy.extclean
	$(MAKE) -C $(DUMMY_DIR) distclean || true

dummy.cvsclean: dummy.extclean
	$(MAKE) -C $(DUMMY_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: flite

FLITE_DIR            ?=../flite
#FLITE_CONFIGURE_ARGS ?=--with-flite-dir=/NOPE
#FLITE_CONFIGURE_ARGS ?=

flite.autogen_stamp: 
	(cd $(FLITE_DIR); sh ./autogen.sh) || true
	touch $@

flite.configure_stamp: flite.autogen_stamp
	(cd $(FLITE_DIR); sh ./configure $(CONFIGURE_ARGS) $(FLITE_CONFIGURE_ARGS); make clean) || true
	touch $@

flite.build_stamp: flite.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(FLITE_DIR) all install || true
	touch $@

flite.extclean:
	rm -f flite.autogen_stamp flite.configure_stamp flite.build_stamp

flite.clean: flite.extclean
	$(MAKE) -C $(FLITE_DIR) clean || true

flite.distclean: flite.extclean
	$(MAKE) -C $(FLITE_DIR) distclean || true

flite.cvsclean: flite.extclean
	$(MAKE) -C $(FLITE_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: gfsm

GFSM_DIR            ?=../gfsm
#GFSM_CONFIGURE_ARGS ?=--disable-gfsm
#GFSM_CONFIGURE_ARGS ?=

gfsm.autogen_stamp: 
	(cd $(GFSM_DIR); sh ./autogen.sh) || true
	touch $@

gfsm.configure_stamp: gfsm.autogen_stamp
	(cd $(GFSM_DIR); sh ./configure $(CONFIGURE_ARGS) $(GFSM_CONFIGURE_ARGS); make clean) || true
	touch $@

gfsm.build_stamp: gfsm.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(GFSM_DIR) all install || true
	touch $@

gfsm.extclean:
	rm -f gfsm.autogen_stamp gfsm.configure_stamp gfsm.build_stamp

gfsm.clean: gfsm.extclean
	$(MAKE) -C $(GFSM_DIR) clean || true

gfsm.distclean: gfsm.extclean
	$(MAKE) -C $(GFSM_DIR) distclean || true

gfsm.cvsclean: gfsm.extclean
	$(MAKE) -C $(GFSM_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: deque

DEQUE_DIR            ?=../deque
DEQUE_CONFIGURE_ARGS ?=
deque.autogen_stamp: 
	(cd $(DEQUE_DIR); sh ./autogen.sh) || true
	touch $@

deque.configure_stamp: deque.autogen_stamp
	(cd $(DEQUE_DIR); sh ./configure $(CONFIGURE_ARGS) $(DEQUE_CONFIGURE_ARGS); make clean) || true
	touch $@

deque.build_stamp: deque.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(DEQUE_DIR) all install || true
	touch $@

deque.extclean:
	rm -f deque.autogen_stamp deque.configure_stamp deque.build_stamp

deque.clean: deque.extclean
	$(MAKE) -C $(DEQUE_DIR) clean || true

deque.distclean: deque.extclean
	$(MAKE) -C $(DEQUE_DIR) distclean || true

deque.cvsclean: deque.extclean
	$(MAKE) -C $(DEQUE_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: pdstring

PDSTRING_DIR            ?=../pdstring
PDSTRING_CONFIGURE_ARGS ?=--enable-object-externals
pdstring.autogen_stamp: 
	(cd $(PDSTRING_DIR); sh ./autogen.sh) || true
	touch $@

pdstring.configure_stamp: pdstring.autogen_stamp
	(cd $(PDSTRING_DIR); sh ./configure $(CONFIGURE_ARGS) $(PDSTRING_CONFIGURE_ARGS); make clean) || true
	touch $@

pdstring.build_stamp: pdstring.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(PDSTRING_DIR) all install || true
	touch $@

pdstring.extclean:
	rm -f pdstring.autogen_stamp pdstring.configure_stamp pdstring.build_stamp

pdstring.clean: pdstring.extclean
	$(MAKE) -C $(PDSTRING_DIR) clean || true

pdstring.distclean: pdstring.extclean
	$(MAKE) -C $(PDSTRING_DIR) distclean || true

pdstring.cvsclean: pdstring.extclean
	$(MAKE) -C $(PDSTRING_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: readdir

READDIR_DIR            ?=../readdir
READDIR_CONFIGURE_ARGS ?=
readdir.autogen_stamp: 
	(cd $(READDIR_DIR); sh ./autogen.sh) || true
	touch $@

readdir.configure_stamp: readdir.autogen_stamp
	(cd $(READDIR_DIR); sh ./configure $(CONFIGURE_ARGS) $(READDIR_CONFIGURE_ARGS); make clean) || true
	touch $@

readdir.build_stamp: readdir.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(READDIR_DIR) all install || true
	touch $@

readdir.extclean:
	rm -f readdir.autogen_stamp readdir.configure_stamp readdir.build_stamp

readdir.clean: readdir.extclean
	$(MAKE) -C $(READDIR_DIR) clean || true

readdir.distclean: readdir.extclean
	$(MAKE) -C $(READDIR_DIR) distclean || true

readdir.cvsclean: readdir.extclean
	$(MAKE) -C $(READDIR_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: sprinkler
##  + one directory higher in the tree than usual...

SPRINKLER_DIR            ?=../../sprinkler
SPRINKLER_CONFIGURE_ARGS ?=
sprinkler.autogen_stamp: 
	(cd $(SPRINKLER_DIR); sh ./autogen.sh) || true
	touch $@

sprinkler.configure_stamp: sprinkler.autogen_stamp
	(cd $(SPRINKLER_DIR); sh ./configure $(CONFIGURE_ARGS) $(SPRINKLER_CONFIGURE_ARGS); make clean) || true
	touch $@

sprinkler.build_stamp: sprinkler.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(SPRINKLER_DIR) all install || true
	touch $@

sprinkler.extclean:
	rm -f sprinkler.autogen_stamp sprinkler.configure_stamp sprinkler.build_stamp

sprinkler.clean: sprinkler.extclean
	$(MAKE) -C $(SPRINKLER_DIR) clean || true

sprinkler.distclean: sprinkler.extclean
	$(MAKE) -C $(SPRINKLER_DIR) distclean || true

sprinkler.cvsclean: sprinkler.extclean
	$(MAKE) -C $(SPRINKLER_DIR) cvsclean || true

##======================================================================
## Rules: subdirectory: weightmap

WEIGHTMAP_DIR            ?=../weightmap
WEIGHTMAP_CONFIGURE_ARGS ?=
weightmap.autogen_stamp: 
	(cd $(WEIGHTMAP_DIR); sh ./autogen.sh) || true
	touch $@

weightmap.configure_stamp: weightmap.autogen_stamp
	(cd $(WEIGHTMAP_DIR); sh ./configure $(CONFIGURE_ARGS) $(WEIGHTMAP_CONFIGURE_ARGS); make clean) || true
	touch $@

weightmap.build_stamp: weightmap.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(WEIGHTMAP_DIR) all install || true
	touch $@

weightmap.extclean:
	rm -f weightmap.autogen_stamp weightmap.configure_stamp weightmap.build_stamp

weightmap.clean: weightmap.extclean
	$(MAKE) -C $(WEIGHTMAP_DIR) clean || true

weightmap.distclean: weightmap.extclean
	$(MAKE) -C $(WEIGHTMAP_DIR) distclean || true

weightmap.cvsclean: weightmap.extclean
	$(MAKE) -C $(WEIGHTMAP_DIR) cvsclean || true


##======================================================================
## Rules: local
autogen: $(SUBDIRS:=.autogen_stamp)
configure: $(SUBDIRS:=.configure_stamp)

build.stamp: $(SUBDIRS:=.build_stamp)
	touch $@

readme: README.txt
README.txt: README.pod
	pod2text README.pod $@

extclean: $(SUBDIRS:=.extclean)

clean: $(SUBDIRS:=.clean)
	rm  -f build.stamp config.log
	rm -rf $(MOOCOW_BUILD)

realclean: distclean

distclean: $(SUBDIRS:=.distclean) clean

cvsclean: $(SUBDIRS:=.cvsclean) clean
