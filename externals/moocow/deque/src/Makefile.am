# File: ./src/Makefile.am
# Package: deque
# Description:
#   + src-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------
include $(top_srcdir)/pdexternal.am

#-----------------------------------------------------------------------
# pd externals (hacked _PROGRAMS target)
#-----------------------------------------------------------------------

##--- externals
pdexterns_PROGRAMS = deque

## --- possible externals
EXTRA_PROGRAMS = deque

## --- patches
pdexterns_DATA =

## --- documentation
pddoc_DATA = deque-help.pd

#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

deque_SOURCES = \
	mooPdUtils.h \
	squeue.c squeue.h \
	dsqueue.c dsqueue.h \
	deque.c
