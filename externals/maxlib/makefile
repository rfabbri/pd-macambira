NAME=maxlib
CSYM=maxlib

current: pd_irix6

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

# define PD_0_36 to compilie with pd0.36 and below
PDNTCFLAGS = /W3 /WX /MD /O2 /G6 /DNT /DPD /DPD_0_36 /DMAXLIB /nologo
VC="C:\Programme\Microsoft Visual Studio\VC98"

PDNTINCLUDE = /I. /Ic:\pd\tcl\include /Ic:\pd\src /I$(VC)\include /Iinclude

PDNTLDIR = $(VC)\Lib
PDNTLIB = $(PDNTLDIR)\msvcrt.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\user32.lib \
	$(PDNTLDIR)\uuid.lib \
	$(PDNTLDIR)\ws2_32.lib \
	$(PDNTLDIR)\pthreadVC.lib \
	c:\pd\bin\pd.lib
        
PDNTEXTERNALS = borax.obj divide.obj ignore.obj match.obj pitch.obj speedlim.obj \
                minus.obj plus.obj multi.obj average.obj chord.obj score.obj \
                divmod.obj pulse.obj fifo.obj lifo.obj iso.obj dist.obj \
                step.obj netdist.obj beat.obj rhythm.obj history.obj netrec.obj \
                scale.obj delta.obj velocity.obj listfunnel.obj tilt.obj \
                gestalt.obj temperature.obj arbran.obj beta.obj bilex.obj \
                cauchy.obj expo.obj gauss.obj linear.obj poisson.obj triang.obj \
                weibull.obj netserver.obj netclient.obj nroute.obj remote.obj \
                edge.obj subst.obj pong.obj mlife.obj limit.obj unroute.obj \
                urn.obj split.obj wrap.obj rewrap.obj timebang.obj sync.obj \
		listfifo.obj arraycopy.obj allow.obj deny.obj nchange.obj

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\allow.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\arbran.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\arraycopy.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\average.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\beat.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\beta.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\bilex.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\borax.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\cauchy.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\chord.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\delta.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\deny.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\dist.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\divide.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\divmod.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\edge.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\expo.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\fifo.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\gauss.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\gestalt.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\history.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\ignore.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\iso.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\linear.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\listfifo.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\listfunnel.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\lifo.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\limit.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\match.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\minus.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\mlife.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\multi.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\nchange.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\netclient.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\netdist.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\netrec.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\netserver.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\nroute.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\pitch.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\plus.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\poisson.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\pong.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\pulse.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\remote.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\rewrap.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\rhythm.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\scale.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\score.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\speedlim.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\split.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\step.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\subst.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\sync.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\temperature.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\tilt.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\timebang.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\triang.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\unroute.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\urn.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\velocity.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\weibull.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\wrap.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$(CSYM)_setup $*.obj $(PDNTEXTERNALS) $(PDNTLIB)


# ----------------------- IRIX 6.5 -----------------------

pd_irix6: $(NAME).pd_irix6

.SUFFIXES: .pd_irix6

SGICFLAGS6 = -n32 -DPD -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-Ofast=ip32 -r12000 -shared

SGIINCLUDE =  -I../../src

SGIGCFLAGS = -mabi=n32 -DPD -DMAXLIB -DUNIX -DIRIX -DN32 -O3 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch -mips4

SGIEXTERNALS = borax.o ignore.o match.o pitch.o speedlim.o \
                 plus.o minus.o divide.o multi.o average.o chord.o \
                 score.o divmod.o pulse.o fifo.o lifo.o iso.o dist.o \
                 remote.o step.o netdist.o beat.o rhythm.o history.o \
                 netrec.o scale.o delta.o velocity.o mlife.o subst.o \
                 listfunnel.o tilt.o gestalt.o temperature.o arbran.o \
                 beta.o bilex.o cauchy.o expo.o gauss.o linear.o poisson.o \
                 triang.o weibull.o netserver.o netclient.o nroute.o \
                 edge.o pong.o limit.o unroute.o urn.o split.o wrap.o \
                 rewrap.o timebang.o sync.o listfifo.o arraycopy.o \
		 allow.o deny.o nchange.o

