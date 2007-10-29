/*
videogrid external for Puredata
Copyright (C) 2007  Sergi Lario
sll :: slario-at-gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* incloure les definicions de variables i 
   prototipus de dades i de funcions de puredata */
#include "m_pd.h"
/* incloure estructures de dades i capceleres de funcions gàfiques bàsiques de pd */
#include "g_canvas.h"
/* incloure estructures de dades i capceleres de funcions per a gestionar una cua */
#include "cua.h"
/* incloure estructures de dades i capceleres de funcions per convertir imatges a diferents formats */
/* #include "magickconverter.h" */
/* incloure estructures de dades i capceleres de funcions per tractar frames de vídeo */
#include "qtconverter.h"
/* incloure estructures de dades i capceleres de funcions per traballar amb threads */
#include "pthread.h"

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&& VIDEOGRID &&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

/* definició de l'amplada i l'alçada d'una casella */
#define W_CELL 60
#define H_CELL 40

/* crear un apuntador al nou objecte */
static t_class *videogrid_class;
/* indica el nombre de videogrid creats - utilitzat per diferenciar el nom d'instàncies d'objectes del mateix tipus */
static int videogridcount = 0;

/* definició de la classe i la seva estructura de dades */

typedef struct _videogrid {
    t_object  x_obj; 
    /* declaració de la sortida de l'objecte */
    t_outlet *x_sortida;
    /* llista d'objectes gràfics */
    t_glist *x_glist;
    /* nombre de files */
    int x_num_fil;
    /* nombre de columnes */
    int x_num_col;
    /* posició de la última imatge en el tauler */
    int x_ultima_img;
    /* path del directori actual */
    path x_dir_actual;
    /* path del directori a canviar */
    path x_dir_canvi;
    /* posicio ultim al directori actual */
    int x_dir_pos;
    /* apuntador al primer element posicionat al tauler */
    Node *x_tauler_primer;
    /* cua d'imatges */
    Cua x_cua;
    /* nom de l'objecte */
    t_symbol *x_name;
    /* color de fons */
    t_symbol *x_color_fons;
    /* color del marge */
    t_symbol *x_color_marc;
    /* mutex per evitar concurrencia sobre la cua al accedir diferents threads*/
    pthread_mutex_t x_lock;

} t_videogrid;


/* calcula la posició x del tauler a partir de la posició de l'element de la cua (d'esquerra a dreta) */
int getX(t_videogrid* x, int posCua){
    int c = x->x_num_col;
    int xpos = (posCua % c) * W_CELL;
    return(xpos + 1);
}

/* calcula la posició y del tauler a partir de la posició de l'element de la cua (de dalt a baix) */
int getY(t_videogrid* x, int posCua){
    int c = x->x_num_col;
    int ypos = (posCua / c) * H_CELL;
    return(ypos + 1);
}

/* elimina les imatges temporals */
void eliminar_imatges_temporals(int maxim){
    FILE *fitxer;
    path path_total;
    int contador = 0;
    char contador_str[BYTES_NUM_TEMP];
    while(contador < maxim){
        strcpy(path_total,PATH_TEMPORAL);
        sprintf(contador_str,"%d", contador);
        strcat(path_total,contador_str);
        strcat(path_total,".");
        strcat(path_total,FORMAT_MINIATURA);
        /* elimina el fitxer si no hi ha cap problema */
        if(unlink(path_total)){
            /* post("Imatge temporal %s eliminada\n",path_total); */
        }
        contador++;
    }
    post("Videogrid: Imatges temporals eliminades\n",path_total);
}

int format_adequat_v(path nomF){
    int retorn = 0;
    path  ig_path = "";
    strcat(ig_path,nomF);
    char *t1;
    path extensio = "";
    for ( t1 = strtok(ig_path,".");
          t1 != NULL;
          t1 = strtok(NULL,".") )
        strcpy(extensio,t1);
    if(strcmp(extensio,"mov")==0) retorn = 1;
    /*
    if(strcmp(extensio,"eps")==0) retorn = 1;
    if(strcmp(extensio,"gif")==0) retorn = 1;
    if(strcmp(extensio,"jpg")==0) retorn = 1;
    if(strcmp(extensio,"jpeg")==0) retorn = 1;
    if(strcmp(extensio,"png")==0) retorn = 1;
    if(strcmp(extensio,"ppm")==0) retorn = 1;
    if(strcmp(extensio,"tif")==0) retorn = 1;
    if(strcmp(extensio,"tiff")==0) retorn = 1;
    */

    return (retorn);
}

