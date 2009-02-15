# File: ./Makefile.am
# Package: pd-string
# Description:
#   + top-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## Options & Subdirectories

##-- aclocal options (top level only)
ACLOCAL_AMFLAGS = -I m4 

## --- automake options (top level only)
#AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip
AUTOMAKE_OPTIONS = foreign

## --- recursion subdirectories
SUBDIRS = config src

#-----------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------
include $(top_srcdir)/pdexternal.am

#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

all-local: $(PODS:.pod=.txt)

##-----------------------------------------------------------------------
## Variables: distribution

## --- extra distribution files
EXTRA_DIST += $(PODS:.pod=.txt)
