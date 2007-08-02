## -*- Mode: Makefile -*-
##
## File: externals/moocow/extended/Makefile
## Author: Bryan Jurish <moocow@bbaw.de>
## Description: pd-extended makefile for moocow's externals
##

##======================================================================
## Variables

SUBDIRS = \
	../deque \
	../pdstring \
	../readdir \
	../weightmap \
	../../sprinkler

# ../flite ../gfsm

#CFLAGS ?= -march=k8

MOOCOW_DIR   ?=$(shell pwd)
MOOCOW_BUILD ?=$(MOOCOW_DIR)/build

CONFIGURE_ARGS=\
	CFLAGS="$(CFLAGS)" \
	--with-pd-include="$(pd_src)" \
	--with-pd-dir="$(MOOCOW_BUILD)" \
	--with-pd-extdir="$(MOOCOW_BUILD)/externs"

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
	for d in $(SUBDIRS); do $(MAKE) -C $$d clean || true; rm -f $$d/build.stamp; done;
	rm  -f build.stamp config.log
	rm -rf $(MOOCOW_BUILD)

realclean: clean
	for d in $(SUBDIRS); do $(MAKE) -C $$d realclean || true; done;

cvsclean: clean
	for d in $(SUBDIRS); do $(MAKE) -C $$d cvsclean || true; done;


##======================================================================
## Templates: subdir

## RULES = $(call subdir_template,$(dir_path),$(configure_args))
define subdir_template
 $(1)-autogen:
	(cd $(1); ./autogen.sh) || true

 $(1)-configure: $(1)-autogen
	(cd $(1); ./configure $(CONFIGURE_ARGS) $(2)) || true

 $(1)/build.stamp: $(1)-configure
	$(MAKE) -C $(1) all install || true
	touch $$@

 $(1)-clean:
	$(MAKE) -C $(1) clean || true
endef


##======================================================================
## Rules: subdirectories

$(eval $(call subdir_template,../deque))
#$(eval $(call subdir_template,../flite))
#$(eval $(call subdir_template,../gfsm))
$(eval $(call subdir_template,../pdstring,--enable-object-externals))
$(eval $(call subdir_template,../readdir))
$(eval $(call subdir_template,../../sprinkler))
$(eval $(call subdir_template,../weightmap))
