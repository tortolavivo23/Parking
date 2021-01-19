#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


struct dato {
        int num;
        int coche; //1 si es coche, 0 si es camión
};

struct lista { /* lista simple enlazada */
    struct dato *datos;
    struct lista *sig;
};

struct lista *creanodo(void) {
    return (struct lista *) malloc(sizeof(struct lista));
}

struct lista *insertafinal(struct lista *l, struct dato *x) {
    struct lista *p,*q;
    q = creanodo(); /* crea un nuevo nodo */
    q->datos = x; /* copiar los datos */
    q->sig = NULL;
    if (l == NULL) {
        return q;
    }
    /* la lista argumento no es vacía. Situarse en el último */
    p = l;
    while (p->sig != NULL)
        p = p->sig;
    p->sig = q;
    return l;
}

struct lista *eliminaprimer(struct lista *p) {
    struct lista *q;
    if (p == NULL) return p;
    q = p->sig;
    free(p); /* libera la memoria y hemos perdido el enlace, por eso se guarda en q */
    p = q; /* restaurar p al nuevo valor */
    return p;
}

struct dato *primerelemento(struct lista *p){
    if (p == NULL) return NULL;
    return p->datos;
}

struct dato *creadato(int num, int coche){
    struct dato *d;
    d = (struct dato *) malloc(sizeof(struct dato));
    d->num=num;
    d->coche=coche;
    return d;
}

struct lista *anulalista(struct lista *l) {
    struct lista *p,*q;
    p = l;
    while (p != NULL) {
        q = p->sig; /* para no perder el nodo */
        free(p);
        p = q;
    }
    return NULL;
}

int equals(struct dato *dato1, struct dato *dato2){
    if(dato1==NULL){
        if(dato2==NULL){
            return 1;
        }
        return 0;
    }
    if((dato1->num==dato2->num)&&(dato1->coche==dato2->coche)){
        return 1;
    }
    else{
        return 0;
    }
}

//La variable parking simulara las plazas del parking a ellas se accede de la siguiente forma:
//planta 2 aparcamiento 4 seria parking[2-1][4-1];
int **parking;
int *plazacamionplanta;

//Cantidad de plazas y pisos del programa
int plazas;
int pisos;

//Guardara el primer par de sitios libre y las plazas libres que hay
int plazaslibres, plazascamiones;

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

//cola
struct lista *cola;

void mostrarParking(){
    int i,j;
    for(i=0;i<pisos;i++){
        for(j=0;j<plazas;j++){
            printf("[%i] ",parking[i][j]);
        }
        printf("\n");
    }
}

