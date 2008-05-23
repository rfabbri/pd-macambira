
/*
imagegrid external for Puredata
Sergi Lario 	:: slario-at-gmail.com

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
/*#include "cua.h"*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* nombre de caracters per el nom del path del fitxer */
#define BYTESNOMFITXER 512

typedef char path[BYTESNOMFITXER];

/* estructures i tipus de dades de la cua */

/* estructura de dades: un node de la cua */
struct node
{
    /* nom del path de la imatge */
    path pathFitxer;
    /* apuntador al següent node en cua */
    struct node *seguent;
};

/* definició del tipus node */
typedef struct node Node;

/* definició del tipus de cua */
typedef struct
{
    Node *davanter;
    Node *final;
}Cua;


/* declaracions de les funcions */

/* crea una cua */
void crearCua(Cua *cua);
/* encuara un element al final de la cua */
void encuar (Cua *cua, path x);
/* elimina un element de la cua */
int desencuar (Cua *cua);
/* retorna si la cua és buida */
int cuaBuida(Cua *cua);
/* elimina el contingut de la cua */
void eliminarCua(Cua *cua);
/* retorna el nombre de nodes de la cua */
int numNodes(Cua *cua);
/* escriu el contingut de la cua */
void escriuCua(Cua *cua);

/* funcions cua */
/* implementació de les funcions */
void crearCua(Cua *cua)
{
    cua->davanter=cua->final=NULL;
}

/* funció que encua el node al final de la cua */
void encuar (Cua *cua, path x)
{
    Node *nou;
    nou=(Node*)malloc(sizeof(Node));
    strcpy(nou->pathFitxer,x);
    nou->seguent=NULL;
    if(cuaBuida(cua))
    {
        cua->davanter=nou;
    }
    else
        cua->final->seguent=nou;
    cua->final=nou;
}

/* elimina l'element del principi de la cua */
int desencuar (Cua *cua)
{
    if(!cuaBuida(cua))
    {
        Node *nou;
        nou=cua->davanter;
        cua->davanter=cua->davanter->seguent;
        free(nou);
        return 1;
    }
    else
    {
    	/* printf("Cua buida\a\n"); */
    	return 0;
    }
       
}

/* funció que retorna si la cua és buida */
int cuaBuida(Cua *cua)
{
    return (cua->davanter==NULL);
}

/* elimina el contingut de la cua */
void eliminarCua(Cua *cua)
{
    while (!cuaBuida(cua)) desencuar(cua); 
    printf("Cua eliminada\n");
}

/* funció que retorna el nombre de nodes de la cua */
int numNodes(Cua *cua)
{
    int contador=0;
    Node *actual;
    actual=cua->davanter;
    if(actual) contador=1;
    while((actual)&&(actual != cua->final)){
         contador ++;
         actual = actual->seguent;
    }
    return (contador);
}

/* funció que escriu la cua de nodes per la sortida estàndard */
void escriuCua(Cua *cua)
{
    if(!cuaBuida(cua))
    {
        Node *actual;
        actual=cua->davanter;
        printf("CUA DE NODES\n[");
        do{
            printf("#%s#",actual->pathFitxer);
            actual = actual->seguent;
        }while(actual);
        printf("]\n");
        
    }
    else
        printf("Cua buida\n");
}


/* incloure estructures de dades i capceleres de funcions per convertir imatges a diferents formats */
/*#include "magickconverter.h"*/

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

/* funcions per convertir */


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

/* incloure estructures de dades i capceleres de funcions per traballar amb threads */
#include "pthread.h"

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&& IMAGEGRID &&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

/* definició de l'amplada i l'alçada d'una casella */
#define W_CELL 60
#define H_CELL 40

/* definició del gruix en pixels del marc del tauler */
#define GRUIX 2

/* crear un apuntador al nou objecte */
static t_class *imagegrid_class;
/* indica el nombre de imagegrid creats - utilitzat per diferenciar el nom d'instàncies d'objectes del mateix tipus */
static int imagegridcount = 0;

/* definició de la classe i la seva estructura de dades */

typedef struct _imagegrid {
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
    /* v 0.2 -- posicó de la cel·la seleccionada */
    int x_pos_selected;
    /* v 0.2 -- color de seleccio */
    t_symbol *x_color_grasp;

} t_imagegrid;


