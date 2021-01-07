#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define COCHES 10
#define CAMIONES 1

//La variable parking simulara las plazas del parking a ellas se accede de la siguiente forma:
//planta 2 aparcamiento 4 seria parking[2-1][4-1];
int **parking;
int *plazacamionplanta;

//Cantidad de plazas y pisos del programa
int plazas;
int pisos;

//Guardara el primer par de sitios libre y las plazas libres que hay
int plazaslibres, plazascamiones;



//Mutex para dispositivo de control
pthread_mutex_t mutex;

//COndicion del mutex
pthread_cond_t espera[COCHES+1];

void *coche(void *num){
    //valor booleano util para la busqueda del aprcamiento
    int salida=1;


    //valor para que el coche sepa donde aparca
    int aparcamiento[2];
    int coche_id= *(int *)num;

    //Valor de las plazas de camión que hay en la planta antes y después de aparcar el coche o sacarlo
    int placamion;

    //LOCK DEL MUTEX
    pthread_mutex_lock(&mutex);
    //printf("Quiere entrar el coche %i\n",coche_id);


    while(plazaslibres==0){
        pthread_cond_wait(&espera[coche_id], &mutex);
    }

    //Entra en seccion critica
    //printf("coche %i en la seccion critica\n",coche_id);
    plazaslibres--;
    placamion=0;
    //Busca sitio para aparcar
    for(int i=0;i<pisos && salida;i++){
        for(int j=0;j<plazas && salida;j++){
            //Sabemos que hay un sitio diferente al que no accedera un camion que estara libre, lo aprovechamos
            if(parking[i][j]==0){
                parking[i][j]=coche_id;
                aparcamiento[0]=i;
                aparcamiento[1]=j;
                //Contamos las plazas de camión en la planta después de aparcar el coche
                for(int k=j; k<plazas-1; k++){
                    if(parking[i][k]==0&&parking[i][k+1]==0){
                        k++;
                        placamion++;
                    }
                }
                //La diferencia de plazas de camiones entre antes de aparcar el coche y ahora, lo quitamos de las plazas de camión.
                plazascamiones-=(plazacamionplanta[i]-placamion);
                plazacamionplanta[i]=placamion;
                salida=0;
                printf("ENTRADA: Coche %i aparcado en plaza %i del piso %i. Plazas libres: %i. Plazas camion: %i\n",coche_id,aparcamiento[1],aparcamiento[0],plazaslibres, plazascamiones);
            }
        }
    }


    pthread_mutex_unlock(&mutex);

    sleep( (rand() % 5) + 3);

    //Entramos en la seccion critica
    pthread_mutex_lock(&mutex);
    parking[aparcamiento[0]][aparcamiento[1]]=0;
    plazaslibres++;
    placamion=0;
    for(int i=0; i<plazas-1; i++){
        if(parking[aparcamiento[0]][i]==0&&parking[aparcamiento[0]][i+1]==0){
            i++;
            placamion++;
        }
    }
    plazascamiones+=(placamion-plazacamionplanta[aparcamiento[0]]);
    plazacamionplanta[aparcamiento[0]]=placamion;
    printf("SALIDA: Coche %i saliendo. Plazas libres: %i. Plazas camión: %i\n",coche_id,plazaslibres,plazascamiones);
    if(plazascamiones>0){
        for(int i=COCHES+1;i<=COCHES+CAMIONES;i++){
            pthread_cond_signal(&espera[i]);
        }
    }
    for(int i=1;i<=COCHES;i++){
        pthread_cond_signal(&espera[i]);
    }

    //Esta implementacion funciona, problema, el coche 3 que se supone que entra rapido, puede ser de los ultimos en entrar.

    pthread_mutex_unlock(&mutex);
    //Salimos de la seccion critica
}

void *camion(void *num){
    //valor booleano util para la busqueda del aprcamiento
    int salida=1;


    //valor para que el camión sepa donde aparca
    int aparcamiento[3];
    int coche_id= *(int *)num;

    //LOCK DEL MUTEX
    pthread_mutex_lock(&mutex);
    //printf("Quiere entrar el coche %i\n",coche_id);


    while(plazascamiones==0){
        pthread_cond_wait(&espera[coche_id], &mutex);
    }

    //Entra en seccion critica
    //printf("coche %i en la seccion critica\n",coche_id);


    plazaslibres=plazaslibres-2;
    plazascamiones--;
    printf("Estoy aquí\n");
    //Busca sitio para aparcar
    for(int i=0;i<pisos && salida;i++){
        for(int j=0;j<plazas-1 && salida;j++){
            //Sabemos que hay un sitio diferente al que no accedera un camion que estara libre, lo aprovechamos
            if(parking[i][j]==0&&parking[i][j+1]==0){
                parking[i][j]=coche_id;
                parking[i][j+1]=coche_id;
                aparcamiento[0]=i;
                aparcamiento[1]=j;
                aparcamiento[2]=j+1;
                plazacamionplanta[i]--;
                salida=0;
                printf("ENTRADA: Camion %i aparcado en plaza %i del piso %i. Plazas libres: %i\n",coche_id,aparcamiento[1],aparcamiento[0],plazaslibres);
            }
        }
    }


    pthread_mutex_unlock(&mutex);

    sleep( (rand() % 5) + 3);

    //Entramos en la seccion critica
    pthread_mutex_lock(&mutex);
    parking[aparcamiento[0]][aparcamiento[1]]=0;
    parking[aparcamiento[0]][aparcamiento[2]]=0;
    plazaslibres=plazaslibres+2;
    plazascamiones++;
    plazacamionplanta[aparcamiento[0]]++;
    printf("SALIDA: Camion %i saliendo. Plazas libres: %i\n",coche_id,plazaslibres);
    for(int i=1;i<=COCHES+CAMIONES;i++){
        pthread_cond_signal(&espera[i]);
    }
    //Esta implementacion funciona, problema, el coche 3 que se supone que entra rapido, puede ser de los ultimos en entrar.

    pthread_mutex_unlock(&mutex);
    //Salimos de la seccion critica
}


int main(int argc, char *argv[]) {


    int i,j; //Contadores para los bucles
    pthread_t th;
    //Identificadores de los coches que van a entrar
    int coches_id[COCHES+CAMIONES+1];



    //Cantidad de plazas y pisos del programa
    plazas=atoi(argv[2]);
    pisos=atoi(argv[1]);


    parking = (int**) malloc(pisos * sizeof (int*));
    plazacamionplanta =(int*) malloc(pisos * sizeof (int ));
    for(i=0;i<pisos;i++){
        parking[i]=(int*) malloc(plazas*sizeof (int));
    }

    //Seteamos los valores del parking a 0 al principio comienza vacio
    for(i=0;i<pisos;i++){
        plazacamionplanta[i]=plazas/2;
        for(j=0;j<plazas;j++){
            parking[i][j]=0;
        }
    }

    //Al principio esta vacio todas las plazas estan libres
    plazaslibres=plazas*pisos;
    plazascamiones=(plazas/2)*pisos;




    pthread_mutex_init(&mutex,NULL);

    //prueba con COCHES coches
    for(int i=1;i<=COCHES;i++){
        pthread_cond_init(&espera[i], NULL);
        coches_id[i]=i;
        pthread_create(&th,NULL,coche,(void*)&coches_id[i]);
    }
    for(int i=COCHES+1; i<=COCHES+CAMIONES; i++){
        pthread_cond_init(&espera[i], NULL);
        coches_id[i]=i;
        pthread_create(&th,NULL,camion,(void*)&coches_id[i]);
    }

    while (1);
    return 0;

}
