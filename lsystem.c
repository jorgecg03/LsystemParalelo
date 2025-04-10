#include <SDL2/SDL.h> // Librería para manejo de gráficos y eventos
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "a.h"

#define WIDTH 800	// Ancho de la ventana
#define HEIGHT 600	// Alto de la ventana
#define M_PI 3.14159265358979323846

Lsystem *ls;	// Estructura del sistema de lindemayer actual
State *state;	// Pila de estados para manejo de posiciones/ángulos con corchetes [ ]
int x, y;		// Coordenadas actuales del cursor de dibujo
double angle;	// Ángulo de orientación actual
char *curgen;	// Generación actual del L-system (cadena)

int offsetX = 0, offsetY = 0;	// Desplazamiento visual de la escena en pantalla

/**
 * Guarda el estado actual (posición y ángulo) en una pila.
 * Utilizado para estructuras de ramificación en el dibujo (carácter '[').
 */
void pushstate(void) {
	State *s = emalloc(sizeof(State));
	s->x = x;
	s->y = y;
	s->angle = angle;
	s->prev = state;
	state = s;
}

/**
 * Restaura el último estado de dibujo guardado en la pila.
 * Utilizado al cerrar una rama del sistema (carácter ']').
 */
void popstate(void) {
	State *s = state;
	x = s->x;
	y = s->y;
	angle = s->angle;
	state = state->prev;
	free(s);
}

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
void nextgen(void) {
    char *newgen = emalloc(1024 * strlen(curgen)); 
    newgen[0] = '\0';

    for (char *p = curgen; *p; p++) {
        char *prod = production(*p);
        if (prod)
            strcat(newgen, prod);
        else {
            size_t len = strlen(newgen);
            newgen[len] = *p;
            newgen[len + 1] = '\0';
        }
    }

    free(curgen);
    curgen = strdup(newgen);
	printf("\ncadena: %s\n",curgen);
    free(newgen);
}

/**
 * Traza una línea desde la posición actual en la dirección del ángulo actual.
 * Luego actualiza la posición del cursor.
 */
void forward(SDL_Renderer *renderer) {
	int x1 = x + ls->linelen * cos(angle * M_PI / 180.0);
	int y1 = y - ls->linelen * sin(angle * M_PI / 180.0);
	SDL_RenderDrawLine(renderer, x + offsetX, y + offsetY, x1 + offsetX, y1 + offsetY);
	x = x1;
	y = y1;
}

/**
 * Rota el ángulo actual en una cierta cantidad.
 */
void rotate(double angulo) {
	angle += angulo;
	if (angle >= 360.0) angle -= 360.0;
	if (angle <= 0.0)   angle += 360.0;
}


/**
 * Limpia la pantalla y dibuja la generación actual del sistema de Lindenmayer.
 * Interpreta los caracteres de la cadena generada para realizar acciones de dibujo.
 */
void redraw(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);	// Color blanco
    SDL_RenderClear(renderer);	// Limpia pantalla


	x = WIDTH / 2;
	y = HEIGHT - 400;
	angle = ls->initangle;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);	// Color negro para dibujar

	for (char *s = curgen; *s; s++) {	// Recorre la cadena actual
		switch (*s) {
			case 'F':
			case 'G':
				forward(renderer);		// Avanza y dibuja línea
				break;
			case '-':
				rotate(ls->leftangle);	// Gira hacia la izquierda
				break;
			case '+':
				rotate(ls->rightangle);	// Gira hacia la derecha
				break;
			case '[':
				pushstate();			// Guarda estado actual
				break;
			case ']':
				popstate();				// Restaura estado anterior
				break;
		}
	}
	SDL_RenderPresent(renderer);		// Muestra en pantalla lo dibujado
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s archivo\n", argv[0]);
        return 1;
    }

    ls = parse(argv[1]);		// Parsea el archivo L-system
    curgen = strdup(ls->axiom);	// Copia el axioma como cadena inicial

	// Inicializa SDL
    SDL_Init(SDL_INIT_VIDEO);	
    SDL_Window *win = SDL_CreateWindow("L-System", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	// Pone pantalla completa
    SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);

	// Dibuja por primera vez
    redraw(ren);	

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                nextgen();		// Avanza a la siguiente generación
                redraw(ren);	// Redibuja
            }
            else if (e.type == SDL_KEYDOWN) {
                // Movimiento con las teclas de flecha
                if (e.key.keysym.sym == SDLK_LEFT) {
                    offsetX += 100;
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    offsetX -= 100;
                } else if (e.key.keysym.sym == SDLK_UP) {
                    offsetY += 100;
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    offsetY -= 100;
                }
				else if (e.key.keysym.sym == SDLK_ESCAPE) {  // Detener el programa al presionar ESC
					quit = 1;
				}

				// Redibuja después del movimiento
                redraw(ren);	
            }
        }

		// Pequeña pausa para no saturar CPU
        SDL_Delay(10);
    }

	// Limpieza
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