void load_tkprocs(void){
	// ########### procediments per imagegrid -- slario(at)gmail.com [a partir del codi del grid de l'Ives: ydegoyon(at)free.fr] #########
	sys_gui("proc imagegrid_apply {id} {\n");
	// strip "." from the TK id to make a variable name suffix
	sys_gui("set vid [string trimleft $id .]\n");
	// for each variable, make a local variable to hold its name...
	sys_gui("set var_graph_name [concat graph_name_$vid]\n");
	sys_gui("global $var_graph_name\n");
	sys_gui("set var_graph_num_fil [concat graph_num_fil_$vid]\n");
	sys_gui("global $var_graph_num_fil\n");
	sys_gui("set var_graph_num_col [concat graph_num_col_$vid]\n");
	sys_gui("global $var_graph_num_col\n");
	sys_gui("set var_graph_color_fons [concat graph_color_fons_$vid]\n");
	sys_gui("global $var_graph_color_fons\n");
	sys_gui("set var_graph_color_marc [concat graph_color_marc_$vid]\n");
	sys_gui("global $var_graph_color_marc\n");
	sys_gui("set var_graph_color_grasp [concat graph_color_grasp_$vid]\n");
	sys_gui("global $var_graph_color_grasp\n");
	sys_gui("set cmd [concat $id dialog [eval concat $$var_graph_name] [eval concat $$var_graph_num_fil] [eval concat $$var_graph_num_col] [eval concat $$var_graph_color_fons] [eval concat $$var_graph_color_marc] [eval concat $$var_graph_color_grasp] \\;]\n");
	// puts stderr $cmd
	sys_gui("pd $cmd\n");
	sys_gui("}\n");
	sys_gui("proc imagegrid_cancel {id} {\n");
	sys_gui("set cmd [concat $id cancel \\;]\n");
	// puts stderr $cmd
	sys_gui("pd $cmd\n");
	sys_gui("}\n");
	sys_gui("proc imagegrid_ok {id} {\n");
	sys_gui("imagegrid_apply $id\n");
	sys_gui("imagegrid_cancel $id\n");
	sys_gui("}\n");
	sys_gui("proc pdtk_imagegrid_dialog {id name num_fil num_col color_fons color_marc color_grasp} {\n");
	sys_gui("set vid [string trimleft $id .]\n");
	sys_gui("set var_graph_name [concat graph_name_$vid]\n");
	sys_gui("global $var_graph_name\n");
	sys_gui("set var_graph_num_fil [concat graph_num_fil_$vid]\n");
	sys_gui("global $var_graph_num_fil\n");
	sys_gui("set var_graph_num_col [concat graph_num_col_$vid]\n");
	sys_gui("global $var_graph_num_col\n");
	sys_gui("set var_graph_color_fons [concat graph_color_fons_$vid]\n");
	sys_gui("global $var_graph_color_fons\n");
	sys_gui("set var_graph_color_marc [concat graph_color_marc_$vid]\n");
	sys_gui("global $var_graph_color_marc\n");
	sys_gui("set var_graph_color_grasp [concat graph_color_grasp_$vid]\n");
	sys_gui("global $var_graph_color_grasp\n");
	sys_gui("set $var_graph_name $name\n");
	sys_gui("set $var_graph_num_fil $num_fil\n");
	sys_gui("set $var_graph_num_col $num_col\n");
	sys_gui("set $var_graph_color_fons $color_fons\n");
	sys_gui("set $var_graph_color_marc $color_marc\n");
	sys_gui("set $var_graph_color_grasp $color_grasp\n");
	sys_gui("toplevel $id\n");
	sys_gui("wm title $id {imagegrid}\n");
	sys_gui("wm protocol $id WM_DELETE_WINDOW [concat imagegrid_cancel $id]\n");
	sys_gui("label $id.label -text {IMAGEGRID PROPERTIES}\n");
	sys_gui("pack $id.label -side top\n");
	sys_gui("frame $id.buttonframe\n");
	sys_gui("pack $id.buttonframe -side bottom -fill x -pady 2m\n");
	sys_gui("button $id.buttonframe.cancel -text {Cancel} -command \"imagegrid_cancel $id\"\n");
	sys_gui("button $id.buttonframe.apply -text {Apply} -command \"imagegrid_apply $id\"\n");
	sys_gui("button $id.buttonframe.ok -text {OK} -command \"imagegrid_ok $id\"\n");
	sys_gui("pack $id.buttonframe.cancel -side left -expand 1\n");
	sys_gui("pack $id.buttonframe.apply -side left -expand 1\n");
	sys_gui("pack $id.buttonframe.ok -side left -expand 1\n");
	sys_gui("frame $id.1rangef\n");
	sys_gui("pack $id.1rangef -side top\n");
	sys_gui("label $id.1rangef.lname -text \"Nom :\"\n");
	sys_gui("entry $id.1rangef.name -textvariable $var_graph_name -width 7\n");
	sys_gui("pack $id.1rangef.lname $id.1rangef.name -side left\n");
	sys_gui("frame $id.2rangef\n");
	sys_gui("pack $id.2rangef -side top\n");
	sys_gui("label $id.2rangef.lnum_fil -text \"Fils :\"\n");
	sys_gui("entry $id.2rangef.num_fil -textvariable $var_graph_num_fil -width 7\n");
	sys_gui("pack $id.2rangef.lnum_fil $id.2rangef.num_fil -side left\n");
	sys_gui("frame $id.3rangef\n");
	sys_gui("pack $id.3rangef -side top\n");
	sys_gui("label $id.3rangef.lnum_col -text \"Cols :\"\n");
	sys_gui("entry $id.3rangef.num_col -textvariable $var_graph_num_col -width 7\n");
	sys_gui("pack $id.3rangef.lnum_col $id.3rangef.num_col -side left\n");
	sys_gui("frame $id.4rangef\n");
	sys_gui("pack $id.4rangef -side top\n");
	sys_gui("label $id.4rangef.lcolor_fons -text \"Color fons :\"\n");
	sys_gui("entry $id.4rangef.color_fons -textvariable $var_graph_color_fons -width 7\n");
	sys_gui("pack $id.4rangef.lcolor_fons $id.4rangef.color_fons -side left\n");
	sys_gui("frame $id.5rangef\n");
	sys_gui("pack $id.5rangef -side top\n");
	sys_gui("label $id.5rangef.lcolor_marc -text \"Color marc :\"\n");
	sys_gui("entry $id.5rangef.color_marc -textvariable $var_graph_color_marc -width 7\n");
	sys_gui("pack $id.5rangef.lcolor_marc $id.5rangef.color_marc -side left\n");
	sys_gui("frame $id.6rangef\n");
	sys_gui("pack $id.6rangef -side top\n");
	sys_gui("label $id.6rangef.lcolor_grasp -text \"Color grasp :\"\n");
	sys_gui("entry $id.6rangef.color_grasp -textvariable $var_graph_color_grasp -width 7\n");
	sys_gui("pack $id.6rangef.lcolor_grasp $id.6rangef.color_grasp -side left\n");
	sys_gui("bind $id.1rangef.name <KeyPress-Return> [concat imagegrid_ok $id]\n");
	sys_gui("bind $id.2rangef.num_fil <KeyPress-Return> [concat imagegrid_ok $id]\n");
	sys_gui("bind $id.3rangef.num_col <KeyPress-Return> [concat imagegrid_ok $id]\n");
	sys_gui("bind $id.4rangef.color_fons <KeyPress-Return> [concat imagegrid_ok $id]\n");
	sys_gui("bind $id.5rangef.color_marc <KeyPress-Return> [concat imagegrid_ok $id]\n");
	sys_gui("bind $id.6rangef.color_grasp <KeyPress-Return> [concat imagegrid_ok $id]\n");
	sys_gui("focus $id.1rangef.name\n");
	sys_gui("}\n");
	sys_gui("proc table {w content args} {\n");
	sys_gui("frame $w -bg black\n");
	sys_gui("set r 0\n");
	sys_gui("foreach row $content {\n");
	sys_gui("set fields {}\n");
	sys_gui("set c 0\n");
	sys_gui("foreach col $row {\n");
	// lappend fields [label $w.$r/$c -text $col]
	sys_gui("set img [image create photo -file $col]\n");
	sys_gui("lappend fields [label $w.$r/$c -image $img]\n");
	sys_gui("incr c\n");
	sys_gui("}\n");
	sys_gui("eval grid $fields -sticky news -padx 1 -pady 1\n");
	sys_gui("incr r\n");
	sys_gui("}\n");
	sys_gui("set w\n");
	sys_gui("}\n");
	sys_gui("proc pdtk_imagegrid_table {id name num_fil num_col} {\n");
	sys_gui("table .tauler {\n");
	sys_gui("{sll80x60.gif 3160x120.gif sll80x60.gif}\n");
	sys_gui("{sll80x60.gif sll80x60.gif sll80x60.gif}\n");
	sys_gui("{sll80x60.ppm sll80x60.gif 3160x120.gif}\n");
	sys_gui("}\n");
	sys_gui("pack .tauler\n");
	sys_gui("}\n");

}

