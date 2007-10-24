#include "qtconverter.h"

/*
void post(char args[]){
    printf("%s",args);
}
*/

int convertir_img(pathimage pathFitxer, tipus_format f, int W, int H, int posi){
    /*
    Quicktime per les conversions
    */

    /* RGB vectors */
    unsigned char * qt_rows[3];
    /* YUV vesctor frame */
    unsigned char *qt_frame = NULL;
    /* quicktime decoder */
    quicktime_t *qt;
    /* quicktime color model */	
    int qt_cmodel;

    int nN = posi;
    int x_vwidth = 0;
    int x_vheight = 0;
    /* convertir(entrada,FORMAT_MINIATURA, W_CELL, H_CELL, nN); */
    qt = quicktime_open(pathFitxer, 1, 0);

    if (!(qt)){
        /* post("videogrid: error opening qt file"); */
        return -1;
    }

    if (!quicktime_has_video(qt)) {
        /* post("videogrid: no video stream"); */
        quicktime_close(qt);
        return -1;
	
    }
    else if (!quicktime_supported_video(qt,0)) {
        /* post("videogrid: unsupported video codec\n"); */
        quicktime_close(qt);
        return -1;
    }
    else 
    {
        qt_cmodel = BC_YUV420P;
        x_vwidth  = quicktime_video_width(qt,0);
        x_vheight = quicktime_video_height(qt,0);
	
	
        free(qt_frame);
        qt_frame = (unsigned char*)malloc(x_vwidth*x_vheight*4);
	
        int size = x_vwidth * x_vheight;
        qt_rows[0] = &qt_frame[0];
        qt_rows[2] = &qt_frame[size];
        qt_rows[1] = &qt_frame[size + (size>>2)];
 
        quicktime_set_cmodel(qt, qt_cmodel);
    }

    /* int length = quicktime_video_length(qt,0); */
    /* int Vpos = quicktime_video_position(qt,0); */
    lqt_decode_video(qt, qt_rows, 0);
 
    switch(qt_cmodel){
        case BC_YUV420P:
            printf(" ");
            /* post("videogrid: qt colormodel : BC_YUV420P"); */
	
	/* per a fer la miniatura
	   cada k colomnes pillem una
	   cada l files pillem una */
            int w = x_vwidth;
            int h = x_vheight;
            int k = (w/W);
            int l = (h/H);

            /*int cont=0;*/
            
            char nNstr[BYTES_NUM_TEMP];
            pathimage  ig_path = PATH_TEMPORAL;

            sprintf(nNstr, "%d", nN);
            strcat(ig_path,nNstr);
            strcat(ig_path,".");
            strcat(ig_path,FORMAT_MINIATURA);
        /*    printf("Creacio de la imatge %s ...",ig_path); */
	/* escriu el contingut de data a un arxiu. */
            FILE *fp = fopen(ig_path, "w");
            fprintf (fp, "P6\n%d %d\n255\n", W, H);

            int i,j,y,u,v,r,g,b;

            for (i=0;i<(l*H);i=i+l) {
                for (j=0;j<(k*W);j=j+k) {
                    y=qt_rows[0][(w*i+j)];
                    u=qt_rows[1][(w/2)*(i/2)+(j/2)];
                    v=qt_rows[2][(w/2)*(i/2)+(j/2)];
                    r = CLAMP8(y + 1.402 *(v-128));
                    g = CLAMP8(y - 0.34414 *(u-128) - 0.71414 *(v-128));
                    b = CLAMP8(y + 1.772 *(u-128));
                    fprintf (fp, "%c%c%c", r,g,b);
                }
            }

	/* escriu el contingut de data a un arxiu.*/
            fclose (fp);
    }
    return 0;
}



/*
cc -fPIC -c -O -Wall -Wmissing-prototypes -o qtconverter.o -c qtconverter.c
cc -o qtconverter qtconverter.o -lc -lm -lquicktime `Wand-config --ldflags --libs`
./qtconverter 
*/

/*
int main(void){
    pathimage imatge = "/usr/lib/pd/extra/videogrid/videos/dscn0243.mov";
    tipus_format fo = "ppm";
    int flauta;
    flauta = convertir(imatge,fo,60,40,2);
    printf("\n%d FET\n", flauta);
    return(0);
}
*/