/* afegir una imatge al grid */
void videogrid_afegir_imatge(t_videogrid *x, path entrada)
{
    int maxim;
    char nNstr[BYTES_NUM_TEMP];
    int pos = 0;
    /* escriu l'argument entrat */
    if (format_adequat_v(entrada) == 1){
        /* post("Afegint la imatge %s ...",entrada); */
        maxim = x->x_num_fil * x->x_num_col;
        path  ig_path = PATH_TEMPORAL;

        /* si hi ha tants nodes a la cua com el maxim */
        if((numNodes(&x->x_cua)) >=  maxim){
            /* desencua */
            int extret;
            extret = desencuar(&x->x_cua);
            /* obtenir la posició en la cua del nou node */
            if(x->x_ultima_img == maxim-1) {
                pos = 0;
            }else{
                pos = x->x_ultima_img+1;
            }
            sys_vgui(".x%x.c delete %xS%d\n", glist_getcanvas(x->x_glist), x, pos);
        }
        /* encua el nou node */
        encuar(&x->x_cua, entrada);
        /* si no és el primer element a encuar incrementem la posicio de la última imatge insertada */
        if(numNodes(&x->x_cua) != 1) x->x_ultima_img ++;
        /* si assoleix el maxim torna a començar, inicialitzant la posició en el tauler de la última imatge insertada */
        if(x->x_ultima_img == maxim) x->x_ultima_img = 0;

        /*
        Quicktime per les conversions
        */
        int nN = x->x_ultima_img;
        convertir_img(entrada,FORMAT_MINIATURA, W_CELL, H_CELL, nN);
        sprintf(nNstr, "%d", nN);
        strcat(ig_path,nNstr);
        strcat(ig_path,".");
        strcat(ig_path,FORMAT_MINIATURA);
        /*printf("Creacio de la imatge %s ...",ig_path);*/
        sys_vgui("image create photo img%x%d -file %s\n",x,nN,ig_path);
        /* printf("1. Creacio de la imatge %s ...",ig_path); */
        sys_vgui(".x%x.c create image %d %d -image img%x%d -tags %xS%d\n",
             glist_getcanvas(x->x_glist),
             text_xpix(&x->x_obj, x->x_glist) + getX(x,nN) + (W_CELL/2), 
             text_ypix(&x->x_obj, x->x_glist) + getY(x,nN) + (H_CELL/2),
             x,nN,x,nN);
        /* printf("2. Creacio de la imatge %s ...",ig_path); */
        if(nN == 0){
            x->x_tauler_primer = x->x_cua.final;
            /* post("Ara el primer del tauler es %s\n",x->x_tauler_primer->pathFitxer); */
        }
        /* printf("SURT de la creacio de la imatge %s ...",ig_path); */
    }else{
        post("Videogrid: El format del fitxer %s és incompatible.",entrada);
    }
    /*
    sys_vgui("image create photo img%x -file %s\n",x,entrada);
    sys_vgui(".x%x.c create image %d %d -image img%x -tags %xS\n", 
    glist_getcanvas(glist),text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x,x);
    */
}

