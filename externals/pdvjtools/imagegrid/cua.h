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