/* calcula la posició x del tauler a partir de la posició de l'element de la cua (d'esquerra a dreta) */
int getX(t_imagegrid* x, int posCua){
    int c = x->x_num_col;
    int xpos = (posCua % c) * W_CELL + ((posCua % c) + 1) * GRUIX;
    return(xpos + 1);
}

/* calcula la posició y del tauler a partir de la posició de l'element de la cua (de dalt a baix) */
int getY(t_imagegrid* x, int posCua){
    int c = x->x_num_col;
    int ypos = (posCua / c) * H_CELL + ((posCua / c) + 1) * GRUIX;
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
    /* post("Imagegrid: Imatges temporals eliminades\n",path_total); */
}

int format_adequat(path nomF){
    int retorn = 0;
    path  ig_path = "";
    strcat(ig_path,nomF);
    char *t1;
    path extensio = "";
    for ( t1 = strtok(ig_path,".");
          t1 != NULL;
          t1 = strtok(NULL,".") )
        strcpy(extensio,t1);
    if(strcmp(extensio,"bmp")==0) retorn = 1;
    if(strcmp(extensio,"eps")==0) retorn = 1;
    if(strcmp(extensio,"gif")==0) retorn = 1;
    if(strcmp(extensio,"jpg")==0) retorn = 1;
    if(strcmp(extensio,"jpeg")==0) retorn = 1;
    if(strcmp(extensio,"png")==0) retorn = 1;
    if(strcmp(extensio,"ppm")==0) retorn = 1;
    if(strcmp(extensio,"tif")==0) retorn = 1;
    if(strcmp(extensio,"tiff")==0) retorn = 1;

    return (retorn);
}

/* afegir una imatge al grid */
void imagegrid_afegir_imatge(t_imagegrid *x, path entrada)
{
    int maxim;
    char nNstr[BYTES_NUM_TEMP];
    int pos = 0;
    /* escriu l'argument entrat */
    if (format_adequat(entrada) == 1){
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
        ImageMagick per les conversions
        */
        int nN = x->x_ultima_img;
        convertir(entrada,FORMAT_MINIATURA, W_CELL, H_CELL, nN);
        sprintf(nNstr, "%d", nN);
        strcat(ig_path,nNstr);
        strcat(ig_path,".");
        strcat(ig_path,FORMAT_MINIATURA);
        /* post("Creacio de la imatge %s ...",ig_path); */
        sys_vgui("image create photo img%x%d -file %s\n",x,nN,ig_path);
        sys_vgui(".x%x.c create image %d %d -image img%x%d -tags %xS%d\n",
             glist_getcanvas(x->x_glist),
             text_xpix(&x->x_obj, x->x_glist) + getX(x,nN) + (W_CELL/2),
             text_ypix(&x->x_obj, x->x_glist) + getY(x,nN) + (H_CELL/2),
             x,nN,x,nN);
        if(nN == 0){
            x->x_tauler_primer = x->x_cua.final;
            /* post("Ara el primer del tauler es %s\n",x->x_tauler_primer->pathFitxer); */
        }
        /* printf("SURT de la creacio de la imatge %s ...",ig_path); */
    }else{
        post("Imagegrid: Incompatible file format (%s).\n",entrada);
    }
    /*
    sys_vgui("image create photo img%x -file %s\n",x,entrada);
    sys_vgui(".x%x.c create image %d %d -image img%x -tags %xS\n", 
    glist_getcanvas(glist),text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x,x);
    */
}

