VPATH = ../obj:./
OBJ_DIR = ../obj
BIN_DIR = ../bin
PDEXEC = $(BIN_DIR)/pd
EXT= pd_linux
GUINAME= pd-gui

INSTALL_PREFIX = /usr/local
GFLAGS = -DINSTALL_PREFIX=\"$(INSTALL_PREFIX)\"

# there should be a better way to do this...
prefix = /usr/local
MANDIR = ${prefix}/man

# ALSA compilation

SOUND_ALSA = 

DEFINES = 
MORECFLAGS = -DDL_OPEN  -DUSEAPI_OSS

INCLUDE = -I.
GINCLUDE = $(INCLUDE)  -I/usr/local/include/tcl8.4 -I/usr/local/include/tk8.4 -I/usr/X11R6/include -I/usr/local/include
GLIB =  -lX11 -L/usr/X11R6/lib -L/usr/local/lib -ltcl84 -ltk84

LDFLAGS = -Wl,-export-dynamic -L/usr/local/lib/pth -lpthread
LIB =   -lm

#select either the DBG and OPT compiler flags below:
OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
WARN_CFLAGS = -Wall -W -Wstrict-prototypes  -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch
ARCH_CFLAGS = -DPD -DUNIX 

CFLAGS = $(ARCH_CFLAGS) $(WARN_CFLAGS) $(OPT_CFLAGS) $(DEFINES) $(MORECFLAGS)

# you might want ALSA linked in non-shared because
# many Linux machines don't have the ALSA shared library.  To link
# ALSA non-shared, move the # sign below.

ifeq (${SOUND_ALSA},yes)
CFLAGS += -DPA_USE_ALSA -DUSEAPI_ALSA
SYSSRC += s_audio_alsa.c \
    ../portaudio/pa_linux_alsa/callback_thread.c \
    ../portaudio/pa_linux_alsa/pa_linux_alsa.c \
    ../portaudio/pa_linux_alsa/blocking_calls.c
endif

# Which system

SYSTEM = $(shell uname -m)

ifeq (${SYSTEM},alpha)
#LIB += -lffm -lm
CFLAGS += -mieee -mcpu=ev56
endif

# Which compiler

ifeq (${CC},ccc)
CFLAGS += -g3 -D__COMPAQC__ -arch host
endif

# the sources

SYSSRC += s_midi_oss.c s_audio_oss.c 

SRC = g_canvas.c g_graph.c g_text.c g_rtext.c g_array.c g_template.c g_io.c \
    g_scalar.c g_traversal.c g_guiconnect.c g_readwrite.c g_editor.c \
    g_all_guis.c g_bang.c g_hdial.c g_hslider.c g_mycanvas.c g_numbox.c \
    g_toggle.c g_vdial.c g_vslider.c g_vumeter.c \
    m_pd.c m_class.c m_obj.c m_atom.c m_memory.c m_binbuf.c \
    m_conf.c m_glob.c m_sched.c \
    s_main.c s_inter.c s_file.c s_print.c \
    s_loader.c s_path.c s_entry.c s_audio.c s_midi.c \
    d_ugen.c d_ctl.c d_arithmetic.c d_osc.c d_filter.c d_dac.c d_misc.c \
    d_math.c d_fft.c d_mayer_fft.c d_fftroutine.c d_array.c d_global.c \
    d_delay.c d_resample.c \
    x_arithmetic.c x_connective.c x_interface.c x_midi.c x_misc.c \
    x_time.c x_acoustics.c x_net.c x_qlist.c x_gui.c d_soundfile.c \
    $(SYSSRC)