/* dibuixa videogrid */
void videogrid_drawme(t_videogrid *x, t_glist *glist, int firsttime)
{
    /* post("Entra a drawme amb firsttime: %d", firsttime); */
    if (firsttime) {
        char name[MAXPDSTRING];
        canvas_makefilename(glist_getcanvas(x->x_glist), x->x_name->s_name, name, MAXPDSTRING);
        sys_vgui(".x%x.c create rectangle %d %d %d %d -fill %s -tags %xGRID -outline %s\n",
            glist_getcanvas(glist),
            text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
            text_xpix(&x->x_obj, glist) + (x->x_num_col * W_CELL) + 1, text_ypix(&x->x_obj, glist) + (x->x_num_fil * H_CELL) + 1,
            x->x_color_fons->s_name, x,x->x_color_marc->s_name);

        canvas_fixlinesfor(glist_getcanvas(glist), (t_text*)x);
        /* si hi elements a la cua els afegeix (redimensió) */
        if(!cuaBuida(&x->x_cua))
        {
            path  ig_path;
            int nN = 0;
            char nNstr[BYTES_NUM_TEMP];
            Node *actual;
            actual=x->x_cua.davanter;
            do{
                strcpy(ig_path,PATH_TEMPORAL);
                sprintf(nNstr, "%d", nN);
                strcat(ig_path,nNstr);
                strcat(ig_path,".");
                strcat(ig_path,FORMAT_MINIATURA);
                /* post("reestablint la imatge %s", actual->pathFitxer); */
                // videogrid_afegir_imatge(x,actual->pathFitxer);
                convertir_img(actual->pathFitxer,FORMAT_MINIATURA, W_CELL, H_CELL, nN);
                sys_vgui("image create photo img%x%d -file %s\n",x,nN,ig_path);
                sys_vgui(".x%x.c create image %d %d -image img%x%d -tags %xS%d\n", 
                         glist_getcanvas(x->x_glist),text_xpix(&x->x_obj, x->x_glist) + getX(x,nN) + (W_CELL/2), text_ypix(&x->x_obj, x->x_glist) + getY(x,nN) + (H_CELL/2),x,nN,x,nN);
                actual = actual->seguent;
                nN++;
            }while(actual);
        }
    }
    else { 
        sys_vgui(".x%x.c coords %xGRID %d %d %d %d\n", glist_getcanvas(glist), x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),text_xpix(&x->x_obj, glist) + (x->x_num_col*W_CELL) + 1, text_ypix(&x->x_obj, glist) + (x->x_num_fil*H_CELL) + 1);
        if(!cuaBuida(&x->x_cua))
        {
            int contador = 0;
            while(contador < numNodes(&x->x_cua)){
                sys_vgui(".x%x.c coords %xS%d \
                        %d %d\n",
                glist_getcanvas(glist), x, contador,
                text_xpix(&x->x_obj, x->x_glist) + getX(x,contador) + (W_CELL/2), text_ypix(&x->x_obj, x->x_glist) + getY(x,contador) + (H_CELL/2));
                contador++;
            }
            
           /* char buf[800];
            sprintf(buf, "pdtk_videogrid_table %%s %s %d %d\n", x->x_name->s_name, x->x_num_fil, x->x_num_col);
            gfxstub_new(&x->x_obj.ob_pd, x, buf); */
        }
    }
}

/* borra videogrid */
void videogrid_erase(t_videogrid* x,t_glist* glist)
{
    int maxim = x->x_num_fil * x->x_num_col;
    path path_total;
    char contador_str[2];
    /* post("Entra a erase"); */
    /* elimina les imatges */
    int contador = 0;
    while(contador < numNodes(&x->x_cua)){
        sys_vgui(".x%x.c delete %xS%d\n", glist_getcanvas(x->x_glist), x, contador);
        strcpy(path_total,PATH_TEMPORAL);
        sprintf(contador_str,"%d", contador);
        strcat(path_total,contador_str);
        strcat(path_total,".");
        strcat(path_total,FORMAT_MINIATURA);
        if(unlink(path_total)){
            /* post("Imatge temporal %s eliminada\n",path_total); */
        }
        contador++;
    }

    /* elimina el grid */
    sys_vgui(".x%x.c delete %xGRID\n", glist_getcanvas(glist), x);
    eliminar_imatges_temporals(maxim);
}

/* mètode de la clase que escriu un missatge al rebre un bang */
void videogrid_bang(t_videogrid *x)
{
    /* post("Hello videogrid !!"); */
    escriuCua(&x->x_cua);
}

/* mètode de la classe que es dispara al rebre una entrada de missatge amb [putvideo +string( com a paràmetre */
void videogrid_putvideo(t_videogrid *x, t_symbol *entrada)
{
    /* comprova que existeixi el fitxer */
    FILE *fitxer;
    path e;
    strcpy(e,entrada->s_name);
    /* post("putvideo de %s\n", e); */
    
    fitxer = fopen(e,"r");
    if (!fitxer) {
        post("Videogrid: Problema amb l'obertura del fitxer %s\n",e);
    }
    else {
        /* post("s'encua la imatge %s\n", e); */
        videogrid_afegir_imatge(x,e);
        /*outlet_symbol(x->x_sortida, entrada);*/
    }
    /* post("putvideo amb img = %s\n", e); */
}

