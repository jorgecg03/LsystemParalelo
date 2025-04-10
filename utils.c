#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/**
 * emalloc: reserva memoria del tamaño indicado y la llena con ceros.
 * En caso de error, imprime el mensaje y termina el programa.
 */
void*
emalloc(size_t size)
{
    void *p = calloc(1, size);  // calloc inicializa en cero

    if(p == NULL){
        fprintf(stderr, "emalloc: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return p;
}