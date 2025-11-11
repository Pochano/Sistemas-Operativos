#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>

/* -----------------------------------------------------------
                    Laboratorio 8: Threads

    Alumno: Valdivia Castillo, José Miguel Mateo
    Curso: Sistemas Operativos
    Fecha: 11/11/2025
    Nombre del Proyecto: Threads_Laboratorio.c
    Tiempo Invertido: 3 horas

   ----------------------------------------------------------- */


#define NUM_THREADS 100

/* ----------------------
    Variables Globales
   ---------------------- */

int Contador_Threads = 0;
int Indicador_Salida = 0;
pthread_t Threads[NUM_THREADS];

/* -----------------------------------------------------------
    Se ha creado esta estructura auxiliar para poder para 
    pasar múltiples parámetros (ID del hilo y número i)
    del Thread usado para la función 
    Calcular_Fibonacci_e_Imprimir.
   ----------------------------------------------------------- */

typedef struct {
    int ID_Thread;      
    int Numero_Ingresado;
} Argumentos_Fibonacci;


/* -----------------------------------------------------------
    Función Fibonacci Recursivo: Halla el n-ésimo fibonacci 
    usando recursión, retornando el dicho valor.
    Si el programa se encuentra en proceso de salida
    (Indicador_Salida == 1), el hilo actual se cancela
    inmediatamente para evitar que imprima resultados
    después de que el usuario haya elegido "Exit".
   ----------------------------------------------------------- */

uint64_t  Fibonacci(int n){

    if (Indicador_Salida != 0) {
        pthread_exit(NULL);
    }

    if(n <= 0){
        return 0;
    }
    if(n == 1){
        return 1;
    }
    return Fibonacci(n-1) + Fibonacci(n-2);
}

/* -----------------------------------------------------------
    Función de Hilo - Imprimir Nombre: Es una función que 
    será ejecutada por un hilo el cual imprimirá mi nombre 
    junto el ID del hilo que lo ejecuta. 
   ----------------------------------------------------------- */

void* Imprimir_Nombre(void *Argumento_ID){

    int ID_Thread = *(int*) Argumento_ID;
    free(Argumento_ID);

    if(Indicador_Salida != 0){
        pthread_exit(NULL);
    }

    printf("Thread ID: %d - My name is Jose (Pochano)!\n", ID_Thread);
    pthread_exit(NULL);

}

/* -----------------------------------------------------------
    Función de Hilo - Imprimir Fibonacci: Es una función que 
    será ejecutada por un hilo la cual recibe una estructura
    con el ID del hilo y la posición "i" del número solicitado
    por el usuario. Si durante el cálculo el usuario elige 
    salir, la función se interrumpe antes de imprimir.
   ----------------------------------------------------------- */

void* Calcular_Fibonacci_e_Imprimir(void *Argumento_ID){

    Argumentos_Fibonacci* Data = (Argumentos_Fibonacci*) Argumento_ID;
    
    int ID_Thread = Data->ID_Thread;
    int Numero_Ingresado = Data->Numero_Ingresado;

    free(Data);

    if(Indicador_Salida != 0){
        pthread_exit(NULL);
    }

    uint64_t  Resultado = Fibonacci(Numero_Ingresado);
    printf("Thread ID: %d - Fibonacci Number for %d: % " PRIu64 "\n",
    ID_Thread, Numero_Ingresado, Resultado);

    pthread_exit(NULL);

}

int main(){

    int Opcion_Seleccionada;

    while(1){

        printf("Enter your choice:\n");
        printf("1. Print name.\n");
        printf("2. Print i-th Fibonacci number.\n");
        printf("3. Exit program.\n");
        scanf("%d", &Opcion_Seleccionada);

        if(Opcion_Seleccionada == 1){

            if(Contador_Threads < NUM_THREADS){

                int* ID_Thread = malloc(sizeof(int));
                *ID_Thread = Contador_Threads;

                pthread_create(&Threads[Contador_Threads++], NULL,
                Imprimir_Nombre, ID_Thread);

            }
            else{
                printf("Se superó el máximo número de threads.\n");
            }
        }

        else if(Opcion_Seleccionada == 2){

            if(Contador_Threads < NUM_THREADS){

                Argumentos_Fibonacci* Data = malloc(sizeof(Argumentos_Fibonacci));
                Data->ID_Thread = Contador_Threads;

                printf("Enter the position for Fibonacci number: ");
                scanf("%d", &Data->Numero_Ingresado);

                pthread_create(&Threads[Contador_Threads++], NULL,
                Calcular_Fibonacci_e_Imprimir, Data);

            }
            else{
                printf("Se superó el máximo número de threads.\n");
            }


        }

        else if(Opcion_Seleccionada == 3){

            printf("Exiting program. Finishing up all the remaining threads.\n");

            Indicador_Salida = 1; /* ----------------------------------------
                                    Se actualiza la variable para avisar a
                                    las demás funciones que deben parar su
                                    ejecución.
                                    ----------------------------------------- */

            /* -----------------------------------------------------------------
               Se recorren y cancelan todos los hilos creados hasta el momento.
               ----------------------------------------------------------------- */

            for(int i = 0; i < Contador_Threads; i++){
                pthread_cancel(Threads[i]);
                pthread_join(Threads[i], NULL);
            }

            printf("All threads have been terminated. Goodbye Teacher Rosa!\n");
            break;

        }

        else{
            printf("Invalid Option: Please try positive numbers between 1 and 3\n");
        }

        printf("\n");

    }

    return 0;

}