/* mètode de la classe que es dispara al rebre una entrada de missatge amb [putvideodir +string( com a paràmetre */
void *videogrid_putvideodir_thread(t_videogrid *x)
{	
    DIR *dirp;
    struct dirent * direntp;
    path nomImatge, directoriAnterior, pathActual;
    int numEncuats = 0, numPosDir = 0;
    int maxim;
    if ((dirp = opendir(x->x_dir_canvi)) == NULL)
    {
        post("Videogrid: No es pot obrir el directori %s\n", x->x_dir_canvi);
    }else{
        maxim = x->x_num_fil * x->x_num_col;
        strcpy(directoriAnterior, x->x_dir_actual);
        strcpy(x->x_dir_actual, x->x_dir_canvi);
        /* si es el mateix directori entrat l'ultim busca la ultima imatge afegida per a seguir a encuant a partir d'ella en endavant */
        if(strcmp(directoriAnterior, x->x_dir_actual) == 0){
            post("Videogrid: Repeteix directori %s\n", x->x_dir_actual);
            while ( (direntp = readdir( dirp )) != NULL ){
                /* es descarta el mateix directori, el directori anterior i tot el que no sigui un fitxer regular */
                if((strcmp(direntp->d_name,"..") != 0)&&(strcmp(direntp->d_name,".") != 0)&&(direntp->d_type == DT_REG)){
                    /* incrementa la posició en el directori */
                    numPosDir++;
                    /* assolir la posició anterior en el directori */
                    if(numPosDir > x->x_dir_pos){
                        /* si el nombre de nodes encuats per aquest directori no supera el màxim encua el nou node */
                        if(numEncuats < maxim){
                            /* post("s'encua la imatge %s\n", direntp->d_name); */
                            /* concatena el path i el nom de la imatge */
                            strcpy(nomImatge,direntp->d_name);
                            strcpy(pathActual,x->x_dir_actual);
                            strcat(pathActual,"/");
                            strcat(pathActual,nomImatge);
			    pthread_mutex_lock(&x->x_lock);
                            videogrid_afegir_imatge(x, pathActual);
			    pthread_mutex_unlock(&x->x_lock);
                            /* incrementa en 1 per indicar el nombre de nodes encuats per aquest directori */
                            numEncuats++;
                            /* es desa la posició en el directori de l'últim node encuat */
                            x->x_dir_pos = numPosDir;
                        }
                    }
                }
            }
        }else{
            /* directori diferent omple la cua començant pel primer fitxer */
            post("Videogrid: Nou directori %s \n", x->x_dir_actual);
            while ( (direntp = readdir( dirp )) != NULL ){
                /* es descarta el mateix directori, el directori anterior i tot el que no sigui un fitxer regular */
                if((strcmp(direntp->d_name,"..") != 0)&&(strcmp(direntp->d_name,".") != 0)&&(direntp->d_type == DT_REG)){
                    /* incrementa la posició en el directori */
                    numPosDir++;
                    /* si el nombre de nodes encuats per aquest directori no supera el màxim enca el nou node */
                    if(numEncuats < maxim){ 
                        /* post("s'encua la imatge %s\n", direntp->d_name); */
                        /* concatena el path i el nom de la imatge */
                        strcpy(nomImatge,direntp->d_name);
                        strcpy(pathActual,x->x_dir_actual);
                        strcat(pathActual,"/");
                        strcat(pathActual,nomImatge);
			pthread_mutex_lock(&x->x_lock);
                        videogrid_afegir_imatge(x, pathActual);
			pthread_mutex_unlock(&x->x_lock);
                        /* incrementa en 1 per indicar el nombre de nodes encuats per aquest directori */
                        numEncuats++;
                        /* es desa la posició en el directori de l'últim node encuat */
                        x->x_dir_pos = numPosDir;
                    }
                }
            }
        }
        /* si la posicio de l'actual es la de l'utim fitxer del directori, inicialitza la posició */
        if(x->x_dir_pos >= numPosDir) x->x_dir_pos = 0;
        closedir(dirp);
    }
    /* escriu l'argument entrat */
    /* post("Obtenint imatges del directori: %s ...",x->x_dir_canvi); */
    /* envia a la sorida l'argument entrat */
    /* outlet_symbol(x->x_sortida, entrada); */
    pthread_exit(NULL);
}

void videogrid_putvideodir(t_videogrid *x, t_symbol *entrada)
{

    pthread_t unthread;
    pthread_attr_t unatribut;
    pthread_attr_init( &unatribut );
    
    strcpy(x->x_dir_canvi,entrada->s_name);
     
    // ----------------      THREAD CREAT -------------------------
    pthread_mutex_init(&x->x_lock, NULL);
    pthread_create(&unthread,&unatribut,(void *)videogrid_putvideodir_thread, x);
    pthread_mutex_destroy(&x->x_lock);
}