void *coche(void *num) {
    while (1) {


        //valor booleano util para la busqueda del aprcamiento
        int salida = 1;


        //valor para que el coche sepa donde aparca
        int aparcamiento[2];
        int coche_id = *(int *) num;

        //Valor de las plazas de camión que hay en la planta antes y después de aparcar el coche o sacarlo
        int placamion;

        //Creamos un dato para poder usarlo;
        struct dato *d;


        //LOCK DEL MUTEX
        pthread_mutex_lock(&mutex);

        //printf("Quiere entrar el coche %i\n",coche_id);
        d = creadato(coche_id,1);
        cola = insertafinal(cola, d);

        //Si no hay plazas o no es el primer elemento de la cola, debe esperar la señal para continuar.
        while (plazaslibres==0||equals(d,primerelemento(cola))==0) {
            pthread_cond_wait(&esperacoches[coche_id], &mutex);
        }
        cola = eliminaprimer(cola);
        //Entra en seccion critica
        //printf("coche %i en la seccion critica\n",coche_id);
        plazaslibres--;
        placamion = 0;
        //Busca sitio para aparcar
        for (int i = 0; i < pisos && salida; i++) {
            for (int j = 0; j < plazas && salida; j++) {
                //Sabemos que hay un sitio diferente al que no accedera un camion que estara libre, lo aprovechamos
                if (parking[i][j] == 0) {
                    parking[i][j] = coche_id;
                    aparcamiento[0] = i;
                    aparcamiento[1] = j;
                    //Contamos las plazas de camión en la planta después de aparcar el coche
                    for (int k = j; k < plazas - 1; k++) {
                        if (parking[i][k] == 0 && parking[i][k + 1] == 0) {
                            k++;
                            placamion++;
                        }
                    }
                    //La diferencia de plazas de camiones entre antes de aparcar el coche y ahora, lo quitamos de las plazas de camión.
                    plazascamiones -= (plazacamionplanta[i] - placamion);
                    plazacamionplanta[i] = placamion;
                    salida = 0;
                    printf("ENTRADA: Coche %i aparcado en plaza %i del piso %i. Plazas libres: %i. Plazas camion: %i\n",
                           coche_id, aparcamiento[1], aparcamiento[0], plazaslibres, plazascamiones);
                    mostrarParking();
                }
            }
        }

        //Enviamos señal al primer elemento de la cola
        d = primerelemento(cola);
        if (d != NULL) {
            if (d->coche == 0) {
                pthread_cond_signal(&esperacamiones[d->num]);
            } else if (d->coche == 1) {
                pthread_cond_signal(&esperacoches[d->num]);
            }
        }


        pthread_mutex_unlock(&mutex);

        sleep((rand() % 5) + 3);

        //Entramos en la seccion critica
        pthread_mutex_lock(&mutex);
        parking[aparcamiento[0]][aparcamiento[1]] = 0;
        plazaslibres++;
        placamion = 0;
        for (int i = 0; i < plazas - 1; i++) {
            if (parking[aparcamiento[0]][i] == 0 && parking[aparcamiento[0]][i + 1] == 0) {
                i++;
                placamion++;
            }
        }
        plazascamiones += (placamion - plazacamionplanta[aparcamiento[0]]);
        plazacamionplanta[aparcamiento[0]] = placamion;
        printf("SALIDA: Coche %i saliendo. Plazas libres: %i. Plazas camión: %i\n", coche_id, plazaslibres,
               plazascamiones);
        /*
        for (int i = 1; i <= cantcoches; i++) {
            pthread_cond_signal(&esperacoches[i]);
        }
        for (int i = 1; i <= cantcamiones; i++) {
            pthread_cond_signal(&esperacamiones[i]);
        }*/

        //Esta implementacion funciona, problema, el coche 3 que se supone que entra rapido, puede ser de los ultimos en entrar.

        //Enviamos señal al primer elemento de la cola
        d = primerelemento(cola);
        if (d != NULL) {
            if (d->coche == 0) {
                pthread_cond_signal(&esperacamiones[d->num]);
            } else{
                pthread_cond_signal(&esperacoches[d->num]);
            }
        }

        pthread_mutex_unlock(&mutex);
        //Salimos de la seccion critica
        sleep((rand() % 10) + 3);
    }
}