OBJ = $(SRC:.c=.o) 
EXTERNS = ../extra/*/*.$(EXT)

GSRC =  t_main.c t_tkcmd.c

GOBJ = $(GSRC:.c=.o)

#
#  ------------------ targets ------------------------------------
#

.PHONY: pd gui externs all

all: $(PDEXEC) $(BIN_DIR)/pd-watchdog $(BIN_DIR)/$(GUINAME) $(BIN_DIR)/pdsend \
    $(BIN_DIR)/pdreceive  $(BIN_DIR)/pd.tk externs

bin: $(PDEXEC) $(BIN_DIR)/pd-watchdog $(BIN_DIR)/$(GUINAME) $(BIN_DIR)/pdsend \
    $(BIN_DIR)/pdreceive  $(BIN_DIR)/pd.tk

$(OBJ) : %.o : %.c
	$(CC) $(CFLAGS) $(GFLAGS) $(INCLUDE) -c -o $(OBJ_DIR)/$*.o $*.c 

$(GOBJ) : %.o : %.c
	$(CC) $(CFLAGS) $(GFLAGS) $(GINCLUDE) -c -o $(OBJ_DIR)/$*.o $*.c 

pd: $(PDEXEC)

gui: $(BIN_DIR)/$(GUINAME)

pd-watchdog: $(BIN_DIR)/pd-watchdog

$(BIN_DIR)/pd-watchdog: s_watchdog.c
	cc -O2 $(STRIPFLAG) -o $(BIN_DIR)/pd-watchdog s_watchdog.c

$(BIN_DIR)/pdsend: u_pdsend.c
	cc $(CFLAGS)  $(STRIPFLAG) -o $(BIN_DIR)/pdsend u_pdsend.c

$(BIN_DIR)/pdreceive: u_pdreceive.c
	cc $(CFLAGS)  $(STRIPFLAG) -o $(BIN_DIR)/pdreceive u_pdreceive.c

$(PDEXEC): $(OBJ) 
	cd ../obj;  $(CC) $(LDFLAGS) $(DBG_CFLAGS) -o $(PDEXEC) $(OBJ) \
	$(LIB)

$(BIN_DIR)/pd-gui: $(GOBJ) $(GSRC)
	cd ../obj; $(CC) $(INCLUDE) -o $(BIN_DIR)/$(GUINAME) $(GOBJ) \
	$(GLIB)

$(BIN_DIR)/pd.tk: u_main.tk
	echo set pd_nt 0 > $(BIN_DIR)/pd.tk
	grep -v "set pd_nt" < u_main.tk >> $(BIN_DIR)/pd.tk

#this is for Max OSX only...
$(BIN_DIR)/pdtcl: $(GOBJ) $(GSRC)
	cd ../obj; libtool -dynamic -o $(BIN_DIR)/pdtcl $(GOBJ) \
	/Library/Frameworks/Tk.framework/Versions/Current/Tk \
	/Library/Frameworks/Tcl.framework/Versions/Current/Tcl \
	/usr/lib/libSystem.B.dylib

externs: 
	cd ../extra/bonk~;make 
	cd ../extra/choice;make 
	cd ../extra/expr~;make 
	cd ../extra/fiddle~;make 
	cd ../extra/loop~;make 
	cd ../extra/lrshift~;make 
	cd ../extra/pique;make 

INSTDIR = $(DESTDIR)/$(INSTALL_PREFIX)
MANINSTDIR = $(DESTDIR)/$(MANDIR)

install:  all
	install -d $(INSTDIR)/lib/pd/bin
	install $(BIN_DIR)/$(GUINAME) $(INSTDIR)/lib/pd/bin/$(GUINAME)
	install $(BIN_DIR)/pd-watchdog $(INSTDIR)/lib/pd/bin/pd-watchdog
	install -m644 $(BIN_DIR)/pd.tk $(INSTDIR)/lib/pd/bin/pd.tk
	install -d $(INSTDIR)/bin
	install -m755 $(PDEXEC) $(INSTDIR)/bin/pd
	install -m 755 $(BIN_DIR)/pdsend $(INSTDIR)/bin/pdsend
	install -m 755 $(BIN_DIR)/pdreceive $(INSTDIR)/bin/pdreceive 
	install -d $(INSTDIR)/lib/pd/extra
	install -d $(INSTDIR)/lib/pd/externs
	install -m 644 $(EXTERNS) $(INSTDIR)/lib/pd/extra
	cp -r ../doc $(INSTDIR)/lib/pd/
	install -d $(INSTDIR)/include
	install -m644 m_pd.h $(INSTDIR)/include/m_pd.h
	install -d $(MANINSTDIR)/man1
	gzip < ../man/pd.1 >  $(MANINSTDIR)/man1/pd.1.gz
	chmod 644 $(MANINSTDIR)/man1/pd.1.gz
	gzip < ../man/pdsend.1 >  $(MANINSTDIR)/man1/pdsend.1.gz
	chmod 644 $(MANINSTDIR)/man1/pdsend.1.gz
	gzip < ../man/pdreceive.1 >  $(MANINSTDIR)/man1/pdreceive.1.gz
	chmod 644 $(MANINSTDIR)/man1/pdreceive.1.gz

local-clean:
	-rm -f ../obj/* $(BIN_DIR)/pd $(BIN_DIR)/$(GUINAME) $(BIN_DIR)/pdsend \
	    $(BIN_DIR)/pdreceive $(BIN_DIR)/pd-watchdog m_stamp.c
	-rm -f *~
	-rm -f $(BIN_DIR)/pdsend $(BIN_DIR)/pdreceive
	-(cd ../doc/6.externs; rm -f *.pd_linux)
	-rm -f makefile.dependencies
	touch makefile.dependencies
	chmod 666 makefile.dependencies

extra-clean:
	-rm -f `find ../extra/ -name "*.pd_*"`
	-rm -f tags

clean: extra-clean local-clean

distclean: clean
	-rm config.cache config.log config.status makefile tags
	echo all: > makefile
	echo -e  "\t./configure" >> makefile
	echo -e "\tmake" >> makefile

tags: $(SRC) $(GSRC); ctags *.[ch]

depend: 
	$(CC) $(INCLUDE) $(CFLAGS) -M $(SRC) > makefile.dependencies

uninstall:
	-rm -r $(INSTDIR)/lib/pd
	-rm $(INSTDIR)/bin/pd
	-rm $(INSTDIR)/bin/pdsend
	-rm $(INSTDIR)/bin/pdreceive
	-rm $(INSTDIR)/include/m_pd.h
	-rm $(MANINSTDIR)/man/man1/pd.1.gz
	-rm $(MANINSTDIR)/man/man1/pdsend.1.gz
	-rm $(MANINSTDIR)/man/man1/pdreceive.1.gz

include makefile.dependencies







