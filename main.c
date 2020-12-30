#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//La variable parking simulara las plazas del parking a ellas se accede de la siguiente forma:
//planta 2 aparcamiento 4 seria parking[2-1][4-1];
    int **parking;

//Cantidad de plazas y pisos del programa
    int plazas;
    int pisos;

//Guardara el primer par de sitios libre y las plazas libres que hay
    int plazaslibres;
    int parlibre[2];

//Mutex para dispositivo de control
    pthread_mutex_t mutex;
//COndicion del mutex
    pthread_cond_t espera[5];

void *coche( void *num){
    //valor booleano util para la busqueda del aprcamiento
    int salida=1;

    //valor para que el coche sepa donde aparca
    int aparcamiento[2];

    int coche_id= *(int *)num;
    printf("Entra coche %i\n",coche_id);
    pthread_mutex_lock(&mutex);
    while (plazaslibres==0){
        pthread_cond_wait(&espera[coche_id], &mutex);
    }

    //Entra en seccion critica
    if(plazaslibres==2){
        plazaslibres--;
        parking[parlibre[0]][parlibre[1]]=coche_id;
        parlibre[0]=-1;
        parlibre[1]=-1;
    }
    else{
        plazaslibres--;
        //Busca sitio para aparcar
        for(int i=0;i<pisos && salida;i++){
            for(int j=0;j<plazas && salida;j++){
                //Sabemos que hay un sitio diferente al que no accedera un camion que estara libre, lo aprovechamos
                if(i!=parlibre[0] || (j!=parlibre[1] && j!=parlibre[1]+1)){
                    parking[i][j]=coche_id;
                    aparcamiento[0]=i;
                    aparcamiento[1]=j;
                    salida=0;
                    printf("Coche aparcado en plaza %i del piso %i\n",j,i);
                }
            }
        }
    }

    pthread_mutex_unlock(&mutex);

    sleep( (rand() % 2) + 1);

    //Entramos en la seccion critica
    pthread_mutex_lock(&mutex);
    printf("Coche saliendo de la plaza %i, del piso %i\n",aparcamiento[1],aparcamiento[0]);
    parking[aparcamiento[0]][aparcamiento[1]]=0;
    plazaslibres++;
    if(aparcamiento[1]!=0){
        if(aparcamiento[1]-1==0){
            parlibre[0]=aparcamiento[0];
            parlibre[1]=aparcamiento[1]-1;
        }
    }
    if(aparcamiento[1]!=plazas-1){
        if(aparcamiento[1]+1==0){
            parlibre[0]=aparcamiento[0];
            parlibre[1]=aparcamiento[1];
        }
    }
    pthread_mutex_unlock(&mutex);
    //Salimos de la seccion critica
}

int main( int argc, char *argv[]) {

    printf("hoal");

    int i,j; //Contadores para los bucles
    pthread_t th;



    //Cantidad de plazas y pisos del programa
    plazas=atoi(argv[2]);
    pisos=atoi(argv[1]);

    printf("1");

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

    printf("2");

    if(plazas<2){
        //No entran camiones
        parlibre[0]=-1;
        parlibre[1]=-1;
    }

    pthread_mutex_init(&mutex,NULL);
    printf("3");
    //prueba con 5 coches
    for(int i=0;i<5;i++){
        pthread_cond_init(&espera[i],NULL);
        pthread_create(&th,NULL,coche,(void*)&i);
    }

    while (1);
    return 0;
}
