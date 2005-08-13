dnl AC_INIT(src/main.cpp)
AC_INIT([readanysf~], [0.15],
        [august black <august - alien mur at>],
        [readanysf~])   dnl last part is the tarname



dnl ----------------------------------
dnl ---- have your makefiles call recursively 
dnl ---- makefiles at subdirectories
dnl ----------------------------------
AC_PROG_MAKE_SET

dnl ---- If you have any makefiles in subdirectories
dnl ---- you must also put them in the AC_CONFIG_FILES statement
dnl ---- we do not do this, it causes autoconf errors ????
dnl ----------------------------------
dnl AC_CONFIG_FILES([  Makefile src/Makefile ])

AM_SANITY_CHECK
AM_INIT_AUTOMAKE(readanysf, 0.15)

AM_CONFIG_HEADER(config.h)
dnl AM_MAINTAINER_MODE



AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
dnl AC_PROG_AWK
dnl AC_PROG_LN_S
dnl AC_PROG_RANLIB

echo
echo "$CPPFLAGS"
echo "$CXXFLAGS"
echo "$CFLAGS"
echo "$LDFLAGS"
echo

have_vorbis=no
have_mad=no
have_flc=no

flext_sys=2
flext_dir=/usr/local/lib/pd/flext
flext_shared=yes





dnl ------------------------------------------
dnl ----  do some magic to gues the host opsys
dnl ----  taken from libvorbis configure.in
dnl ------------------------------------------
AC_CANONICAL_HOST

LDFLAGS="$LDFLAGS -L/usr/local/lib -ldl"
if test -z "$GCC"; then
        case $host in 
        *-*-irix*)
                dnl If we're on IRIX, we wanna use cc even if gcc 
                dnl is there (unless the user has overriden us)...
                if test -z "$CC"; then
                        CC=cc
                fi
		;;
        sparc-sun-solaris*)
                CFLAGS="-xO4 -fast -w -fsimple -native -xcg92"
		;;
        *)
                CFLAGS="-O" 
		;;
        esac
else

        case $host in 
        *86-*-linux*)
                CPPFLAGS="$CPPFLAGS -DUNIX -Wno-deprecated -Wall -Wimplicit -Wunused -Wmissing-prototypes -O1"
		LDFLAGS="$LDFLAGS -shared"
		dnl we could test for bad glibc here, but don't
		pd_suffix=pd_linux
		;;
        powerpc-*-linux*)
                CPPFLAGS="$CPPFLAGS -DUNIX -Wno-deprecated -Wall -Wimplicit -Wunused -Wmissing-prototypes -O1"
		LDFLAGS="$LDFLAGS -shared"
		pd_suffix=pd_linux
		;;
        *-*-linux*)
                CPPFLAGS="$CPPFLAGS -DUNIX -Wno-deprecated -Wall -Wimplicit -Wunused -Wmissing-prototypes -O1"
		LDFLAGS="$LDFLAGS -shared"
		pd_suffix=pd_linux
		;;
        sparc-sun-*)
		echo "YOU HAVE A SPARC STATION,	not setting any flags, not supported yet"
		;;
        *-*-darwin*)
                CPPFLAGS="$CPPFLAGS -DUNIX -Wno-deprecated -Wall -Wimplicit -Wunused -Wmissing-prototypes -O3 "
		LDFLAGS="$LDFLAGS -bundle -undefined suppress -flat_namespace"
		pd_suffix=pd_darwin
		;;
        *)
		dnl assume unix
                CPPFLAGS="$CPPFLAGS -DUNIX -Wno-deprecated -Wall -Wimplicit -Wunused -Wmissing-prototypes -O1"
		LDFLAGS="$LDFLAGS -shared"
		pd_suffix=pd_linux
		;;
        esac
fi


dnl -dylib -dynamic -flat_namespace -undefined suppress

echo
echo "Using cppflags= $CPPFLAGS"
echo "Using ldflags= $LDFLAGS"
echo








dnl ------------------------------------------
dnl ----  check if to build flext statically
dnl ------------------------------------------
AC_ARG_ENABLE(flext-shared,
  AC_HELP_STRING([--disable-flext-shared], [build with flext compiled in statically, not the best idea]),
  [flext_shared=$enableval], [flext_shared=yes])





dnl ------------------------------------------
dnl ----  add PureData includes dir 
dnl ----  usually /usr/local/include
dnl ------------------------------------------
AC_ARG_WITH(pd_dir, 
	[  --with-pd-dir=path	pd header path (default=/usr/local/include) ], 
	[
	CPPFLAGS="$CPPFLAGS -I$withval"
	echo
	echo "pd dir is $withval"
	echo
	])

