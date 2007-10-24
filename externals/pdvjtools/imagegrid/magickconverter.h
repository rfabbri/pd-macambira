#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <wand/MagickWand.h>*/
#include <wand/magick-wand.h>

#define BYTESNOMFITXERIMATGE 512
#define BYTESTIPUSFROMAT 4

#define FORMAT_MINIATURA "ppm"
#define PATH_TEMPORAL "/tmp/imgrid_"
#define BYTES_NUM_TEMP 4

#define ThrowWandException(wand) \
{ \
    char \
    *description; \
    \
    ExceptionType \
    severity; \
    \
    description=MagickGetException(wand,&severity); \
    (void) fprintf(stderr,"%s %s %ld %s\n",GetMagickModule(),description); \
    description=(char *) MagickRelinquishMemory(description); \
    exit(-1); \
}

typedef char pathimage[BYTESNOMFITXERIMATGE];

typedef char tipus_format[BYTESTIPUSFROMAT];

void convertir(pathimage pathFitxer, tipus_format f, int W, int H, int posi);
