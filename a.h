// Declaración anticipada de estructuras
typedef struct Lsystem Lsystem;
typedef struct Rule Rule;
typedef struct State State;

/**
 * Representa un sistema de Lindenmayer (L-system).
 *
 * Un L-system se define por:
 * - name: Nombre del sistema, usado en la interfaz o para referencia.
 * - axiom: Cadena inicial que define la generación 0.
 * - rules: Lista enlazada de reglas de producción.
 * - linelen: Longitud de línea para cada paso de dibujo.
 * - initangle: Ángulo inicial del cursor de dibujo.
 * - leftangle: Ángulo de rotación hacia la izquierda (para el símbolo '-').
 * - rightangle: Ángulo de rotación hacia la derecha (para el símbolo '+').
 */
struct Lsystem
{
	char*	name;        // Nombre del sistema
	char*	axiom;       // Axioma o cadena base
	Rule*	rules;       // Reglas de producción (lista enlazada)
	int	linelen;         // Longitud de cada segmento dibujado
	double	initangle;   // Ángulo inicial del cursor
	double	leftangle;   // Rotación hacia la izquierda
	double	rightangle;  // Rotación hacia la derecha
};

/**
 * Representa una regla de producción del L-system.
 *
 * Cada regla transforma un carácter (pred) en una secuencia (succ).
 * Las reglas están encadenadas en una lista enlazada.
 */
struct Rule
{
	char	pred;     // Símbolo que será reemplazado
	char*	succ;     // Cadena de reemplazo
	Rule*	next;     // Siguiente regla en la lista
};

/**
 * Representa el estado del cursor en el dibujo.
 *
 * Se utiliza como una pila para manejar ramificaciones.
 * Guarda la posición (x, y) y el ángulo.
 */
struct State
{
	int	x;        // Coordenada X del cursor
	int	y;        // Coordenada Y del cursor
	double	angle;        // Ángulo de orientación del cursor
	State*	prev;     // Puntero al estado anterior en la pila
};

/**
 * emalloc - Envoltorio de malloc que aborta si falla.
 * Similar a malloc, pero garantiza que el programa terminará si no hay memoria.
 */
void* emalloc(size_t size);

/**
 * parse - Parsea un archivo y crea un L-system a partir de él.
 *
 * @filename: ruta al archivo que contiene la definición del L-system.
 * @return: puntero a una estructura Lsystem completamente inicializada.
 *
 * El archivo debe tener una sintaxis específica con axioma y reglas.
 */
Lsystem* parse(char *filename);

