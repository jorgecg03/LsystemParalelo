#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "a.h"

#define BUFSIZE 1024

// Declaración de funciones auxiliares
Rule* mkrule(char pred, char *succ);
void skipws(FILE *f);
char* next(FILE *f);
char* readstring(FILE *f);
char* readnumber(FILE *f, int real);


/**
 * parse - Parsea un archivo de definición de L-system.
 * 
 * @filename: Ruta al archivo de entrada.
 * @return: Estructura Lsystem* completamente construida a partir del archivo.
 * 
 * El archivo debe contener tokens como:
 * - name 'Nombre'
 * - axiom F+F+F
 * - rule F → F-F++
 * - line-length 10
 * - initial-angle 90
 * - left-angle 45
 * - right-angle 45
 */
Lsystem* parse(char *filename) {
    Lsystem *ls;
    Rule *r;
    FILE *fp;
    char *s, c;

    // Abrir el archivo de entrada
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // Reservar memoria para el sistema L
    ls = emalloc(sizeof(Lsystem));

    while (1) {
        skipws(fp);
        s = next(fp);
        if (strlen(s) == 0) {
            free(s);
            break;
        }
        // Análisis del contenido del archivo dependiendo del token leído
        if (strcmp(s, "name") == 0) { // Leer nombre del sistema
            free(s);
            skipws(fp);
            ls->name = readstring(fp);

        } else if (strcmp(s, "axiom") == 0) { // Leer axioma inicial
            free(s);
            skipws(fp);
            ls->axiom = next(fp);

        } else if (strcmp(s, "rule") == 0) { // Leer regla
            free(s);
            skipws(fp);
            s = next(fp);
            c = s[0]; // Leer el símbolo predicado
            free(s);
            skipws(fp);
            s = next(fp);
            if (strcmp(s, "->") != 0) {
                fprintf(stderr, "expected '->' but got '%s'\n", s);
                exit(1);
            }
            free(s);
            skipws(fp);
            s = next(fp); // Leer el sucesor de la regla
            r = mkrule(c, s); // Crear la regla
            r->next = ls->rules; //Enlaza la nueva regla con el resto de la lista
            ls->rules = r;// Insertarla al inicio de la lista de reglas

        } else if (strcmp(s, "line-length") == 0) {// Leer longitud de línea
            free(s);
            skipws(fp);
            s = readnumber(fp, 0);
            ls->linelen = atoi(s);
            free(s);

        } else if (strcmp(s, "initial-angle") == 0) {// Leer ángulo inicial
            free(s);
            skipws(fp);
            s = readnumber(fp, 1);
            ls->initangle = atof(s);
            free(s);

        } else if (strcmp(s, "left-angle") == 0) {// Leer ángulo de giro a la izquierda
            free(s);
            skipws(fp);
            s = readnumber(fp, 1);
            ls->leftangle = atof(s);
            free(s);

        } else if (strcmp(s, "right-angle") == 0) {// Leer ángulo de giro a la derecha
            free(s);
            skipws(fp);
            s = readnumber(fp, 1);
            ls->rightangle = atof(s);
            free(s);

        } else {
            fprintf(stderr, "unexpected token '%s'\n", s);
            exit(1);
        }
    }
    // Verificaciones finales para asegurar que el sistema esté completo
    if (ls->name == NULL)
        fprintf(stderr, "missing lsystem name\n"), exit(1);
    if (ls->axiom == NULL)
        fprintf(stderr, "no axiom defined\n"), exit(1);
    if (ls->rules == NULL)
        fprintf(stderr, "no rules defined\n"), exit(1);
	

    fclose(fp);
    return ls;
}

/**
 * mkrule - Crea una nueva regla de producción para el L-system.
 *
 * @pred: Carácter que se desea reemplazar.
 * @succ: Cadena con la sustitución correspondiente.
 * @return: Puntero a la regla creada.
 */
Rule*
mkrule(char pred, char *succ)
{
	Rule *r = emalloc(sizeof(Rule));
	r->pred = pred;
	r->succ = strdup(succ);
	r->next = NULL;
	return r;
}

/**
 * skipws - Omite espacios en blanco en el búfer de entrada.
 *
 * @f: Puntero al búfer de entrada.
 */


void skipws(FILE *f) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (!isspace(c)) {
            ungetc(c, f);
            break;
        }
    }
}

/**
 * next - Lee el siguiente token desde el búfer.
 *
 * @f: Puntero al búfer.
 * @return: Cadena con el token leído.
 */


char* next(FILE *f) {
    char buf[BUFSIZE];
    int c, n = 0;

    while ((c = fgetc(f)) != EOF) {
        if (isspace(c)) {
            ungetc(c, f);
            break;
        }
        if (n < BUFSIZE - 1)
            buf[n++] = c;
    }

    buf[n] = '\0';
    return strdup(buf);
}

/**
 * readstring - Lee una cadena entre comillas simples.
 *
 * @f: Puntero al búfer.
 * @return: Cadena leída (sin las comillas).
 */


char* readstring(FILE *f) {
    char buf[BUFSIZE];
    int c, n = 0;

    c = fgetc(f);
    if (c != '\'') {
        fprintf(stderr, "expected ' but got %c\n", c);
        exit(1);
    }

    while ((c = fgetc(f)) != EOF) {
        if (c == '\'')
            break;
        if (!isalnum(c) && c != ' ') {
            fprintf(stderr, "unexpected character %c in string\n", c);
            exit(1);
        }
        if (n < BUFSIZE - 1)
            buf[n++] = c;
    }

    buf[n] = '\0';
    return strdup(buf);
}

/**
 * readnumber - Lee un número (entero o real) del búfer.
 *
 * @f: Puntero al búfer.
 * @real: Si es 1, permite punto decimal.
 * @return: Cadena con el número leído.
 */


char* readnumber(FILE *f, int real) {
    char buf[BUFSIZE];
    int c, n = 0, d = 0;

    while ((c = fgetc(f)) != EOF) {
        if (isspace(c))
            break;
        if (isdigit(c) ||
            (n == 0 && (c == '-' || c == '+')) ||
            (real && c == '.' && !d)) {
            if (c == '.')
                d = 1;
            if (n < BUFSIZE - 1)
                buf[n++] = c;
        } else {
            fprintf(stderr, "unexpected %c in number\n", c);
            exit(1);
        }
    }

    buf[n] = '\0';
    return strdup(buf);
}
