#include <stdio.h>
#include <stdlib.h>

    int** parking ;

int main( int argc, char *argv[]) {

    int i,j; //Contadores para los bucles
    //La variable parking simulara las plazas del parking a ellas se accede de la siguiente forma:
    //planta 2 aparcamiento 4 seria parking[2-1][4-1];

    parking = (int**) malloc(atoi(argv[1]) * sizeof (int*));
    for(i=0;i<atoi(argv[1]);i++){
        parking[i]=(int*) malloc(atoi(argv[2])*sizeof (int));
    }

    //Seteamos los valores del parking a 0 al principio comienza vacio
    for(i=0;i<atoi(argv[1]);i++){
        for(j=0;j<atoi(argv[2]);j++){
            parking[i][j]=0;
        }
    }

    return 0;
}
