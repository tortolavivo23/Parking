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

//Guardara las plazas libres que hay
int plazaslibres;

//Cantidad de coches
int cantcoches;

//Cantidad de camiones
int cantcamiones;


//Mutex para dispositivo de control
pthread_mutex_t mutex;

//COndicion del mutex
pthread_cond_t *esperacoches;
pthread_cond_t *esperacamiones;

void mostrarParking(){
    int i,j;
    for(i=0;i<pisos;i++){
        for(j=0;j<plazas;j++){
            printf("[%i] ",parking[i][j]);
        }
        printf("\n");
    }
}

void *coche(void *num){
    //valor booleano util para la busqueda del aprcamiento
    int salida=1;


    //valor para que el coche sepa donde aparca
    int aparcamiento[2];
    int coche_id= *(int *)num;

    //LOCK DEL MUTEX
    pthread_mutex_lock(&mutex);
    //printf("Quiere entrar el coche %i\n",coche_id);


    while(plazaslibres==0){
        pthread_cond_wait(&esperacoches[coche_id], &mutex);
    }

    //Entra en seccion critica
    //printf("coche %i en la seccion critica\n",coche_id);


    plazaslibres--;
    //Busca sitio para aparcar
    for(int i=0;i<pisos && salida;i++){
        for(int j=0;j<plazas && salida;j++){
            //Sabemos que hay un sitio diferente al que no accedera un camion que estara libre, lo aprovechamos
            if(parking[i][j]==0){
                parking[i][j]=coche_id;
                aparcamiento[0]=i;
                aparcamiento[1]=j;
                salida=0;
                printf("ENTRADA: Coche %i aparcado en plaza %i del piso %i. Plazas libres: %i\n",coche_id,aparcamiento[1],aparcamiento[0],plazaslibres);
                mostrarParking();
            }
        }
    }


    pthread_mutex_unlock(&mutex);

    sleep( (rand() % 5) + 3);

    //Entramos en la seccion critica
    pthread_mutex_lock(&mutex);
    parking[aparcamiento[0]][aparcamiento[1]]=0;
    plazaslibres++;
    printf("SALIDA: Coche %i saliendo. Plazas libres: %i\n",coche_id,plazaslibres);
    for(int i=1;i<=cantcoches;i++){
        pthread_cond_signal(&esperacoches[i]);
    }
    //Esta implementacion funciona, problema, el coche 3 que se supone que entra rapido, puede ser de los ultimos en entrar.
    //Por ejemplo, es decir, no hay un orden de entrada.

    pthread_mutex_unlock(&mutex);
    //Salimos de la seccion critica
}



int main(int argc, char *argv[]) {


    int i,j; //Contadores para los bucles
    pthread_t th;
    //Identificadores de los coches y camiones que van a entrar

    int *coches_id;
    int *camiones_id;



    //Cantidad de plazas y pisos del programa
    plazas=atoi(argv[2]);
    pisos=atoi(argv[1]);


    parking = (int**) malloc(pisos * sizeof (int*));
    for(i=0;i<pisos;i++){
        parking[i]=(int*) malloc(plazas*sizeof (int));
    }

    if(argc>=4){
        cantcoches=atoi(argv[3]);
    }
    else{
        cantcoches=2*plazas*pisos;
    }
    //Creamos un array de ids para los coches y uno para las condiciones del mutex de cada coche:
    coches_id = (int*) malloc((cantcoches+1)*sizeof (int));
    esperacoches = (pthread_cond_t*) malloc((cantcoches+1)*sizeof (pthread_cond_t));


    if(argc>=5){
        cantcamiones=atoi(argv[4]);
    }
    else{
        cantcamiones=0;
    }
    //Creamos el array de ids para los camiones:
    camiones_id = (int*) malloc((cantcamiones+1)*sizeof (int));
    esperacamiones = (pthread_cond_t*) malloc((cantcamiones+1)*sizeof (pthread_cond_t));

    //Seteamos los valores del parking a 0 al principio comienza vacio
    for(i=0;i<pisos;i++){
        for(j=0;j<plazas;j++){
            parking[i][j]=0;
        }
    }

    //Al principio esta vacio todas las plazas estan libres
    plazaslibres=plazas*pisos;




    pthread_mutex_init(&mutex,NULL);
    //entran cantcoches coches
    for(int i=1;i<=cantcoches;i++){
        pthread_cond_init(&esperacoches[i], NULL);
        coches_id[i]=i;
        pthread_create(&th,NULL,coche,(void*)&coches_id[i]);
    }

    while (1);
    return 0;

}