dnl ------------------------------------------
dnl ---- check for PureData Header
dnl ------------------------------------------
AC_CHECK_HEADER(m_pd.h, [have_pd_hdr=yes ], [
	have_pd_hdr=no
	echo
	echo "no m_pd.h header found.  try with option --with-pd-dir=/path/to/pd/src"
	echo
	exit
	])



dnl ------------------------------------------
dnl ----  add Flext includes and lib dir 
dnl ----  usually /usr/local/lib/pd/flext
dnl ------------------------------------------
AC_ARG_WITH(flext_dir, 
	[  --with-flext-dir=path	flext headers path (default=/usr/local/lib/pd/flext) ], 
	[
	flext_dir=$withval
	CPPFLAGS="$CPPFLAGS -I$withval"
	])



dnl ------------------------------------------
dnl ----  add Flext Sys Type
dnl ----  1=Max/MSP  2=PureData
dnl ------------------------------------------
AC_ARG_WITH(flext-sys, 
	[  --with-flext-sys=<N>		1 for Max/Msp, 2 for PD (default=2)], 
	[if  test "$withval" = 1; then
		flext_sys=1
	elif test "$withval" = 2; then
		flext_sys=2
	else
	echo
	echo "Unknown FLEXT_SYS type, using default 2"
		flext_sys=2;
	echo
   fi
])



dnl ------------------------------------------
dnl ----  Checks for libraries.
dnl ------------------------------------------

dnl ------------------------------------------
dnl ----  Checks for Pthreads.
dnl ------------------------------------------
AC_CHECK_LIB(pthread, pthread_create, [ PTHREAD_LIBS="-lpthread" ], [ PTHREAD_LIBS=""])



dnl ------------------------------------------
dnl ----  Checks for SecretRabbitCode.
dnl ------------------------------------------
AC_CHECK_LIB(samplerate, src_is_valid_ratio, [ 
	SRC_LIBS="-lsamplerate" 
	AC_CHECK_HEADER(samplerate.h, [have_src_hdr=yes ], [  
        	echo
        	echo "found libsamplerate, but didn't find header.  make sure samplerate.h is somewhere"
		echo "that can be found and try configure again."
        	echo
        	exit
        	])
	
	], [ 
	AC_CHECK_HEADER(samplerate.h, [
		echo
        	echo "didn't find libsamplerate, but found samplerate.h"
		echo "this is mandatory for readanysf~, so we'll try to compile with libsamplerate and see."
        	echo
		SRC_LIBS="-lsamplerate"
		], [  
		SRC_LIBS=""
		echo "you need libsamplerate. download it here <http://www.mega-nerd.com/SRC/>"
		echo
	        exit
	        ])
	])



dnl ------------------------------------------
dnl ----  Checks for OggVorbis libraries.
dnl ------------------------------------------
AC_CHECK_LIB(ogg, ogg_stream_init, [have_ogg=yes ], [have_ogg=no])
AC_CHECK_LIB(vorbis, vorbis_info_init, [have_vorbis=yes ], [have_vorbis=no])
AC_CHECK_LIB(vorbisfile, ov_read_float, [have_vorbisfile=yes ], [have_vorbisfile=no])
if test "$have_ogg" = "yes"; then
        if test "$have_vorbis" = "yes"; then
	        if test "$have_vorbisfile" = "yes"; then
                	VORBIS_CFLAGS="-DREAD_VORBIS"
	                VORBIS_LIBS="-logg -lvorbis -lvorbisfile"
		fi
	fi
else
	VORBIS_LIBS=""
	echo
	echo "Vorbis library found, compiling WITHOUT vorbis support!"
fi




dnl ------------------------------------------
dnl ----  Checks for MAD mp3 decoding libraries
dnl ------------------------------------------
AC_CHECK_LIB(mad,  mad_frame_decode, [
	MAD_LIBS="-lmad"
	MAD_CFLAGS="-DREAD_MAD"
	have_mad=yes
	], [
	MAD_LIBS=""
	MAD_CFLAGS=""
	have_mad=no
	])