/* dibuixa imagegrid */
void imagegrid_drawme(t_imagegrid *x, t_glist *glist, int firsttime)
{
    /* post("Entra a drawme amb firsttime: %d", firsttime); */
    if (firsttime) {
        char name[MAXPDSTRING];
        canvas_makefilename(glist_getcanvas(x->x_glist), x->x_name->s_name, name, MAXPDSTRING);
        sys_vgui(".x%x.c create rectangle %d %d %d %d -fill %s -tags %xGRID -outline %s\n",
            glist_getcanvas(glist),
            text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
            text_xpix(&x->x_obj, glist) + (x->x_num_col * W_CELL) + 1 + (x->x_num_col * GRUIX) + GRUIX, text_ypix(&x->x_obj, glist) + (x->x_num_fil * H_CELL) + 1 + (x->x_num_fil * GRUIX) + GRUIX,
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
                // imagegrid_afegir_imatge(x,actual->pathFitxer);
                convertir(actual->pathFitxer,FORMAT_MINIATURA, W_CELL, H_CELL, nN);
                sys_vgui("image create photo img%x%d -file %s\n",x,nN,ig_path);
                sys_vgui(".x%x.c create image %d %d -image img%x%d -tags %xS%d\n", 
                         glist_getcanvas(x->x_glist),text_xpix(&x->x_obj, x->x_glist) + getX(x,nN) + (W_CELL/2), text_ypix(&x->x_obj, x->x_glist) + getY(x,nN) + (H_CELL/2),x,nN,x,nN);
                actual = actual->seguent;
                nN++;
            }while(actual);
        }		
    }
    else { 
        sys_vgui(".x%x.c coords %xGRID %d %d %d %d\n", glist_getcanvas(glist), x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),text_xpix(&x->x_obj, glist) + (x->x_num_col*W_CELL) + 1 + (x->x_num_col * GRUIX) + GRUIX, text_ypix(&x->x_obj, glist) + (x->x_num_fil*H_CELL) + 1 + (x->x_num_fil * GRUIX) + GRUIX);
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
            sprintf(buf, "pdtk_imagegrid_table %%s %s %d %d\n", x->x_name->s_name, x->x_num_fil, x->x_num_col);
            gfxstub_new(&x->x_obj.ob_pd, x, buf); */
        }
        if (x->x_pos_selected > -1){
        sys_vgui(".x%x.c coords %xGRASP %d %d %d %d\n", glist_getcanvas(glist), x, 
        text_xpix(&x->x_obj, x->x_glist) + getX(x,x->x_pos_selected), text_ypix(&x->x_obj, x->x_glist) + getY(x,x->x_pos_selected),
        text_xpix(&x->x_obj, x->x_glist) + getX(x,x->x_pos_selected) + W_CELL, text_ypix(&x->x_obj, x->x_glist) + getY(x,x->x_pos_selected) + H_CELL);
	}
	sys_vgui(".x%x.c delete %xLINIA\n", glist_getcanvas(x->x_glist), x);
    }

    int xI = text_xpix(&x->x_obj, glist);
    int yI = text_ypix(&x->x_obj, glist);
    int xF = xI + (x->x_num_col * W_CELL) + ((x->x_num_col + 1) * GRUIX);
    int yF = yI + (x->x_num_fil * H_CELL) + ((x->x_num_fil + 1) * GRUIX);
    int vlines = 0;
    int xi = 0;
    while(vlines < x->x_num_col){
	xi = xI + getX(x,vlines) - GRUIX + 1;
	sys_vgui(".x%x.c create line %d %d %d %d -fill %s -width %d -tag %xLINIA\n", glist_getcanvas(x->x_glist), xi, yI, xi, yF, x->x_color_marc->s_name,GRUIX,x);
	vlines++;
    }
    xi = xi + W_CELL + GRUIX;
    sys_vgui(".x%x.c create line %d %d %d %d -fill %s -width %d -tag %xLINIA\n", glist_getcanvas(x->x_glist), xi, yI, xi, yF, x->x_color_marc->s_name,GRUIX,x);
    int hlines = 0;
    int yi = 0;
    while(hlines < x->x_num_fil){
	yi = yI + ((H_CELL + GRUIX) * hlines) + 2;
	sys_vgui(".x%x.c create line %d %d %d %d -fill %s -width %d -tag %xLINIA\n", glist_getcanvas(x->x_glist), xI, yi, xF, yi, x->x_color_marc->s_name,GRUIX,x);
	hlines++;
    }
    yi = yi + H_CELL + GRUIX;
    sys_vgui(".x%x.c create line %d %d %d %d -fill %s -width %d -tag %xLINIA\n", glist_getcanvas(x->x_glist), xI, yi, xF, yi, x->x_color_marc->s_name,GRUIX,x);
}

/* borra imagegrid v 0.2 -- int toclear */
void imagegrid_erase(t_imagegrid* x,t_glist* glist, int toclear)
{
    int maxim = x->x_num_fil * x->x_num_col;
    path path_total;
    char contador_str[BYTES_NUM_TEMP];
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

    /* elimina el grid v 0.2 -- excepte quan es fa un clear */
    if(toclear == 0){
    	sys_vgui(".x%x.c delete %xGRID\n", glist_getcanvas(glist), x);
	sys_vgui(".x%x.c delete %xLINIA\n", glist_getcanvas(x->x_glist), x);
    }
    /* v 0.2 -- elimina el marc de la casella seleccionada */
    if(x->x_pos_selected > -1){	
	sys_vgui(".x%x.c delete %xGRASP\n", glist_getcanvas(glist), x);
        x->x_pos_selected = -1;
    }
    eliminar_imatges_temporals(maxim);
}

