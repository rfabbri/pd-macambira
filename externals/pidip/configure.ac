dnl Process this file with autoconf to produce a configure script.

AC_INIT
PDP_PIDIP_VERSION=0.12.10
PD_DIR=/usr/local/pd
PDP_DIR=/usr/local/pd/pdp
FFMPEG_SOURCE_DIR=/SOURCES/ffmpeg

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_SANITY_CHECK

AM_MAINTAINER_MODE
AM_WITH_DMALLOC

AC_PATH_GENERIC(imlib2, 1.0.0, [
  AC_SUBST(IMLIB_LIBS)
  AC_SUBST(IMLIB_CFLAGS) ],
  AC_MSG_ERROR(Cannot find imlib2: Is imlib2-config in the path?) )

dnl the above doesn't work for some reason :/
IMLIB_LIBS=`imlib2-config --libs`
IMLIB_CFLAGS=`imlib2-config --cflags`
AC_SUBST(IMLIB_LIBS)
AC_SUBST(IMLIB_CFLAGS)

AC_PATH_GENERIC(aalib, 1.0.0, [
  AC_SUBST(AALIB_LIBS)
  AC_SUBST(AALIB_CFLAGS) ],
  AC_MSG_ERROR(Cannot find aalib: Is aalib-config in the path?) )

dnl the above doesn't work for some reason :/
AALIB_LIBS=`aalib-config --libs`
AALIB_CFLAGS=`aalib-config --cflags`
AC_SUBST(AALIB_LIBS)
AC_SUBST(AALIB_CFLAGS)

AC_CHECK_LIB(Magick, XWindowByProperty, , AC_MSG_ERROR(screen captures requires ImageMagick library!!), -I/usr/X11R6/include -L/usr/X11R6/lib )
AC_CHECK_LIB(bz2, BZ2_bzBuffToBuffCompress, , AC_MSG_ERROR(streaming requires bz2 library!!))
AC_CHECK_LIB(z, deflate, , AC_MSG_ERROR(streaming requires compress library!!))
AC_CHECK_LIB(dl, dlopen, , AC_MSG_ERROR(streaming requires dynamic loader library!!))
AC_CHECK_LIB(mp3lame, InitMP3, , AC_MSG_ERROR(streaming requires lame library!!), -lm)
AC_CHECK_LIB(ogg, ogg_stream_init, , AC_MSG_ERROR(streaming requires ogg library!!), -lm)
AC_CHECK_LIB(vorbis, vorbis_analysis_init, , AC_MSG_ERROR(streaming requires vorbis library!!), -lm)
AC_CHECK_LIB(vorbisenc, vorbis_encode_setup_init, , AC_MSG_ERROR(streaming requires vorbis encoder library!!), -lm)

AC_SUBST(PDP_PIDIP_VERSION)

MAGICK_LIBS="-L/usr/X11R6/lib -lMagick `Magick-config --libs` `Magick-config --ldflags`"
MAGICK_CFLAGS="-I/usr/X11R6/include `Magick-config --cflags` "
AC_SUBST(MAGICK_LIBS)
AC_SUBST(MAGICK_CFLAGS)

echo -n "looking for pd sources (required) ... "
if test -f $PD_DIR/src/m_pd.h
then
  AC_SUBST(PD_DIR)
  echo "ok." 
else
  echo "pd source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi

echo -n "looking for pdp sources (required) ... "
if test -f $PDP_DIR/CHANGES.LOG
then
  AC_SUBST(PDP_DIR)
  echo "ok." 
else
  echo "pdp source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi

echo -n "looking for ffmpeg sources (required) ... "
if test -f $FFMPEG_SOURCE_DIR/libavformat/avformat.h
then
  AC_SUBST(FFMPEG_SOURCE_DIR)
  echo "ok." 
else
  echo "ffmpeg source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi

LIBS="$LIBS -lbz2 -lz -ldl -lmp3lame -logg -lvorbis -lvorbisenc"
CFLAGS="$CFLAGS -Wall"

AC_CONFIG_FILES([
Makefile
system/Makefile
modules/Makefile
])
AC_OUTPUT
