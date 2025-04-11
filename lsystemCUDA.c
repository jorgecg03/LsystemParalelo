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

#define MAX_OUTPUT_SIZE 1024*1024
#define MAX_RULE_LENGTH 10

Lsystem *ls;	// Estructura del sistema de lindemayer actual
State *state;	// Pila de estados para manejo de posiciones/ángulos con corchetes [ ]
int x, y;		// Coordenadas actuales del cursor de dibujo
double angle;	// Ángulo de orientación actual
char *curgen;	// Generación actual del L-system (cadena)




__device__ int apply_rule(char input, char *output) {
    // Regla de ejemplo: F -> FF, todo lo demás se mantiene igual
    if (input == 'F') {
        output[0] = 'F';
        output[1] = 'F';
        return 2; // longitud de la nueva cadena
    } else {
        output[0] = input;
        return 1;
    }
}

__global__ void nextgen_kernel(char *curgen, int len, char *output, int *offsets) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= len) return;

    char temp[MAX_RULE_LENGTH];
    int newlen = apply_rule(curgen[idx], temp);

    // Copiar la producción generada al búfer de salida en su posición correspondiente
    int offset = offsets[idx];
    for (int i = 0; i < newlen; i++) {
        output[offset + i] = temp[i];
    }
}

void nextgen_cuda(const char *curgen_host, char **newgen_host) {
    int len = strlen(curgen_host);
    char *curgen_dev, *output_dev;
    int *offsets_host = (int *)malloc(len * sizeof(int));
    int *offsets_dev;

    // Precalcular offsets para concatenar producciones en orden
    int total = 0;
    for (int i = 0; i < len; i++) {
        offsets_host[i] = total;
        total += (curgen_host[i] == 'F') ? 2 : 1;
    }

    cudaMalloc(&curgen_dev, len);
    cudaMemcpy(curgen_dev, curgen_host, len, cudaMemcpyHostToDevice);

    cudaMalloc(&offsets_dev, len * sizeof(int));
    cudaMemcpy(offsets_dev, offsets_host, len * sizeof(int), cudaMemcpyHostToDevice);

    cudaMalloc(&output_dev, total);
    char *result_host = (char *)malloc(total + 1);

    int threadsPerBlock = 256;
    int blocksPerGrid = (len + threadsPerBlock - 1) / threadsPerBlock;
    nextgen_kernel<<<blocksPerGrid, threadsPerBlock>>>(curgen_dev, len, output_dev, offsets_dev);
    cudaDeviceSynchronize();

    cudaMemcpy(result_host, output_dev, total, cudaMemcpyDeviceToHost);
    result_host[total] = '\0';

    *newgen_host = result_host;

    cudaFree(curgen_dev);
    cudaFree(output_dev);
    cudaFree(offsets_dev);
    free(offsets_host);
}

// --------------------------- MAIN ----------------------------------

int main() {
    char *axiom = strdup("F+F+F");
    char *nextgen = NULL;

    printf("Axiom: %s\n", axiom);
    nextgen_cuda(axiom, &nextgen);
    printf("Next generation: %s\n", nextgen);

    free(axiom);
    free(nextgen);
    return 0;
}
