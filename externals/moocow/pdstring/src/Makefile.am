# File: ./src/Makefile.am
# Package: pdstring
# Description:
#   + src-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

##--- common includes
include $(top_srcdir)/pdexternal.am

##--- recursion subdirectories
#SUBDIRS = 

#-----------------------------------------------------------------------
# sources (using PROGRAMS)
#-----------------------------------------------------------------------

## --- externals (as _PROGRAMS)
pdexec_PROGRAMS = pdstring
if WANT_OBJECT_EXTERNALS
 pdexec_PROGRAMS += \
	any2bytes \
	bytes2any \
	bytes2wchars \
	wchars2bytes \
	printbytes \
	printwchars
endif

## --- possible single-object externals (as _PROGRAMS)
EXTRA_PROGRAMS = \
	pdstring \
	any2bytes \
	bytes2any \
	bytes2wchars \
	wchars2bytes \
	printbytes \
	printwchars

any2bytes_SOURCES = any2bytes.c mooPdUtils.h pdstringUtils.h pdstringUtils.c
bytes2any_SOURCES = bytes2any.c mooPdUtils.h pdstringUtils.h pdstringUtils.c
bytes2wchars_SOURCES = bytes2wchars.c mooPdUtils.h pdstringUtils.h pdstringUtils.c
wchars2bytes_SOURCES = wchars2bytes.c mooPdUtils.h pdstringUtils.h pdstringUtils.c
printbytes_SOURCES = printbytes.c mooPdUtils.h pdstringUtils.h pdstringUtils.c
printwchars_SOURCES = printwchars.c mooPdUtils.h pdstringUtils.h pdstringUtils.c
pdstring_SOURCES = pdstring.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

#-----------------------------------------------------------------------
# patches, etc.
#-----------------------------------------------------------------------

## --- patches
pdexterns_DATA = \
	any2wchars.pd \
	wchars2any.pd \
	any2string.pd \
	string2any.pd

## --- documentation
pddoc_DATA = \
	pdstring-help.pd \
	any2string-help.pd \
	string2any-help.pd \
	any2bytes-help.pd \
	bytes2any-help.pd \
	bytes2wchars-help.pd \
	wchars2bytes-help.pd \
	any2wchars-help.pd \
	wchars2any-help.pd \
	printbytes-help.pd \
	printwchars-help.pd


#-----------------------------------------------------------------------
# aliases via install hook
#-----------------------------------------------------------------------

if WANT_OBJECT_EXTERNALS
all-local:
	rm -f any2string.$(PDEXT)
	rm -f string2any.$(PDEXT)
endif

install-data-hook:
	rm -f $(pdexternsdir)/any2string.$(PDEXT)
	rm -f $(pdexternsdir)/string2any.$(PDEXT)

uninstall-local:
	rm -f $(pdexternsdir)/any2string.$(PDEXT)
	rm -f $(pdexternsdir)/string2any.$(PDEXT)

#-----------------------------------------------------------------------
# external compilation : flags
#  + now handled by AX_PD_EXTERNAL
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Variables: cleanup
#  + should be handled by pdexternal.am

#-----------------------------------------------------------------------
# Variables: distribution
#  + should be handled by pdexternal.am

#-----------------------------------------------------------------------
# Rules: cleanup
#  + should be handled by pdexternal.am

