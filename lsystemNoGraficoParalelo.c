#include <SDL2/SDL.h> // Librería para manejo de gráficos y eventos
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>


#include "a.h"

#define WIDTH 800	// Ancho de la ventana
#define HEIGHT 600	// Alto de la ventana
#define M_PI 3.14159265358979323846

Lsystem *ls;	// Estructura del sistema de lindemayer actual
State *state;	// Pila de estados para manejo de posiciones/ángulos con corchetes [ ]
int x, y;		// Coordenadas actuales del cursor de dibujo
double angle;	// Ángulo de orientación actual
char *curgen;	// Generación actual del L-system (cadena)

/**
 * Devuelve la producción asociada a un símbolo del sistema.
 * Si no hay regla definida, retorna NULL.
 */
char* production(char c) {
	for (Rule *r = ls->rules; r; r = r->next)
		if (r->pred == c)
			return r->succ;
	return NULL;
}

/**
 * Genera la próxima iteración del sistema de Lindenmayer.
 * Reemplaza cada símbolo de la cadena actual usando las reglas de producción.
 */
void nextgen(int threads) {
    size_t len = strlen(curgen);
    char **buffers = calloc(threads, sizeof(char*));
    for (int i = 0; i < threads; i++)
        buffers[i] = calloc(1024 * len / threads + 1, sizeof(char));

    #pragma omp parallel num_threads(threads)
    {
        int tid = omp_get_thread_num();
        size_t chunk = len / threads;
        size_t start = tid * chunk;
        size_t end = (tid == threads - 1) ? len : start + chunk;

        for (size_t i = start; i < end; i++) {
            char c = curgen[i];
            char *prod = production(c);
            if (prod)
                strcat(buffers[tid], prod);
            else {
                size_t l = strlen(buffers[tid]);
                buffers[tid][l] = c;
                buffers[tid][l + 1] = '\0';
            }
        }
    }

    char *newgen = emalloc(1024 * len);
    newgen[0] = '\0';
    for (int i = 0; i < threads; i++) {
        strcat(newgen, buffers[i]);
        free(buffers[i]);
    }
    free(buffers);

    free(curgen);
    curgen = strdup(newgen);
    free(newgen);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s archivo iteraciones num_hilos\n", argv[0]);
        return 1;
    }
    struct timeval inicio, fin;
    double tiempo;

    int it = atoi(argv[2]); //Obtenemos el número de iteraciones a realizar por linea de comandos
    int threads = atoi(argv[3]); //Obtenemos el número hilos por linea de comandos
    ls = parse(argv[1]);		// Parsea el archivo L-system
    curgen = strdup(ls->axiom);	// Copia el axioma como cadena inicial

    for(int i=1; i<it; i++){
        gettimeofday(&inicio, NULL);// Registrar tiempo inicial
        nextgen(threads);
        gettimeofday(&fin, NULL);   // Registrar tiempo final

        tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_usec - inicio.tv_usec) / 1000000.0; 
        printf("La iteración %d tardó %f segundos en ejecutarse.\n", i,tiempo);

    }

        
}
