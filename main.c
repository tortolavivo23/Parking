#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


//La variable parking simulara las plazas del parking a ellas se accede de la siguiente forma:
//planta 2 aparcamiento 4 seria parking[2-1][4-1];
    int** parking ;

//Cantidad de plazas y pisos del programa
    int plazas;
    int pisos;

//Guardara el primer par de sitios libre y las plazas libres que hay
    int plazaslibres;
    int parlibre[2];





void *coche( void *num){
    int coche_id= *(int *)num;




}



int main( int argc, char *argv[]) {

    int i,j; //Contadores para los bucles



    //Cantidad de plazas y pisos del programa
    plazas=atoi(argv[2]);
    pisos=atoi(argv[1]);

    parking = (int**) malloc(pisos * sizeof (int*));
    for(i=0;i<pisos;i++){
        parking[i]=(int*) malloc(plazas*sizeof (int));
    }

    //Seteamos los valores del parking a 0 al principio comienza vacio
    for(i=0;i<pisos;i++){
        for(j=0;j<plazas;j++){
            parking[i][j]=0;
        }
    }

    //Al principio esta vacio el primer el primer par vacio es el 0-1 y todas las plazas estan libres
    plazaslibres=plazas*pisos;
    parlibre[0]=0;
    parlibre[1]=0;

    if(plazas<2){
        //No entran camiones
        parlibre[0]=-1;
        parlibre[1]=-1;
    }

    pthread_mutex_init(&mutex,NULL);

    return 0;
}
