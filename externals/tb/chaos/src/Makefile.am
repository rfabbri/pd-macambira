NAME = chaos

BUILT_SOURCES = main.cpp ode_base.cpp chaos_base.cpp chaos_dsp.cpp \
	lorenz_dsp.cpp logistic_dsp.cpp


CXXFLAGS  = @CXXFLAGS@ \
	@OPT_FLAGS@ \
	@INCLUDEDIR@ \
	$(DEFS)

LDFLAGS = @LDFLAGS@ \
	$(patsubst %,-framework %,$(FRAMEWORKS))

LIBS = @LIBS@

FRAMEWORKS = @FRAMEWORKS@

TARGET =$(NAME).@EXTENSION@

OBJECTS = $(patsubst %.cpp,%.@OBJEXT@,$(BUILT_SOURCES))

SYSDIR = @SYSDIR@


# ----------------------------- targets --------------------------------


all-local: $(OBJECTS)
	$(CXX) $(LDFLAGS) -shared $^ $(LIBS) -o ../$(TARGET)
	strip --strip-unneeded ../$(TARGET)

%.@OBJEXT@ : %.cpp 
	$(CXX) -c $(CXXFLAGS) $< -o $@

dist-hook:
	rm -f ./*~
	rm -f ./*.@OBJEXT@

clean-local:
	rm -f ../$(TARGET)
	rm -f ./*.@OBJEXT@

install-exec-local:
	install ../$(TARGET) $(SYSDIR)extra