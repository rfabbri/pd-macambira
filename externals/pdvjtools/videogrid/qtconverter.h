#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <quicktime/lqt.h>
//#include <quicktime/colormodels.h>
#include <lqt/lqt.h>
#include <lqt/colormodels.h>

/* 8bits clamp rgb values */

#define CLAMP8(x) (((x)<0) ? 0 : ((x>255)? 255 : (x)))

#define BYTESNOMFITXERIMATGE 512
#define BYTESTIPUSFROMAT 4

#define FORMAT_MINIATURA "ppm"
#define PATH_TEMPORAL "/tmp/vigrid_"
#define BYTES_NUM_TEMP 4

typedef char pathimage[BYTESNOMFITXERIMATGE];

typedef char tipus_format[BYTESTIPUSFROMAT];

int convertir_img(pathimage pathFitxer, tipus_format f, int W, int H, int posi);
/* void post(char args[]); */