/* mètode de la clase que escriu un missatge al rebre un bang */
void imagegrid_bang(t_imagegrid *x)
{
    /* post("Hello imagegrid !!"); */
    escriuCua(&x->x_cua);
}

/* mètode de la classe que es dispara al rebre una entrada de missatge amb [putimg +string( com a paràmetre */
void imagegrid_putimg(t_imagegrid *x, t_symbol *entrada)
{
    /* comprova que existeixi el fitxer */
    FILE *fitxer;
    path e;
    strcpy(e,entrada->s_name);
    /* post("putimg de %s\n", e); */
    
    fitxer = fopen(e,"r");
    if (!fitxer) {
        post("Imagegrid: Problem opening file %s.\n",e);
    }
    else {
        /* post("s'encua la imatge %s\n", e); */
        imagegrid_afegir_imatge(x,e);
        /*outlet_symbol(x->x_sortida, entrada);*/
    }
    /* post("putimg amb img = %s\n", e); */
}

/* mètode de la classe que es dispara al rebre una entrada de missatge amb [putimgdir +string( com a paràmetre */
void *imagegrid_putimgdir_thread(t_imagegrid *x)
{	
    DIR *dirp;
    struct dirent * direntp;
    path nomImatge, directoriAnterior, pathActual;
    int numEncuats = 0, numPosDir = 0;
    int maxim;
    if ((dirp = opendir(x->x_dir_canvi)) == NULL)
    {
        post("Imagegrid: Can not open folder %s.\n", x->x_dir_canvi);
    }else{
        maxim = x->x_num_fil * x->x_num_col;
        strcpy(directoriAnterior, x->x_dir_actual);
        strcpy(x->x_dir_actual, x->x_dir_canvi);
        /* si es el mateix directori entrat l'ultim busca la ultima imatge afegida per a seguir a encuant a partir d'ella en endavant */
        if(strcmp(directoriAnterior, x->x_dir_actual) == 0){
            /* post("Imagegrid: Repeteix directori %s\n", x->x_dir_actual); */
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
                            imagegrid_afegir_imatge(x, pathActual);
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
            /* post("Imagegrid: Nou directori %s \n", x->x_dir_actual); */
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
                        imagegrid_afegir_imatge(x, pathActual);
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

void imagegrid_putimgdir(t_imagegrid *x, t_symbol *entrada)
{

    pthread_t unthread;
    pthread_attr_t unatribut;
    pthread_attr_init( &unatribut );
    
    strcpy(x->x_dir_canvi,entrada->s_name);
     
    // ----------------      THREAD CREAT -------------------------
    pthread_mutex_init(&x->x_lock, NULL);
    pthread_create(&unthread,&unatribut,(void *)imagegrid_putimgdir_thread, x);
    pthread_mutex_destroy(&x->x_lock);
}


/* v 0.2 -- mètode de la classe que desmarca la cel·la seleccionada */
static void imagegrid_ungrasp_selected(t_imagegrid *x)
{
    /* post("Ungrasp selected thumb %d", x->x_pos_selected); */
    if(x->x_pos_selected > -1) {
        sys_vgui(".x%x.c delete %xGRASP\n", glist_getcanvas(x->x_glist), x);
        x->x_pos_selected = -1;
    }
}

/* v 0.2 -- mètode de la classe que marca la cel·la seleccionada */
static void imagegrid_grasp_selected(t_imagegrid *x, int pos)
{
    /*printf("Grasp selected thumb %d", pos);*/
    if(pos != x->x_pos_selected) {
	imagegrid_ungrasp_selected(x);
        /* post("Grasp selected thumb %d", pos); */
        x->x_pos_selected = pos;
        /* nem per aqui ---- */
	sys_vgui(".x%x.c create rectangle %d %d %d %d -fill {} -tags %xGRASP -outline %s -width %d\n",
            glist_getcanvas(x->x_glist),
            text_xpix(&x->x_obj, x->x_glist) + getX(x,x->x_pos_selected), text_ypix(&x->x_obj, x->x_glist) + getY(x,x->x_pos_selected),
            text_xpix(&x->x_obj, x->x_glist) + getX(x,x->x_pos_selected) + W_CELL, text_ypix(&x->x_obj, x->x_glist) + getY(x,x->x_pos_selected) + H_CELL,
            x,x->x_color_grasp->s_name, GRUIX + 1);
        canvas_fixlinesfor(glist_getcanvas(x->x_glist), (t_text*)x);
    }
}

/* v 0.2 -- mètode de la classe que dispara l'element del taules en la posicio N [seek N( */
void imagegrid_seek(t_imagegrid *x, t_floatarg postauler)
{
    /* post("seek a %d\n",postauler); */
    path pathSortida;
    Node *actual;
    int contador = 0;
    int maxim = x->x_num_fil * x->x_num_col;
    /* obtenir el path per enviar a la sortida */
    if((!cuaBuida(&x->x_cua))&&(postauler < numNodes(&x->x_cua))&&(postauler >= 0 )){
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
            /* post("Esta a imagegrid_click amb %d %d a la posicio %d\n", x_pos, y_pos, postauler);*/
            /* v 0.2 -- marcar casella */
            imagegrid_grasp_selected(x, postauler);
        }
    }
}

static int imagegrid_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_imagegrid* x = (t_imagegrid *)z;
    int x_pos = xpix - text_xpix(&x->x_obj, x->x_glist);
    int y_pos = ypix - text_ypix(&x->x_obj, x->x_glist);
    int xa, ya, postauler;
    if (doit) 
    {
        /* obtenir la posicio en el tauler */ 
	// -- v 0.2 -- midoficacio pel gruix del marc //
	xa = ((x_pos) / (W_CELL + GRUIX + 1));
        ya = ((y_pos) / (H_CELL + GRUIX + 1)) * x->x_num_col;
        postauler = ya + xa;	
	// -- v 0.2 -- seleciona la casella disparant el path //
        imagegrid_seek(x, postauler);
    }
    return (1);
}

/* v 0.2 -- mètode de la classe que buida el tauler amb el missatge [clear ( */
void imagegrid_clear(t_imagegrid *x)
{
    imagegrid_erase(x, x->x_glist,1);
    eliminarCua(&x->x_cua);
    x->x_ultima_img = 0;
    x->x_dir_pos = 0;
    x->x_tauler_primer = NULL;
    imagegrid_drawme(x, x->x_glist, 0);
}


static void imagegrid_getrect(t_gobj *z, t_glist *glist,int *xp1, int *yp1, int *xp2, int *yp2)
{
    int cols, fils;
    t_imagegrid* x = (t_imagegrid*)z;
    cols = x->x_num_col;
    fils = x->x_num_fil;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + (cols*W_CELL) + ((cols + 1) * GRUIX);
    *yp2 = text_ypix(&x->x_obj, glist) + (fils*H_CELL) + ((fils + 1) * GRUIX);
    /* post("Esta amb el ratoli en el punt %d %d %d %d o son els vetexs de la caixa... es/bd", xp1, yp1, xp2, yp2); */
}

static void imagegrid_displace(t_gobj *z, t_glist *glist,int dx, int dy)
{
    /* post("Entra a displace amb dx %d i dy %d", dx, dy); */
    t_imagegrid *x = (t_imagegrid *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    sys_vgui(".x%x.c coords %xGRID %d %d %d %d\n",
             glist_getcanvas(glist), x,
             text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
             text_xpix(&x->x_obj, glist) + (x->x_num_col*W_CELL) + 1 + (x->x_num_col * GRUIX) + GRUIX, text_ypix(&x->x_obj, glist) + (x->x_num_fil*H_CELL) + 1 + (x->x_num_fil * GRUIX) + GRUIX);
    imagegrid_drawme(x, glist, 0);
    canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
}

static void imagegrid_select(t_gobj *z, t_glist *glist, int state)
{
    /* post("Entra select amb state %d", state); */
    t_imagegrid *x = (t_imagegrid *)z;
    if (state) {
        /* post("Imagegrid seleccionat"); */
        sys_vgui(".x%x.c itemconfigure %xGRID -outline #0000FF\n", glist_getcanvas(glist), x);
    }
    else {
        /* post("Imagegrid deseleccionat"); */
        sys_vgui(".x%x.c itemconfigure %xGRID -outline %s\n", glist_getcanvas(glist), x, x->x_color_marc->s_name);
    }
}

static void imagegrid_delete(t_gobj *z, t_glist *glist)
{
    /* post("Entra a delete"); */
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

static void imagegrid_vis(t_gobj *z, t_glist *glist, int vis)
{
    /* post("Entra a vist amb vis %d", vis); */
    t_imagegrid* s = (t_imagegrid*)z;
    if (vis)
        imagegrid_drawme(s, glist, 1);
    else
       imagegrid_erase(s,glist,0);
}


static void imagegrid_save(t_gobj *z, t_binbuf *b)
{
    /* post("Entra a save"); */
    t_imagegrid *x = (t_imagegrid *)z;
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
    x->x_name,x->x_num_fil,x->x_num_col,x->x_color_fons,x->x_color_marc,x->x_color_grasp,gensym(cadenaPathsInicials));
    binbuf_addv(b, ";");
}

static void imagegrid_properties(t_gobj *z, t_glist *owner)
{
    char buf[900];
    t_imagegrid *x=(t_imagegrid *)z;

    /* post("Es crida a pdtk_imagegrid dialog passant nom = %s\n fils = %d \t cols = %d \t color fons = %s \t color marc = %s\n", x->x_name->s_name, x->x_num_fil, x->x_num_col, x->x_color_fons->s_name, x->x_color_marc->s_name); */
    sprintf(buf, "pdtk_imagegrid_dialog %%s %s %d %d %s %s %s\n",
            x->x_name->s_name, x->x_num_fil, x->x_num_col, x->x_color_fons->s_name, x->x_color_marc->s_name, x->x_color_grasp->s_name);
    /* post("imagegrid_properties : %s", buf ); */
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void imagegrid_dialog(t_imagegrid *x, t_symbol *s, int argc, t_atom *argv)
{
    int maxim, maxdigit;
    int nfil = 0;
    int ncol = 0;
    if ( !x ) {
        post("Imagegrid: error_ Attempt to alter the properties of an object that does not exist.\n");
    }
    switch (argc) {
	case 3:
		/* versio inicial */
		if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT) 
    		{
        		post("Imagegrid: error_ Some of the values are inconsistent in its data type.\n");
        		return;
    		}
		x->x_name = argv[0].a_w.w_symbol;
    		nfil = (int)argv[1].a_w.w_float;
    		ncol = (int)argv[2].a_w.w_float;
	break;
	case 5:
		/* versio 0.1 */
		if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT || argv[3].a_type != A_SYMBOL || argv[4].a_type != A_SYMBOL) 
    		{
        		post("Imagegrid: error_ Some of the values are inconsistent in its data type.\n");
        		return;
    		}
		x->x_name = argv[0].a_w.w_symbol;
    		nfil = (int)argv[1].a_w.w_float;
    		ncol = (int)argv[2].a_w.w_float;
    		x->x_color_fons = argv[3].a_w.w_symbol;
    		x->x_color_marc = argv[4].a_w.w_symbol;
	break;
	case 6:
		/* versio 0.2 */
		if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT || argv[3].a_type != A_SYMBOL || argv[4].a_type != A_SYMBOL || argv[5].a_type != A_SYMBOL) 
    		{
        		post("Imagegrid: error_ Some of the values are inconsistent in its data type.\n");
        		return;
    		}
		x->x_name = argv[0].a_w.w_symbol;
    		nfil = (int)argv[1].a_w.w_float;
    		ncol = (int)argv[2].a_w.w_float;
    		x->x_color_fons = argv[3].a_w.w_symbol;
    		x->x_color_marc = argv[4].a_w.w_symbol;
		x->x_color_grasp = argv[5].a_w.w_symbol;
	break;	
	default:
	    /* no fa res */
	break;
    }
    /* el màxim es fixa pel nombre de digits utilitzats pel nom de la imatge temporal */
    maxdigit = pow(10,BYTES_NUM_TEMP);
    if((nfil*ncol) <= maxdigit){
       	if((nfil*ncol) > 0){
            x->x_num_fil = nfil;
            x->x_num_col = ncol;
        }else{
            post("Imagegrid: The number of rows and columns is less than the minimum allowed: 1 cell.\n",maxdigit);
        }
    }else{
        post("Imagegrid: The number of rows and columns exceeds the maximum allowed: a total of %d cells.\n",maxdigit);
    }
    /* post("Imagegrid: Modified values\n name = %s\n rows = %d \t cols = %d.\n", x->x_name->s_name, x->x_num_fil, x->x_num_col); */
    /* elimina els nodes no representables amb la nova configuració */
    maxim = x->x_num_fil * x->x_num_col;
    int extret;
    imagegrid_erase(x, x->x_glist,0);
    /* si hi ha més nodes a la cua que el maxim */
    while((numNodes(&x->x_cua)) >  maxim){
        /* desencuem */
        extret = desencuar(&x->x_cua);
    }
    /* al reestablir el tamany del tauler cal saber la posició de l'últim element */
    x->x_ultima_img = numNodes(&x->x_cua) - 1;
    if (x->x_ultima_img <  0) x->x_ultima_img = 0;
    x->x_tauler_primer = x->x_cua.davanter;
    imagegrid_drawme(x, x->x_glist, 1);
}

