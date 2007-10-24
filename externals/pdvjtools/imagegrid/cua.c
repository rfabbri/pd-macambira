/*
cue for imagegrid external
Copyright (C) 2007  Sergi Lario

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

#include "cua.h"

/*
- compilació:
    $ gcc -c -ansi -O -Wall -Wmissing-prototypes cua.c
- muntatge:
    $ gcc cua.o -o cua
- execució:
    $ ./cua
*/
/* programa principal de prova */
/*
int main()
{

    int opc=8;
    char path[BYTESNOMFITXER];
    int ok;
    Cua cua;
    crearCua(&cua);
    
    while(opc!=5)
    {
        printf("\t\t\tMENU PRINCIPAL\n\n\n");
        printf("\t 1. Encuar\n");
        printf("\t 2. Desencuar\n");
        printf("\t 3. Nombre de nodes\n");
        printf("\t 4. Contingut de la cua\n");
        printf("\t 5. Sortir\n");

        scanf("%d", &opc);

        switch(opc)
        {
            case 1:
                printf("path a introduir:\n");
                scanf("%s", path);
                encuar(&cua, path);
                break;

            case 2:
                ok = desencuar(&cua);
                if(ok) printf("node eliminat de la cua\n");
                break;

            case 3:
                printf("nombre de nodes de la cua %d\n", numNodes(&cua));
                break;
            case 4:
                escriuCua(&cua);
                break;
            case 5:
                eliminarCua(&cua);
                break;
        }
    }
    getchar();
    return 0;
}
*/
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
    printf("Cua eliminada\a\n");
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
        printf("Cua buida\a\n");
}
