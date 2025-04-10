#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



#include "a.h"

#define WIDTH 800
#define HEIGHT 600
#define M_PI 3.14159265358979323846

// Variables globales
Lsystem *ls;
State *state;
int x, y, it;         
double angle;
char *curgen;

// Variables para desplazamiento
int offsetX = 0, offsetY = 0;

void pushstate(void) {
	State *s = emalloc(sizeof(State));
	s->x = x;
	s->y = y;
	s->angle = angle;
	s->prev = state;
	state = s;
}

void popstate(void) {
	State *s = state;
	x = s->x;
	y = s->y;
	angle = s->angle;
	state = state->prev;
	free(s);
}

char* production(char c) {
	for (Rule *r = ls->rules; r; r = r->next)
		if (r->pred == c)
			return r->succ;
	return NULL;
}

void nextgen(void) {
    size_t len = strlen(curgen);
    char *newgen = emalloc(1024 * len); 
    newgen[0] = '\0';

    #pragma omp parallel for num_threads(n) schedule(static)
    for (size_t i = 0; i < len; i++) {
        char *prod = production(curgen[i]);
        char *local_result = emalloc(256);

        if (prod) {
            strcpy(local_result, prod);  // Copia la producción
        } else {
            local_result[0] = curgen[i];  // Mantiene el carácter
            local_result[1] = '\0';
        }

        #pragma omp critical
        strcat(newgen, local_result);  // Asegura que el acceso a newgen sea seguro
        free(local_result);
    }

    free(curgen);
    curgen = strdup(newgen);
    free(newgen);
    printf("cadena: %s\n",newgen);
}

void forward(SDL_Renderer *renderer) {
	int x1 = x + ls->linelen * cos(angle * M_PI / 180.0);
	int y1 = y - ls->linelen * sin(angle * M_PI / 180.0);
	SDL_RenderDrawLine(renderer, x + offsetX, y + offsetY, x1 + offsetX, y1 + offsetY);
	x = x1;
	y = y1;
}

void rotate(double angulo) {
	angle += angulo;
	if (angle >= 360.0) angle -= 360.0;
	if (angle <= 0.0)   angle += 360.0;
}

// void redraw(SDL_Renderer *renderer) {
// 	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
//     SDL_RenderClear(renderer);

// 	x = WIDTH / 2;
// 	y = HEIGHT - 20;
// 	angle = ls->initangle;

// 	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

// 	for (char *s = curgen; *s; s++) {
// 		switch (*s) {
// 			case 'F':
// 			case 'G':
// 				forward(renderer);
// 				break;
// 			case '-':
// 				rotate(ls->leftangle);
// 				break;
// 			case '+':
// 				rotate(ls->rightangle);
// 				break;
// 			case '[':
// 				pushstate();
// 				break;
// 			case ']':
// 				popstate();
// 				break;
// 		}
// 	}
// 	SDL_RenderPresent(renderer);
// }

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s archivo.lsys iteraciones\n", argv[0]);
        return 1;
    }
    clock_t inicio, fin;
    double tiempo_cpu;

    ls = parse(argv[1]);
    it = atoi(argv[2]);
    curgen = strdup(ls->axiom);

    for(int i=1; i<it; i++){
        inicio = clock();  // Tiempo de CPU antes de ejecutar la función
        nextgen();
        fin = clock();  // Tiempo de CPU después de ejecutar la función
        tiempo_cpu = ((double)(fin - inicio)) / CLOCKS_PER_SEC;  // Convertimos a segundos
        printf("La iteración %d tardó %f segundos en ejecutarse.\n", i,tiempo_cpu);

    }






        
}
