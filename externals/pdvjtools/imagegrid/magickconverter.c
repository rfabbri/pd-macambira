#include "magickconverter.h"

void convertir(pathimage pathFitxer, tipus_format f, int W, int H, int posi){

    MagickBooleanType
            status;
    MagickWand
            *magick_wand;
    pathimage ig_path = PATH_TEMPORAL;
    char posi_str[BYTES_NUM_TEMP];

    /*printf("\nEl path %s i el format %s\n",pathFitxer,f);*/

    /*
    Read an image.
    */
    MagickWandGenesis();
    magick_wand=NewMagickWand();
    status=MagickReadImage(magick_wand,pathFitxer);
    if (status == MagickFalse)
        ThrowWandException(magick_wand);
    /*
    Turn the images into a thumbnail sequence.
    */
    MagickResetIterator(magick_wand);
    while (MagickNextImage(magick_wand) != MagickFalse)
        MagickResizeImage(magick_wand,W,H,LanczosFilter,1.0);
    /*
    Write the image as 'f' and destroy it.
    */
    sprintf(posi_str, "%d", posi);
    strcat(ig_path,posi_str);
    strcat(ig_path,".");
    strcat(ig_path,f);
 
    /* printf("\nEl nou path %s i el format %s\n",ig_path,f); */
    status=MagickWriteImages(magick_wand,ig_path,MagickTrue);
    if (status == MagickFalse)
        ThrowWandException(magick_wand);
    magick_wand=DestroyMagickWand(magick_wand);
    MagickWandTerminus();
}

/*
int main(void){
    pathimage imatge = "/usr/lib/pd/extra/imagegrid/gifpmmimages/3160x120.gif";
    format fo = "ppm";
    int numi = 1;
    convertir(imatge,fo,60,40,1);
    return(0);
}
*/
