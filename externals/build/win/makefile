# ----------------------- NT -----------------------


EXTERNALS = equalizer.c mandelbrot~.c\
z_index.c z_skeleton.c\
z_lp.c mean.c\
z_drip.c z_tabread4.c\
sinh.c filesize.c z_swap.c\
sl.c z_limiter.c microtime.c\
aenv~.c ftos.c z_strings.c\
sprinkler.c z_matrix.c mixer~.c\
ann.c z_mtx.c z_testfun.c\
slider.c henon.c moog~.c\
streamin13~.c z_makesymbol.c z_zdelay.c\
arraysize.c hex2dec.c zhzxh~.c\
state.c z_point.c notch.c\
stripdir.c z_prime.c z_urn.c\
ascseq.c z_nop.c openpatch.c\
streamout13~.c highpass.c ossmixer.c\
ascwave.c z_msgfile.c piperead~.c\
streamin~.c z_noise.c pipewrite~.c\
atan2~.c highshelf.c prepend.c\
throw13~.c z_multiline.c proc.c\
ticker.c z_pack.c promiscous~.c\
svf~.c hlshelf.c qread.c\
bandpass.c z_multiplex.c randomix.c\
streamout~.c ikeda.c receive13.c\
susloop~.c z_pdf.c receive13~.c\
bangfilt.c z_random.c reson.c\
strippath.c inv.c rl.c\
toddle.c z_sigaverage.c rossler.c\
bangfilt2.c ixprint.c rtin~.c\
wavinfo.c z_quantize.c scramble~.c\
z_dfreq.c z_sfplay.c send13.c\
vbap.c kalashnikov.c send13~.c\
binshift.c z_sfrecord.c serial_bird.c\
unserialize.c lorenz.c serial_ms.c\
vst.c z_skeleton_tilde.c serialctl.c\
catch13~.c lowpass.c serialize.c\
z_connective.c z_sigbin.c sfread~.c\
cdplayer.c z_sigmatrix.c sfwrite13~.c\
z_average.c lowshelf.c sfwrite~.c\
z_down.c z_sigpack.c shell.c\
counter.c z_sort.c sigdelta.c\
z_datetime.c mandelbrot.c\
envgen.c z_sigzero.c\
z_coordinates.c z_stat.c
            

all: $(EXTERNALS:.c=.dll)

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo
VC="C:\Program Files\Microsoft Visual Studio\Vc98"

PDNTINCLUDE = /I. /I.. /I..\..\..\pd\src /I$(VC)\include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	..\..\..\..\pd\bin\pd.lib 

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$*_setup $(*).obj $(PDNTLIB)
	del $*.obj $*.lib $*.exp



clean:
	-del *.obj
	-del *.lib
	-del *.exp
	-del *.dll