.c.pd_irix6:
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/allow.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/arbran.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/arraycopy.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/average.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/beat.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/beta.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/bilex.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/borax.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/cauchy.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/chord.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/delta.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/deny.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/dist.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/divide.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/divmod.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/edge.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/expo.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/fifo.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/gauss.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/gestalt.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/history.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/ignore.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/iso.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/lifo.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/limit.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/linear.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/listfifo.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/listfunnel.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/match.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/minus.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/mlife.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/multi.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/nchange.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/netclient.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/netdist.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/netrec.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/netserver.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/nroute.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/plus.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/pong.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/poisson.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/pulse.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/pitch.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/remote.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/rewrap.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/rhythm.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/scale.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/score.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/speedlim.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/split.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/step.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/subst.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/sync.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/temperature.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/tilt.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/timebang.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/triang.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/unroute.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/urn.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/velocity.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/weibull.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/wrap.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c $*.c
	ld -n32 -IPA -shared -rdata_shared -o $*.pd_irix6 $*.o $(SGIEXTERNALS)
	rm $*.o

# ----------------------- Mac OS X (Darwin) -----------------------


pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DMAXLIB -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../src -I../../obj

DARWINEXTERNALS = borax.o ignore.o match.o pitch.o speedlim.o \
                  plus.o minus.o divide.o multi.o average.o chord.o \
                  score.o divmod.o pulse.o fifo.o lifo.o iso.o dist.o \
                  remote.o step.o netdist.o beat.o rhythm.o history.o \
                  netrec.o scale.o delta.o velocity.o mlife.o subst.o \
                  listfunnel.o tilt.o gestalt.o temperature.o arbran.o \
                  beta.o bilex.o cauchy.o expo.o gauss.o linear.o poisson.o \
                  triang.o weibull.o netserver.o netclient.o nroute.o \
                  edge.o pong.o limit.o unroute.o urn.o split.o wrap.o \
                  rewrap.o timebang.o sync.o listfifo.o arraycopy.o \
		  allow.o deny.o nchange.o

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/allow.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/arbran.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/arraycopy.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/average.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/beat.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/beta.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/bilex.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/borax.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/cauchy.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/chord.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/delta.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/deny.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/dist.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/divide.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/divmod.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/edge.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/expo.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/fifo.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/gauss.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/gestalt.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/history.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/ignore.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/iso.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/lifo.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/limit.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/linear.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/listfifo.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/listfunnel.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/match.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/minus.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/mlife.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/multi.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/nchange.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/netclient.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/netdist.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/netrec.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/netserver.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/nroute.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/plus.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/poisson.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/pong.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/pulse.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/pitch.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/remote.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/rewrap.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/rhythm.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/scale.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/score.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/speedlim.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/weibull.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/split.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/step.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/subst.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/sync.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/temperature.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/tilt.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/timebang.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/triang.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/unroute.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/urn.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/velocity.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/weibull.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/wrap.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c $*.c 
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o $(DARWINEXTERNALS)
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DMAXLIB -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
LINUXINCLUDE =  -I/usr/local/include -I./include

LINUXEXTERNALS = borax.o ignore.o match.o pitch.o speedlim.o \
                 plus.o minus.o divide.o multi.o average.o chord.o \
                 score.o divmod.o pulse.o fifo.o lifo.o iso.o dist.o \
                 remote.o step.o netdist.o beat.o rhythm.o history.o \
                 netrec.o scale.o delta.o velocity.o mlife.o subst.o \
                 listfunnel.o tilt.o gestalt.o temperature.o arbran.o \
                 beta.o bilex.o cauchy.o expo.o gauss.o linear.o poisson.o \
                 triang.o weibull.o netserver.o netclient.o nroute.o \
                 edge.o pong.o limit.o unroute.o urn.o split.o wrap.o \
                 rewrap.o timebang.o sync.o listfifo.o arraycopy.o \
		 allow.o deny.o nchange.o

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/allow.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/arbran.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/arraycopy.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/average.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/beat.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/beta.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/bilex.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/borax.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/cauchy.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/chord.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/delta.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/deny.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/dist.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/divide.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/divmod.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/edge.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/expo.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/fifo.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/gauss.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/gestalt.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/history.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/ignore.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/iso.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/lifo.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/limit.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/linear.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/listfifo.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/listfunnel.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/match.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/minus.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/mlife.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/multi.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/nchange.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/netclient.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/netdist.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/netrec.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/netserver.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/nroute.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/plus.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/pong.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/poisson.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/pulse.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/pitch.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/remote.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/rewrap.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/rhythm.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/scale.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/score.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/speedlim.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/split.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/step.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/subst.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/sync.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/temperature.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/tilt.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/timebang.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/triang.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/unroute.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/urn.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/velocity.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/weibull.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/wrap.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) -lc
	strip --strip-unneeded $*.pd_linux

# ----------------------------------------------------------

PDDIR=/usr/local/lib/pd

install:
	install -d $(PDDIR)/doc/5.reference/maxlib
	cp help/help-*.pd $(PDDIR)/doc/5.reference/maxlib

clean:
	rm -f *.o *.pd_* so_locations