static int videogrid_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_videogrid* x = (t_videogrid *)z;
    int x_pos = xpix - text_xpix(&x->x_obj, x->x_glist);
    int y_pos = ypix - text_ypix(&x->x_obj, x->x_glist);
    int xa, ya, postauler, contador, maxim;
    path pathSortida;
    Node *actual;
    if (doit) 
    {
        /* obtenir la posicio en el tauler */
        xa = x_pos / W_CELL;
        ya = (y_pos / H_CELL) * x->x_num_col;
        postauler = ya + xa;
        /* obtenir el path per enviar a la sortida */
        if((!cuaBuida(&x->x_cua))&&(postauler < numNodes(&x->x_cua))){
            contador = 0;
            maxim = x->x_num_fil * x->x_num_col;
            if(x->x_tauler_primer){
                actual = x->x_tauler_primer;
                while(contador <= postauler){
                    if(contador == postauler){
                        strcpy(pathSortida,actual->pathFitxer); 
                    }
                    if(actual->seguent == NULL){
                        actual = x->x_cua.davanter;
                    }else{
                        actual = actual->seguent;
                    }
                    contador++;
                }
                outlet_symbol(x->x_sortida, gensym(pathSortida));
            }
        }
    }
    return (1);
}

static void videogrid_getrect(t_gobj *z, t_glist *glist,int *xp1, int *yp1, int *xp2, int *yp2)
{
    int cols, fils;
    t_videogrid* x = (t_videogrid*)z;
    cols = x->x_num_col;
    fils = x->x_num_fil;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + (cols*W_CELL);
    *yp2 = text_ypix(&x->x_obj, glist) + (fils*H_CELL);
  /*  post("Esta amb el ratoli en el punt %d %d %d %d o son els vetexs de la caixa... es/bd", xp1, yp1, xp2, yp2); */
}

