# change this!!!
CXX=g++-3.0
CFLAGS=-O6 -mcpu=pentium -g
#CFLAGS=-O6  -g -march=athlon
FLAGS=-DPD

FLEXTPATH=../flext

INCLUDES=/usr/lib/pd/include $(FLEXTPATH)
FLEXTLIB=$(FLEXTPATH)/pd-linux/flext.a
LIBS=m

PDEXTRA=/usr/lib/pd/extra

TARGDIR=./
TARGET=$(TARGDIR)/shabby~.pd_linux $(TARGDIR)/shaffy~.pd_linux 
SRCS=shabby.cpp shaffy.cpp

all: $(TARGDIR) $(TARGET)

$(TARGDIR):
	mkdir $(TARGDIR)

$(TARGDIR)/%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(FLAGS) $(patsubst %,-I%,$(INCLUDES)) $< -o $@

$(TARGET) : $(patsubst %.cpp,$(TARGDIR)/%.o,$(SRCS)) $(FLEXTLIB) 
	$(CXX) $(LDFLAGS) -shared $^ $(patsubst %,-l%,$(LIBS)) -o $@ 
	chmod 755 $@

install: $(TARGET)
	chown root.root $^ 
	cp $^ $(PDEXTRA)

.PHONY: clean
clean:
	rm -f $(TARGDIR)/*.o $(TARGET)
	







