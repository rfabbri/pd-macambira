# Makefile for icc @ linux adapted from Thomas Grill's xsample makefile
#
# usage:
# to build run "make"
# to install (as root), do "make install"
#




# your c++ compiler (if not g++)
 CXX=icc

# where does the PD installation reside?
PD=/usr/lib/pdsrc/pd

# where are the PD header files?
# leave it blank if it is a system directory (like /usr/local/include), 
# 	since gcc 3.2 complains about it
PDINC=

# where do the flext libraries reside?
FLEXTPATH=/usr/lib/flext

# where should the objects be built?
TARGDIR=./

# where should tbext be installed?
# (leave blank to omit installation)
INSTDIR=${PD}/externs

# where should the tbext help be installed?
# (leave blank to omit installation)
HELPDIR=${PD}/doc/5.reference

# additional compiler flags
# (check whether they fit to your system!)
# UFLAGS=-mcpu=pentiumpro  # gcc 2.95
# UFLAGS=-mcpu=pentium4 -mmmx -msse2 -msse -mfpmath=sse  # gcc 3.2 
UFLAGS= -O3 -xW -tpp7 -ip -ipo #icc


FLEXTLIB=$(FLEXTPATH)/flext.a

# compiler stuff
INCLUDES=$(PDINC) 
FLAGS=-DFLEXT_SYS=2 -D__GNUG__
CFLAGS=${UFLAGS} -O6 -funroll-loops -fmove-all-movables -frerun-loop-opt -finline-functions -fno-rtti -fno-exceptions
LDFLAGS=$(CFLAGS)
LIBS=m

# ----------------------------------------------
# the rest can stay untouched
# ----------------------------------------------

NAME=tbext
SRCDIR=source

include make-files.txt

MAKEFILE=makefile
TARGET=$(TARGDIR)/$(NAME).pd_linux

# default target
all: $(TARGDIR) $(TARGET) 

$(patsubst %,$(SRCDIR)/%,$(SRCS)): $(patsubst %,$(SRCDIR)/%,$(HDRS)) $(MAKEFILE) $(CONFIG)
	touch $(patsubst %,$(SRCDIR)/%,$(SRCS))

$(TARGDIR):
	-mkdir $(TARGDIR)

$(TARGDIR)/%.o : $(SRCDIR)/%.cpp 
	$(CXX) -c $(CFLAGS) $(FLAGS) $(patsubst %,-I%,$(INCLUDES) $(FLEXTPATH)) $< -o $@

$(TARGET) : $(patsubst %.cpp,$(TARGDIR)/%.o,$(SRCS)) $(FLEXTLIB) 
	$(CXX) $(LDFLAGS) -shared $^ $(patsubst %,-l%,$(LIBS)) -o $@ 
	strip --strip-unneeded $@
	chmod 755 $@

$(INSTDIR):
	-mkdir $(INSTDIR)

install:: $(INSTDIR)

install:: $(TARGET) 
	cp $^ $(INSTDIR)
	chown root.root $(patsubst %,$(INSTDIR)/%,$(notdir $^))

$(HELPDIR):
	-mkdir $(HELPDIR)

install-help:: $(HELPDIR)

install-help:: ./pd
	chmod 644 $^/*.*
	cp $^/*.* $(HELPDIR)


.PHONY: clean
clean:
	rm -f $(TARGDIR)/*.o $(TARGDIR)/*.il $(TARGET)