t_widgetbehavior   imagegrid_widgetbehavior;

static void imagegrid_setwidget(void)
{
    /* post("Entra a setwidget"); */
    imagegrid_widgetbehavior.w_getrectfn = imagegrid_getrect;
    imagegrid_widgetbehavior.w_displacefn = imagegrid_displace;
    imagegrid_widgetbehavior.w_selectfn = imagegrid_select;
    imagegrid_widgetbehavior.w_activatefn = NULL;
    imagegrid_widgetbehavior.w_deletefn = imagegrid_delete;
    imagegrid_widgetbehavior.w_visfn = imagegrid_vis;
    /* clic del ratoli */
    imagegrid_widgetbehavior.w_clickfn = imagegrid_click;

#if PD_MINOR_VERSION < 37
    imagegrid_widgetbehavior.w_savefn = imagegrid_save;
    imagegrid_widgetbehavior.w_propertiesfn = imagegrid_properties;
#endif

}

/* el constructor de la classe*/
static void *imagegrid_new(t_symbol* name, int argc, t_atom *argv)
{
    /* instanciació del nou objecte */
    t_imagegrid *x = (t_imagegrid *)pd_new(imagegrid_class);
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
    x->x_pos_selected = -1;
    /* fixa el nom de l'objecte */
    char nom[15];
    sprintf(nom, "imagegrid%d", ++imagegridcount);
    name = gensym(nom);
    x->x_name = name;
    /* amb aquest nom es prepara per poder rebre dades */
    pd_bind(&x->x_obj.ob_pd, x->x_name);
    /* crea la cua de nodes */
    crearCua(&x->x_cua);
    post("NEW imagegrid: created with %d parameters.\n", argc);

    switch (argc) {
	case 3:
		/* versio inicial */
		x->x_num_fil = (int)atom_getintarg(1, argc, argv);
        	x->x_num_col = (int)atom_getintarg(2, argc, argv);
		x->x_color_fons = gensym("#F0F0F0");
            	x->x_color_marc = gensym("#0F0F0F");
            	x->x_color_grasp = gensym("#F1882B");
	break;
	case 5:
		/* versio 0.1 */
		x->x_num_fil = (int)atom_getintarg(1, argc, argv);
        	x->x_num_col = (int)atom_getintarg(2, argc, argv);
        	x->x_color_fons = argv[3].a_w.w_symbol;
        	x->x_color_marc = argv[4].a_w.w_symbol;
		x->x_color_grasp = gensym("#F1882B");
	break;
	case 6:
		/* versio 0.2 */
 		x->x_num_fil = (int)atom_getintarg(1, argc, argv);
        	x->x_num_col = (int)atom_getintarg(2, argc, argv);
        	x->x_color_fons = argv[3].a_w.w_symbol;
        	x->x_color_marc = argv[4].a_w.w_symbol;
        	x->x_color_grasp = argv[5].a_w.w_symbol;
	break;
	case 7:
		/* versio 0.1 - paths dels  elsements del tauler */
 		x->x_num_fil = (int)atom_getintarg(1, argc, argv);
        	x->x_num_col = (int)atom_getintarg(2, argc, argv);
        	x->x_color_fons = argv[3].a_w.w_symbol;
        	x->x_color_marc = argv[4].a_w.w_symbol;
        	x->x_color_grasp = argv[5].a_w.w_symbol;
		/*
		// -- llegir la cadena de paths | afegir els paths a la cua //
		// -- ATENCIO! NO AFEGEIX ELS PATHS !!! //
            	char *cadenaPaths;
            	cadenaPaths = (char *)malloc(51200*sizeof(char));
            	strcpy(cadenaPaths,(char *)argv[6].a_w.w_symbol->s_name);
            	// -- printf("Es carreguen els paths %s --- %s **** %s\n", cadenaPaths, argv[5].a_w.w_symbol->s_name,argv[3].a_w.w_symbol->s_name); //
            	// -- split //
            	char *token;
            	t_symbol *tt;
            	for ( token = strtok(argv[6].a_w.w_symbol->s_name,"|");
                	token != NULL;
                  	token = strtok(NULL,"|") ){
                        	tt = gensym(token);
                      		// -- printf("AFEGINT CARREGANT %s\n",tt->s_name); //
                      		// -- imagegrid_putimg(x,tt); //
                      		// -- ATENCIO! NO AFEGEIX ELS PATHS !!! //
                      		// -- imagegrid_afegir_imatge(x,tt->s_name); //
            	}
            	
            	token = strtok(cadenaPaths,"|");
            	while(token){
                	tt = gensym(token);
                	printf("AFEGINT CARREGANT %s\n",tt->s_name);
                	imagegrid_putimg(x,tt);
                	token = strtok(NULL,"|");
            	}
            	free(cadenaPaths);
            	*/
	break;
		
	default:
	    /* crea un objecte nou per defecte */
            /* post("NEW imagegrid created.\n"); */
            /* fixa el nombre de files */
            x->x_num_fil = 3;
            /* fixa el nombre de columnes */
            x->x_num_col = 5;
            /* colors de fons i de marc*/
            x->x_color_fons = gensym("#F0F0F0");
            x->x_color_marc = gensym("#0F0F0F");
            x->x_color_grasp = gensym("#F1882B");
	break;
    }
    /* printf("S'ha instanciat un imagegrid anomenat %s amb les caracteristiques seguents:",x->x_name->s_name);
    printf("Nombre de files %d - Nombre de columnes: %d", x->x_num_fil, x->x_num_col); */

    return (x);
}