void *camion(void *num) {
    while (1) {

        //valor booleano util para la busqueda del aprcamiento
        int salida = 1;


        //valor para que el camión sepa donde aparca
        int aparcamiento[3];
        int camion_id = *(int *) num;

        //Valor de las plazas de camión que hay en la planta antes y después de aparcar el coche o sacarlo
        int placamion;

        //Creamos un dato para poder usarlo;
        struct dato *d;


        //LOCK DEL MUTEX
        pthread_mutex_lock(&mutex);
        //printf("Quiere entrar el camion %i\n",camion_id);
        d = creadato(camion_id-100,0);
        cola=insertafinal(cola, d);
        //Si no hay plazas o no es el primer elemento de la cola, debe esperar la señal para continuar.
        while (plazascamiones==0||equals(d,primerelemento(cola)) == 0) {
            pthread_cond_wait(&esperacamiones[camion_id - 100], &mutex);
        }
        cola=eliminaprimer(cola);
        //Entra en seccion critica
        //printf("coche %i en la seccion critica\n",coche_id);


        plazaslibres -= 2;
        plazascamiones--;
        //Busca sitio para aparcar
        for (int i = 0; i < pisos && salida; i++) {
            if (plazacamionplanta[i]>0) {
                for (int j = 0; j < plazas - 1 && salida; j++) {
                    //Sabemos que hay un sitio diferente al que no accedera un camion que estara libre, lo aprovechamos
                    if (parking[i][j] == 0 && parking[i][j + 1] == 0) {
                        parking[i][j] = camion_id;
                        parking[i][j + 1] = camion_id;
                        aparcamiento[0] = i;
                        aparcamiento[1] = j;
                        aparcamiento[2] = j + 1;
                        plazacamionplanta[i]--;
                        salida = 0;
                        printf("ENTRADA: Camion %i aparcado en plaza %i del piso %i. Plazas libres: %i Plazas camión: %i\n", camion_id,
                               aparcamiento[1], aparcamiento[0], plazaslibres,plazascamiones);
                        mostrarParking();
                    }
                }
            }
        }

        //Enviamos una señal al primer elemento de la cola
        d = primerelemento(cola);
        if (d != NULL) {
            if (d->coche == 0) {
                pthread_cond_signal(&esperacamiones[d->num]);
            } else  {
                pthread_cond_signal(&esperacoches[d->num]);
            }
        }


        pthread_mutex_unlock(&mutex);

        sleep((rand() % 10) + 3);

        //Entramos en la seccion critica
        pthread_mutex_lock(&mutex);
        parking[aparcamiento[0]][aparcamiento[1]] = 0;
        parking[aparcamiento[0]][aparcamiento[2]] = 0;
        plazaslibres = plazaslibres + 2;

        placamion = 0;
        for (int i = 0; i < plazas - 1; i++) {
            if (parking[aparcamiento[0]][i] == 0 && parking[aparcamiento[0]][i + 1] == 0) {
                i++;
                placamion++;
            }
        }
        plazascamiones += (placamion - plazacamionplanta[aparcamiento[0]]);
        plazacamionplanta[aparcamiento[0]] = placamion;
        printf("SALIDA: Camion %i saliendo. Plazas libres: %i. Plazas camión: %i\n", camion_id, plazaslibres,plazascamiones);
        /*
        for (int i = 1; i <= cantcoches; i++) {
            pthread_cond_signal(&esperacoches[i]);
        }
        for (int i = 1; i <= cantcamiones; i++) {
            pthread_cond_signal(&esperacamiones[i]);
        }

        //Esta implementacion funciona, problema, el coche 3 que se supone que entra rapido, puede ser de los ultimos en entrar.
        //Por ejemplo, es decir, no hay un orden de entrada.*/

        //Enviamos una señal al primer elemento de la cola
        d = primerelemento(cola);
        if (d != NULL) {
            if (d->coche == 0) {
                pthread_cond_signal(&esperacamiones[d->num]);
            } else if (d->coche == 1) {
                pthread_cond_signal(&esperacoches[d->num]);
            }
        }
        pthread_mutex_unlock(&mutex);
        //Salimos de la seccion critica
        sleep((rand() % 10) + 3);
    }
}



int main(int argc, char *argv[]) {
    int i,j; //Contadores para los bucles
    pthread_t th;
    //Identificadores de los coches y camiones que van a entrar
    int *coches_id;
    int *camiones_id;
    cola = NULL; //Inicialización

    //Cantidad de plazas y pisos del programa
    plazas=atoi(argv[2]);
    pisos=atoi(argv[1]);


    parking = (int**) malloc(pisos * sizeof (int*));
    plazacamionplanta =(int*) malloc(pisos * sizeof (int ));
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
    coches_id = (int*) malloc((cantcoches+cantcamiones+1)*sizeof (int));
    esperacoches = (pthread_cond_t*) malloc((cantcoches+1)*sizeof (pthread_cond_t));


    if(argc>=5){
        cantcamiones=atoi(argv[4]);
        if(plazas<2){
            printf("Como por la disposición del parking no van a poder entrar camiones, se va a reducir su número a 0.\n");
            cantcamiones=0;
        }
    }
    else{
        cantcamiones=0;
    }

    //Creamos el array de ids para los camiones:
    camiones_id = (int*) malloc((cantcamiones+1)*sizeof (int));
    esperacamiones = (pthread_cond_t*) malloc((cantcamiones+1)*sizeof (pthread_cond_t));

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

    //entran cantcoches coches
    for(int i=1;i<=cantcoches;i++){
        pthread_cond_init(&esperacoches[i], NULL);
        coches_id[i]=i;
        pthread_create(&th,NULL,coche,(void*)&coches_id[i]);
    }
    for(int i=1; i<=cantcamiones; i++){
        pthread_cond_init(&esperacamiones[i], NULL);
        camiones_id[i]=i+100;
        pthread_create(&th,NULL,camion,(void*)&camiones_id[i]);
    }

    while (1);
    cola=anulalista(cola);
    return 0;

}
