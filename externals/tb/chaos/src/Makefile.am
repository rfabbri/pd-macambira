NAME = chaos

BUILT_SOURCES = main.cpp ode_base.cpp chaos_base.cpp chaos_dsp.cpp \
	bernoulli_dsp.cpp bernoulli_msg.cpp \
	bungalow_tent_dsp.cpp bungalow_tent_msg.cpp \
	circle_map_dsp.cpp circle_map_msg.cpp \
	coupled_logistic_dsp.cpp coupled_logistic_msg.cpp \
	chua_dsp.cpp chua_msg.cpp \
	driven_anharmonic_dsp.cpp driven_anharmonic_msg.cpp \
	driven_van_der_pol_dsp.cpp driven_van_der_pol_msg.cpp \
	duffing_map_dsp.cpp duffing_map_msg.cpp \
	gauss_map_dsp.cpp gauss_map_msg.cpp \
	henon_map_dsp.cpp henon_map_msg.cpp \
	ikeda_laser_map_dsp.cpp ikeda_laser_map_msg.cpp \
	latoocarfian_dsp.cpp latoocarfian_msg.cpp \
	latoomutalpha_dsp.cpp latoomutalpha_msg.cpp \
	latoomutbeta_dsp.cpp latoomutbeta_msg.cpp \
	latoomutgamma_dsp.cpp latoomutgamma_msg.cpp \
	logistic_dsp.cpp logistic_msg.cpp \
	lorenz_dsp.cpp lorenz_msg.cpp \
	lozi_map_dsp.cpp lozi_map_msg.cpp \
	roessler_dsp.cpp roessler_msg.cpp \
	sine_map_dsp.cpp sine_map_msg.cpp \
	standard_map_dsp.cpp standard_map_msg.cpp \
	tent_map_dsp.cpp tent_map_msg.cpp

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
	rm ./Makefile

clean-local:
	rm -f ../$(TARGET)
	rm -f ./*.@OBJEXT@

install-exec-local:
	install ../$(TARGET) $(SYSDIR)extra