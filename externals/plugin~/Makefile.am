## Process this file with automake to produce Makefile.in. -*-Makefile-*-

SUFFIXES = .pd_linux

pddir = @PDDIR@

SUBDIRS = ladspa vst win

## Make and install the shared library.

pdinstalldir = $(pddir)/extra/plugin~

# Automake won't accept something ending in ".pd_linux" as a library
pdinstall_PROGRAMS = plugin~.pd_linux

plugin__pd_linux_SOURCES = \
	config.h \
	jload.c \
	jsearch.c \
	jutils.h \
	plugin~.c \
	plugin~.h \
	plugin~_ladspa.c \
	plugin~_ladspa.h \
	plugin~_vst.c \
	plugin~_vst.h \
	vstutils.c \
	vstutils.h

pdincludedir = $(pddir)/src
INCLUDES = -I$(top_srcdir) -I$(pdincludedir)
CFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch
LDFLAGS = -export_dynamic -shared

## Install the documentation.

pdinstall_DATA = \
	AUTHORS \
	BUGS \
	COPYING \
	NEWS \
	README \
	TODO \
	plugin~.pd \
	plugin~.txt

## My kludge
noinst_SCRIPTS = libtoolkludge

## version.h is there for the Windows platform (no autoconf)
EXTRA_DIST = $(pdinstall_DATA) $(noinst_SCRIPTS) version.h
