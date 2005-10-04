#!/usr/bin/make

# JMAX = /opt/lib/jmax
# JMAX = /usr/local/lib/jmax

include ../config.make
include ../bundled/jmax/Makefiles/Makefile.$(JMAX_ARCH)
JMAX = $(prefix)/lib/jmax
CLASSPATH=".:$(JMAX)/java/classes/jmax.jar:$(JMAX)/packages/guiobj/java/classes/guiobj.jar:$(JMAX)/java/lib/jacl/jacl.jar:$(JMAX)/java/lib/jacl/tcljava.jar"

OBJS = \
gridflow/FtsPeepholeObject.class \
gridflow/Peephole.class \
gridflow/PeepholeControlPanel.class \
gridflow/GridFlowExtension.class \
gridflow/PeepholeControlPanel\$1.class \
gridflow/PathInfo.class

SRC = \
FtsPeepholeObject.java \
Peephole.java \
PeepholeControlPanel.java \
GridFlowExtension.java \
PathInfo.java

# libPeephole.so

JC = javac -classpath $(CLASSPATH) -O

all: gridflow.jar

clean::
	rm -f $(OBJS) gridflow.jar PathInfo.java

gridflow.jar: $(SRC)
	mkdir gridflow || true
	$(JC) -d . $(SRC)
	/bin/rm -f gridflow.jar
	jar -cvf gridflow.jar gridflow/*.class
