/* Base/configLinux.h.  Generated from configLinux.h.in by configure.  */

/* fullscreen: querying via libXxf86vm */
/* #undef HAVE_LIBXXF86VM */

/* font rendering */
/* #undef HAVE_LIBFTGL */

/* image loading / saving */
#define HAVE_LIBTIFF 1
#define HAVE_LIBJPEG 1
/* #undef HAVE_LIBMAGICKPLUSPLUS */

/* movie decoding */
#define HAVE_LIBMPEG 1
#define HAVE_LIBMPEG3 1
#define HAVE_LIBQUICKTIME 1
#define HAVE_LQT_OPEN_WRITE 1
/* #undef HAVE_LIBAVIPLAY */
#define HAVE_FFMPEG 1
#define GEM_AVSTREAM_CODECPOINTER 1

/* video grabbing */
#define HAVE_VIDEO4LINUX 1
/* #undef HAVE_VIDEO4LINUX2 */
#define HAVE_LIBDV 1

/* image analysis */
/* #undef HAVE_ARTOOLKIT */

/* posix threads */
#define HAVE_PTHREADS 1

/* enable the use of the all-in-one video/movie objects */
#define NEW_VIDEOFILM 1

/* types, structures, compiler characteristics, ... */
#define SIZEOF_VOID_P 4
#define SIZEOF_UNSIGNED_INT 4