static void videogrid_displace(t_gobj *z, t_glist *glist,int dx, int dy)
{
    /* post("Entra a displace amb dx %d i dy %d", dx, dy); */
    t_videogrid *x = (t_videogrid *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    sys_vgui(".x%x.c coords %xGRID %d %d %d %d\n",
             glist_getcanvas(glist), x,
             text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
             text_xpix(&x->x_obj, glist) + (x->x_num_col*W_CELL), text_ypix(&x->x_obj, glist) + (x->x_num_fil*H_CELL));
    videogrid_drawme(x, glist, 0);
    canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
}

static void videogrid_select(t_gobj *z, t_glist *glist, int state)
{
    /* post("Entra select amb state %d", state); */
    t_videogrid *x = (t_videogrid *)z;
    if (state) {
        /* post("Videogrid seleccionat"); */
        sys_vgui(".x%x.c itemconfigure %xGRID -outline #0000FF\n", glist_getcanvas(glist), x);
    }
    else {
        /* post("Videogrid deseleccionat"); */
        sys_vgui(".x%x.c itemconfigure %xGRID -outline %s\n", glist_getcanvas(glist), x, x->x_color_marc->s_name);
    }
}

static void videogrid_delete(t_gobj *z, t_glist *glist)
{
    /* post("Entra a delete"); */
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

static void videogrid_vis(t_gobj *z, t_glist *glist, int vis)
{
    /* post("Entra a vist amb vis %d", vis); */
    t_videogrid* s = (t_videogrid*)z;
    if (vis)
        videogrid_drawme(s, glist, 1);
    else
       videogrid_erase(s,glist);
}


static void videogrid_save(t_gobj *z, t_binbuf *b)
{
    /* post("Entra a save"); */
    t_videogrid *x = (t_videogrid *)z;
    /* crea la cadena de paths per desar */
    /* 100 possibles paths com a màxim a 512 cada path*/
    /* char cadenaPaths[51200];*/
    char *cadenaPaths, *cadenaPathsInicials;
    path ultimPath = "";
    cadenaPaths = (char *)malloc(51200*sizeof(char));
    strcpy(cadenaPaths,"");
    cadenaPathsInicials = (char *)malloc(51200*sizeof(char));
    strcpy(cadenaPathsInicials,"");
    /*strcpy(cadenaPaths,(char *)argv[5].a_w.w_symbol->s_name);*/
    if(!cuaBuida(&x->x_cua))
    {
        Node *actual;
        int maxim = x->x_num_fil * x->x_num_col;
        int contador = x->x_ultima_img + 1;
        
        if (contador > maxim) { 
               contador = 0;
        }
        /* printf("\n contador %d i maxim %d i laultimaPOS %d \n", contador, maxim, x->x_ultima_img); */
        /*
        strcat(cadenaPaths, actual->pathFitxer);
        strcat(cadenaPaths, "1|\n");
        contador ++;
        */
        /* prenem el davanter de la cua */
        actual=x->x_cua.davanter;
        
        while(contador < numNodes(&x->x_cua)){
            /* afegim els paths del davanter fins a l'ultim node al tauler */
            strcat(cadenaPaths, actual->pathFitxer);
            strcat(cadenaPaths, "|");
            actual = actual->seguent;
            contador ++;
        }
        if(actual != x->x_cua.final){
                /* ara resten els de de l'inici del tauler fins al final de la cua */
                while(actual != x->x_cua.final){
                    strcat(cadenaPathsInicials, actual->pathFitxer);
                    strcat(cadenaPathsInicials, "|");
                    actual = actual->seguent;
                }
                /* afegeix l'últim */
                strcat(ultimPath, actual->pathFitxer);
                strcat(ultimPath, "|");
                /* afegeix l'ultim de la cua */
                strcat(cadenaPathsInicials, ultimPath);
        }else{
            if(x->x_ultima_img == 0){
                strcat(ultimPath, actual->pathFitxer);
                strcat(ultimPath, "|");
                strcat(cadenaPathsInicials, ultimPath);
            }
        }
        /* ordena el paths segons aparicio en el tauler */
        strcat(cadenaPathsInicials, cadenaPaths);
        /* DE MOMENT NO DESA ELS PATHS */
        strcat(cadenaPathsInicials, "");
        /* printf("%s",cadenaPathsInicials); */
    }

    binbuf_addv(b, "ssiissiisss", gensym("#X"),gensym("obj"),
    x->x_obj.te_xpix, x->x_obj.te_ypix,
    atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
    x->x_name,x->x_num_fil,x->x_num_col,x->x_color_fons,x->x_color_marc,gensym(cadenaPathsInicials));
    binbuf_addv(b, ";");
}

static void videogrid_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_videogrid *x=(t_videogrid *)z;

    /* post("Es crida a pdtk_videogrid dialog passant nom = %s\n fils = %d \t cols = %d \t color fons = %s \t color marc = %s\n", x->x_name->s_name, x->x_num_fil, x->x_num_col, x->x_color_fons->s_name, x->x_color_marc->s_name); */
    sprintf(buf, "pdtk_videogrid_dialog %%s %s %d %d %s %s\n",
            x->x_name->s_name, x->x_num_fil, x->x_num_col, x->x_color_fons->s_name, x->x_color_marc->s_name);
    /* post("videogrid_properties : %s", buf ); */
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void videogrid_dialog(t_videogrid *x, t_symbol *s, int argc, t_atom *argv)
{
    int maxim, nfil, ncol, maxdigit;
    if ( !x ) {
        post("Videogrid: error_ intent de modificar le propietats d'un objecte inexistent\n");
    }
    if ( argc != 5 )
    {
        post("Videogrid: error_ sobre el nombre d'arguments ( 5 enlloc de %d )\n",argc);
        return;
    }
    if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT || argv[3].a_type != A_SYMBOL || argv[4].a_type != A_SYMBOL) 
    {
        post("Videogrid: error_ algun dels arguments no es del tipus adequat\n");
        return;
    }
    x->x_name = argv[0].a_w.w_symbol;
    nfil = (int)argv[1].a_w.w_float;
    ncol = (int)argv[2].a_w.w_float;
    x->x_color_fons = argv[3].a_w.w_symbol;
    x->x_color_marc = argv[4].a_w.w_symbol;
    /* el màxim es fixa pel nombre de digits utilitzats pel nom de la imatge temporal */
    maxdigit = pow(10,BYTES_NUM_TEMP);
    if((nfil*ncol) <= maxdigit){
        if((nfil*ncol) > 0){
            x->x_num_fil = nfil;
            x->x_num_col = ncol;
        }else{
            post("Videogrid: El nombre de files i columnes són inferiors al mímin permès: 1 casella\n",maxdigit);
        }
    }else{
        post("Videogrid: El nombre de files i columnes excedeixen del màxim permès: un total de %d caselles\n",maxdigit);
    }
    
    post("Videogrid: Valors modificats_ nom = %s\n fils = %d \t cols = %d\n", x->x_name->s_name, x->x_num_fil, x->x_num_col);
    /* elimina els nodes no representables amb la nova configuració */
    maxim = x->x_num_fil * x->x_num_col;
    int extret;
    videogrid_erase(x, x->x_glist);
    /* si hi ha més nodes a la cua que el maxim */
    while((numNodes(&x->x_cua)) >  maxim){
        /* desencuem */
        extret = desencuar(&x->x_cua);
    }
    /* al reestablir el tamany del tauler cal saber la posició de l'últim element */
    x->x_ultima_img = numNodes(&x->x_cua) - 1;
    if (x->x_ultima_img <  0) x->x_ultima_img = 0;
    x->x_tauler_primer = x->x_cua.davanter;
    videogrid_drawme(x, x->x_glist, 1);
}

t_widgetbehavior   videogrid_widgetbehavior;

static void videogrid_setwidget(void)
{
    /* post("Entra a setwidget"); */
    videogrid_widgetbehavior.w_getrectfn = videogrid_getrect;
    videogrid_widgetbehavior.w_displacefn = videogrid_displace;
    videogrid_widgetbehavior.w_selectfn = videogrid_select;
    videogrid_widgetbehavior.w_activatefn = NULL;
    videogrid_widgetbehavior.w_deletefn = videogrid_delete;
    videogrid_widgetbehavior.w_visfn = videogrid_vis;
    /* clic del ratoli */
    videogrid_widgetbehavior.w_clickfn = videogrid_click;

#if PD_MINOR_VERSION < 37
    videogrid_widgetbehavior.w_savefn = videogrid_save;
    videogrid_widgetbehavior.w_propertiesfn = videogrid_properties;
#endif

}

/* el constructor de la classe*/
static void *videogrid_new(t_symbol* name, int argc, t_atom *argv)
{
    /* instanciació del nou objecte */
    t_videogrid *x = (t_videogrid *)pd_new(videogrid_class);
    /* crea una sortida per l'objecte*/
    x->x_sortida = outlet_new(&x->x_obj,&s_symbol);
    /* s'obté el canvas de pd */
    x->x_glist = (t_glist*) canvas_getcurrent();
    
    /* posició en el tauler de la última imatge afegida */
    x->x_ultima_img = 0;
    /* posició de l'últim fitxer del directori encuat */
    x->x_dir_pos = 0;
    /* apuntador al primer element en el tauler */
    x->x_tauler_primer = NULL;
    /* fixa el nom de l'objecte */
    char nom[15];
    sprintf(nom, "videogrid%d", ++videogridcount);
    name = gensym(nom);
    x->x_name = name;
    /* amb aquest nom es prepara per poder rebre dades */
    pd_bind(&x->x_obj.ob_pd, x->x_name);
    /* crea la cua de nodes */
    crearCua(&x->x_cua);
    post("NOU videogrid: s'han entrat %d arguments\n", argc);
     
    if (argc != 0)
    {
        post("NOU videogrid: s'obre un objecte existent o amb arguments definits\n"); 
        /* x->x_name */
        x->x_num_fil = (int)atom_getintarg(1, argc, argv);
        x->x_num_col = (int)atom_getintarg(2, argc, argv);
        x->x_color_fons = argv[3].a_w.w_symbol;
        x->x_color_marc = argv[4].a_w.w_symbol;
        post("!!!NOU videogrid: s'han entrat %d arguments\n", argc);
        if(argc == 6){
            /* llegir la cadena de paths | afegir els paths a la cua */
            char *cadenaPaths;
            cadenaPaths = (char *)malloc(51200*sizeof(char));
            strcpy(cadenaPaths,(char *)argv[5].a_w.w_symbol->s_name);
            /* printf("Es carreguen els paths %s --- %s **** %s\n", cadenaPaths, argv[5].a_w.w_symbol->s_name,argv[3].a_w.w_symbol->s_name); */
            /* split */
            
            char *token;
            t_symbol *tt;
            for ( token = strtok(argv[5].a_w.w_symbol->s_name,"|");
                  token != NULL;
                  token = strtok(NULL,"|") ){
                      tt = gensym(token);
            
                      /* printf("AFEGINT CARREGANT %s\n",tt->s_name); */
                      /* videogrid_putvideo(x,tt); */
                      /* DE MOMENT NO AFEGEIX ELS PATHS */
                      /* videogrid_afegir_imatge(x,tt->s_name); */
            }
            /*
            token = strtok(cadenaPaths,"|");
            while(token){
                tt = gensym(token);
                printf("AFEGINT CARREGANT %s\n",tt->s_name);
                videogrid_putvideo(x,tt);
                token = strtok(NULL,"|");
            }
            */
            free(cadenaPaths);
        }
    }else{
        /* crea un objecte nou */
        post("NOU videogrid: es crea un objecte nou\n");
        /* fixa el nombre de files */
        x->x_num_fil = 3;
        /* fixa el nombre de columnes */
        x->x_num_col = 5;
        
        /* colors de fons i de marc*/
        x->x_color_fons = gensym("#F0F0F0");
        x->x_color_marc = gensym("#0F0F0F");
    }
    /* printf("S'ha instanciat un videogrid anomenat %s amb les caracteristiques seguents:",x->x_name->s_name);
    printf("Nombre de files %d - Nombre de columnes: %d", x->x_num_fil, x->x_num_col); */

    return (x);
}

static void videogrid_destroy(t_videogrid *x){
    /* elimina el contingut de la cua */
    eliminarCua(&x->x_cua);
    post("Videogrid eliminat");
}

/* generacio d'una nova classe */
/* al carregar la nova llibreria my_lib pd intenta cridar la funció my_lib_setup */  
/* aquesta crea la nova classe i les seves propietats només un sol cop */

void videogrid_setup(void)
{
    /* post("Entra a setup per generar la classe videogrid"); */
    #include "videogrid.tk2c"
    /*
                     sense pas d'arguments
    videogrid_class = class_new(gensym("videogrid"),
        (t_newmethod)videogrid_new,
        (t_method)videogrid_destroy, 
        sizeof(t_videogrid),
        CLASS_DEFAULT,
        A_DEFSYM,
        0);
                    amb pas d'arguments:
    */
    videogrid_class = class_new(gensym("videogrid"),
        (t_newmethod)videogrid_new,
        (t_method)videogrid_destroy,
        sizeof(t_videogrid),
        CLASS_DEFAULT,
        A_GIMME,
        0);
    /*  class_new crea la nova classe retornant un punter al seu prototipus,
        el primer argument es el nom simbolic de la classe,
        el segon i tercer corresponen al constructor i destructor de la classe respectivament,
        (el constructor instancia un objecte i inicialitza les seves dades cada cop que es crea un objecte
        el destructor allibera la memoria reservada al destruid l'objecte per qualsevol causa)
        el quart correspon a la mida de l'estructura de dades, per tal de poder reservar la memoria necessària,
        el cinquè influeix en el mòde de representació gràfica del objectes. Per defecte CLASS_DEFAULT o ``0',
        la resta d'arguments defineixen els arguments de l'objecte i el seu tipus, la llista acaba  amb 0 
    */

    /* afegeix el mètode helloworld_bang a la classe helloworld_class */
    class_addbang(videogrid_class, videogrid_bang);

    /* afegeix el mètode videogrid_putvideo a la classe videogrid per a entrades de missatge 
        que inicien amb putvideo i una cadena string com a argument */	
    class_addmethod(videogrid_class,(t_method)videogrid_putvideo,gensym("putvideo"), A_DEFSYMBOL, 0);

    /*  afegeix el mètode videogrid_putvideodir a la classe videogrid per a entrades de missatge 
        que inicien amb putvideodir i una cadena string com a argument */	
    class_addmethod(videogrid_class,(t_method)videogrid_putvideodir,gensym("putvideodir"), A_DEFSYMBOL, 0);
    /* afegeix un metode per a modificar el valor de les propietats de l'objecte */
    class_addmethod(videogrid_class, (t_method)videogrid_dialog, gensym("dialog"), A_GIMME, 0);
    /* afegeix un metode per l'obtencio de la posicio del clic del ratolí */
    class_addmethod(videogrid_class, (t_method)videogrid_click, gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    /* inicia el comportament de videogrid */
    videogrid_setwidget();

#if PD_MINOR_VERSION >= 37
    class_setsavefn(videogrid_class,&videogrid_save);
    class_setpropertiesfn(videogrid_class, videogrid_properties);
#endif

    /* afegeix el mètode videogrid_widgetbehavior al la classe videogrid per a la creació de l'element visual */
    class_setwidget(videogrid_class,&videogrid_widgetbehavior);
    class_sethelpsymbol(videogrid_class, gensym("videogrid.pd"));
}
