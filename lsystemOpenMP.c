#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined (_OPENMP)
	#include <omp.h>
#endif
#include "a.h"

#define WIDTH 800
#define HEIGHT 600
#define M_PI 3.14159265358979323846

// Variables globales
Lsystem *ls;
int n;
State *state;
int x, y;         
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

void redraw(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

	x = WIDTH / 2;
	y = HEIGHT - 20;
	angle = ls->initangle;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	for (char *s = curgen; *s; s++) {
		switch (*s) {
			case 'F':
			case 'G':
				forward(renderer);
				break;
			case '-':
				rotate(ls->leftangle);
				break;
			case '+':
				rotate(ls->rightangle);
				break;
			case '[':
				pushstate();
				break;
			case ']':
				popstate();
				break;
		}
	}
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s archivo.lsys num_threads\n", argv[0]);
        return 1;
    }
    

    ls = parse(argv[1]);
    n = atoi(argv[2]);
    curgen = strdup(ls->axiom);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("L-System", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // Pantalla completa
    SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);

    redraw(ren);

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = 1;
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                nextgen();
                redraw(ren);
            }
            else if (e.type == SDL_KEYDOWN) {
                // Movimiento con las teclas de flecha
                if (e.key.keysym.sym == SDLK_LEFT) {
                    offsetX -= 10;
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    offsetX += 10;
                } else if (e.key.keysym.sym == SDLK_UP) {
                    offsetY -= 10;
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    offsetY += 10;
                }
                redraw(ren);
            }
        }
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
