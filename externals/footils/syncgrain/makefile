# change this!!!
CXX=g++ # -3.2
CFLAGS=-O6 -mcpu=pentium -g
FLAGS=-DPD

FLEXTPATH=../../grill/flext/source
SNDOBJINC=/usr/local/include


INCLUDES=/usr/lib/pd/include $(SNDOBJINC) $(FLEXTPATH)
FLEXTLIB=$(FLEXTPATH)/../pd-linux/flext.a

LIBS=m sndobj

PDEXTRA=/usr/lib/pd/extra

TARGDIR=./
TARGET=$(TARGDIR)/syncgrain~.pd_linux

# source code:
#SRCS=so_oscil.cpp MyPan.cpp Osc.cpp
#SRCS=so_oscil.cpp Osc.cpp Ident.cpp
#SRCS=so_oscil.cpp
SRCS=syncgrain.cpp

all: $(TARGDIR) $(TARGET)

$(TARGDIR):
	mkdir $(TARGDIR)

$(TARGDIR)/%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(FLAGS) $(patsubst %,-I%,$(INCLUDES)) $< -o $@

$(TARGET) : $(patsubst %.cpp,$(TARGDIR)/%.o,$(SRCS)) $(FLEXTLIB) 
	$(CXX) $(LDFLAGS) -shared $^ $(patsubst %,-l%,$(LIBS)) -o $@ 
	strip $@
	chmod 755 $@

install: $(TARGET)
	chown root.root $^ 
	cp $^ $(PDEXTRA)

.PHONY: clean
clean:
	rm -f $(TARGDIR)*.o $(TARGET)
	







