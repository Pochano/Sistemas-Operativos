#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

/* -----------------------------------------------------------
                    Laboratorio 9: Semáforos
                      Problema del Barbero

    Alumno: Valdivia Castillo, José Miguel Mateo
    Curso: Sistemas Operativos
    Fecha: 18/11/2025
    Nombre del Proyecto: Problema_Barbero.c

   ----------------------------------------------------------- */

void* Barbero_Funcion(void* ID);
void* Cliente_Funcion(void* ID);
void  Atender_Cliente();
void* Crear_Cliente_Funcion();

sem_t Semaforo_Barbero_Listo; 
sem_t Semaforo_Cliente_Listo;
sem_t Semaforo_Mutex_Sillas;

int Sillas_Cantidad;
int Total_Clientes;

int Sillas_Disponibles;
int Clientes_Rechazados = 0;
int Clientes_Atendidos = 0;
time_t Tiempo_de_Espera_Suma;

/* -----------------------------------------------------------
    Barbero_Funcion:
    Función que se usa en el hilo del barbero. Usa el semáforo
    Semaforo_Cliente_Listo para "dormir" cuando no hay clientes
    y despertar cuando alguno llega. Dentro del while, atiende
    a un cliente a la vez, coordinándose con los clientes a
    través de Semaforo_Barbero_Listo y actualiza los contadores
    globales hasta que todos los clientes sean procesados.
   ----------------------------------------------------------- */

void* Barbero_Funcion(void* ID){    
    
    while (1)
    {
        sem_wait(&Semaforo_Cliente_Listo);

        sem_wait(&Semaforo_Mutex_Sillas);
        printf("\n[Barber] Available_seats: %d.\n", Sillas_Disponibles);

        Sillas_Disponibles++;

        sem_post(&Semaforo_Barbero_Listo);

        sem_post(&Semaforo_Mutex_Sillas);

        Atender_Cliente();

        sem_wait(&Semaforo_Mutex_Sillas);
        printf("\n[Barber] Customer was served.\n");
        Clientes_Atendidos++;

        if (Clientes_Atendidos + Clientes_Rechazados == Total_Clientes) {
            sem_post(&Semaforo_Mutex_Sillas);
            pthread_exit(NULL);
        }

        sem_post(&Semaforo_Mutex_Sillas);
    }

    pthread_exit(NULL); 
}

/* -----------------------------------------------------------
    Cliente_Funcion:
    Función usada en cada hilo cliente. Protege el acceso a
    Sillas_Disponibles con Semaforo_Mutex_Sillas para decidir
    si el cliente puede sentarse en la sala de espera o debe
    retirarse. Si se sienta, avisa al barbero mediante 
    Semaforo_Cliente_Listo y luego espera a ser llamado con 
    Semaforo_Barbero_Listo; si no hay sillas, aumenta el
    contador de Clientes_Rechazados.
   ----------------------------------------------------------- */

void* Cliente_Funcion(void* ID){  

    sem_wait(&Semaforo_Mutex_Sillas);

    if (Sillas_Disponibles >= 1){

        Sillas_Disponibles--;

        printf("[Customer pid = %lu] is waiting. Available seats: %d\n",
               pthread_self(), Sillas_Disponibles);

        sem_post(&Semaforo_Cliente_Listo);

        sem_post(&Semaforo_Mutex_Sillas);

        sem_wait(&Semaforo_Barbero_Listo);

        printf("[Customer pid = %lu] is being served. \n", pthread_self());        

    } else {

        Clientes_Rechazados++;
        printf("[Customer pid = %lu] left. Available_seats: %d.\n",
               pthread_self(), Sillas_Disponibles);

        sem_post(&Semaforo_Mutex_Sillas);

    }
        
    pthread_exit(NULL);
}

/* -----------------------------------------------------------
    Atender_Cliente:
    Función auxiliar que calcula un tiempo aleatorio entre 0 y 400
    milisegundos y bloquea el hilo del barbero ese tiempo
    usando usleep, simulando la duración de un corte de pelo.
   ----------------------------------------------------------- */

void Atender_Cliente() {

    int s = rand() % 401;
    s = s * 1000;
    usleep(s);

}

/* -----------------------------------------------------------
    Crear_Cliente_Funcion:
    Función usada en el hilo que genera clientes. Crea hilos
    de tipo Cliente_Funcion de manera secuencial hasta llegar
    a Total_Clientes. Entre cada creación introduce una pausa
    fija (100 ms) para espaciar la llegada de los clientes y
    observar mejor la sincronización en la barbería.
   ----------------------------------------------------------- */

void* Crear_Cliente_Funcion() {

    int Temporal;   
    int Contador_Clientes_Creados = 0;

    while (Contador_Clientes_Creados < Total_Clientes)
    {
        pthread_t Thread_Cliente;
	
        Temporal = pthread_create(&Thread_Cliente, NULL, (void *)Cliente_Funcion, NULL);  
	
        if (Temporal){
            printf("Failed to create thread.");
        }
            
        Contador_Clientes_Creados++;
        usleep(100000);
    }

    return NULL;
}

/* -----------------------------------------------------------
    main:
    Incializa todas las variables, semaforos y threads para 
    usar en las funciones. Al final muestra el número de 
    clientes atendidos y los que tuvieron que retirarse por
    falta de atención.
   ----------------------------------------------------------- */

int main(int argc, char* argv[])
{
    if (argc != 3)
        printf("Command Line Argumentes should be: ./program <number_of_seats> <number_of_customers>\n");
    else
    {
        Sillas_Cantidad = atoi(argv[1]);
        Total_Clientes  = atoi(argv[2]);

        srand(time(NULL));   

        pthread_t Thread_Barbero;
        pthread_t Thread_Creador_Clientes;

        int Temporal;

        sem_init(&Semaforo_Cliente_Listo, 0, 0);
        sem_init(&Semaforo_Barbero_Listo, 0, 0);
        sem_init(&Semaforo_Mutex_Sillas, 0, 1);
    
        Sillas_Disponibles = Sillas_Cantidad; 
        
        Temporal = pthread_create(&Thread_Barbero, NULL, (void *)Barbero_Funcion, NULL);  
    	
        if (Temporal){
            printf("Failed to create thread.");
        }
            
        Temporal = pthread_create(&Thread_Creador_Clientes, NULL, (void *)Crear_Cliente_Funcion, NULL);  
    	
        if (Temporal){
            printf("Failed to create thread.");
        }
         
        pthread_join(Thread_Barbero, NULL);
        pthread_join(Thread_Creador_Clientes, NULL);
            
        printf("\n------------------------------------------------\n");
        printf("Number of served customers: %d\n", (Total_Clientes - Clientes_Rechazados));
        printf("Number of customers that were forced to leave: %d\n", Clientes_Rechazados);
    }

    return 0;
}