dnl ------------------------------------------
dnl ----  Checks for FLAC libraries.
dnl ------------------------------------------
AC_CHECK_LIB(FLAC, FLAC__seekable_stream_decoder_process_single, [
dnl	echo; echo "HAVE THE FUCKING FLAC LIB"; echo
        FLC_LIBS="-lFLAC"
	FLC_CFLAGS="-DREAD_FLAC"
	have_flc=yes
	], [
	echo "NO FLAC LIB, could be an autoconf bug (because of 'AC' in FLAC), checking for FLAC headers...."
	AC_CHECK_HEADER(FLAC/stream_decoder.h, [
		echo "no FLAC lib, found FLAC header, will try with FLAC support."
	        FLC_LIBS="-lFLAC"
		FLC_CFLAGS="-DREAD_FLAC"
		have_flc=yes
		], [
		echo "no FLAC lib, no FLAC header, no FLAC support.  sorry dude."
	        FLC_LIBS=""
		FLC_CFLAGS=""
		have_flc=no
		])
	])



dnl ------------------------------------------
dnl ----  Checks for FLEXT libraries.
dnl ------------------------------------------
AC_CHECK_LIB(flext, AddOutSignal, [have_flext=yes ], [have_flext=no])
if test "$have_flext" = "yes"; then
	if test  "$flext_shared" = "yes"; then
	        FLEXT_LIBS="-lflext"
		FLEXT_CFLAGS="-DFLEXT_SYS=$flext_sys  -DFLEXT_THREADS -DFLEXT_SHARED -DFLEXT_NOGLOBALNEW"
	else
		echo
		echo "FLEXT is not setup for dynamic linking, you should think about using this!"
		echo "try configure --enable-flext-shared"
		echo
		FLEXT_CFLAGS="-DFLEXT_SYS=$flext_sys  -DFLEXT_THREADS -DFLEXT_NOGLOBALNEW"
	        FLEXT_LIBS=""
	fi
else
	if test  "$flext_shared" = "yes"; then
		echo
		echo "you want to link to FLEXT dynamicaly, a very good idea, but..."
		echo "...configure couldn't find the FLEXT lib.  Taking your word for it."
		echo "This seems to be normal on linux.  Don't know why it happens."
		echo "just make sure you have the libflext.so in one of the paths listed"
		echo "under /etc/ld.so.conf (for linux).  and, as root do ldconfig"
		echo
	        FLEXT_LIBS="-lflext"
		FLEXT_CFLAGS="-DFLEXT_SYS=$flext_sys  -DFLEXT_THREADS -DFLEXT_SHARED -DFLEXT_NOGLOBALNEW"
	else
		echo
		echo "FLEXT is now setup to link statically. not the best idea, but it works"
		echo
		FLEXT_CFLAGS="-DFLEXT_SYS=$flext_sys  -DFLEXT_THREADS -DFLEXT_NOGLOBALNEW"
	        FLEXT_LIBS="$flext_dir/libflext_t.a"
	fi	
fi





AC_SUBST(FLEXT_LIBS)
AC_SUBST(FLEXT_CFLAGS)
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(SRC_LIBS)

AC_SUBST(VORBIS_LIBS)
AC_SUBST(VORBIS_CFLAGS)
AC_SUBST(MAD_LIBS)
AC_SUBST(MAD_CFLAGS)
AC_SUBST(FLC_LIBS)
AC_SUBST(FLC_CFLAGS)

AC_SUBST(pd_suffix)


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/time.h sys/socket.h unistd.h netinet/in.h pthread.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_TYPE_SIGNAL

dnl AC_OUTPUT(src/Makefile Makefile, echo timestamp > stamp-h)
AC_OUTPUT(src/Makefile Makefile)

echo
echo "_________________readanysf~__________________" 


if test "$flext_sys" = "1"; then
	echo "	+ FLEXT support for Max/MSP @ $flext_dir"
elif test "$flext_sys" = "2"; then
	echo "	+ FLEXT support for PureData @ $flext_dir"
fi

echo "	+ Wav,Aiff,Next decoding support (always)"

if test "$have_mad" = "yes"; then
	echo "	+ MAD mp3 decoding support"
else
	echo "	- MAD mp3 decoding support"
fi
if test "$have_vorbis" = "yes"; then
	echo "	+ Ogg Vorbis decoding support"
else
	echo "	- Ogg Vorbis decoding support"
fi
if test "$have_flc" = "yes"; then
	echo "	+ Flac decoding support"
else
	echo "	- Flac decoding support"
fi
echo "	+ decoding support for BOTH kinds of music(always)"
echo

echo "now type 'make'"
echo "type 'make install' to install into $bindir"
echo "do ./configure --bindir=/usr/local/lib/pd/extra to install it there"
echo