static void imagegrid_destroy(t_imagegrid *x){
    /* elimina el contingut de la cua */
    eliminarCua(&x->x_cua);
    post("Imagegrid destroyed.\n");
}

/* generacio d'una nova classe */
/* al carregar la nova llibreria my_lib pd intenta cridar la funció my_lib_setup */  
/* aquesta crea la nova classe i les seves propietats només un sol cop */

void imagegrid_setup(void)
{
    /* post("Entra a setup per generar la classe imagegrid"); */
    /* #include "imagegrid.tk2c" */
	load_tkprocs();
    /*
                     sense pas d'arguments
    imagegrid_class = class_new(gensym("imagegrid"),
        (t_newmethod)imagegrid_new,
        (t_method)imagegrid_destroy, 
        sizeof(t_imagegrid),
        CLASS_DEFAULT,
        A_DEFSYM,
        0);
                    amb pas d'arguments:
    */
    imagegrid_class = class_new(gensym("imagegrid"),
        (t_newmethod)imagegrid_new,
        (t_method)imagegrid_destroy,
        sizeof(t_imagegrid),
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
    class_addbang(imagegrid_class, imagegrid_bang);

    /* afegeix el mètode imagegrid_putimg a la classe imagegrid per a entrades de missatge 
        que inicien amb putimg i una cadena string com a argument */	
    class_addmethod(imagegrid_class,(t_method)imagegrid_putimg,gensym("putimg"), A_DEFSYMBOL, 0);

    /*  afegeix el mètode imagegrid_putimgdir a la classe imagegrid per a entrades de missatge 
        que inicien amb putimgdir i una cadena string com a argument */	
    class_addmethod(imagegrid_class,(t_method)imagegrid_putimgdir,gensym("putimgdir"), A_DEFSYMBOL, 0);
    /* afegeix un metode per a modificar el valor de les propietats de l'objecte */
    class_addmethod(imagegrid_class, (t_method)imagegrid_dialog, gensym("dialog"), A_GIMME, 0);
    /* afegeix un metode per l'obtencio de la posicio del clic del ratolí */
    class_addmethod(imagegrid_class, (t_method)imagegrid_click, gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    /* v 0.2 -- afegeix un metode netejar el contigut del tauler */
    class_addmethod(imagegrid_class, (t_method)imagegrid_clear, gensym("clear"), 0);
    /* v 0.2 -- afegeix un metode seek #num per disparar */
    class_addmethod(imagegrid_class, (t_method)imagegrid_seek, gensym("seek"), A_FLOAT, 0);
    /* inicia el comportament de imagegrid */
    imagegrid_setwidget();

#if PD_MINOR_VERSION >= 37
    class_setsavefn(imagegrid_class,&imagegrid_save);
    class_setpropertiesfn(imagegrid_class, imagegrid_properties);
#endif

    /* afegeix el mètode imagegrid_widgetbehavior al la classe imagegrid per a la creació de l'element visual */
    class_setwidget(imagegrid_class,&imagegrid_widgetbehavior);
    class_sethelpsymbol(imagegrid_class, gensym("imagegrid.pd"));
}
