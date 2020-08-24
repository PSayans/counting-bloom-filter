#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "bloom.h"


//lista enlazada que contiene los punteros a las funciones de hashing
struct bloom_hash {
    hash_function func;
    struct bloom_hash *next;
};


//estructura del propio BF con la lista de hashes, el array de bits (cambiar a integer) y el tamaño
struct bloom_filter {
    struct bloom_hash *func;
    void *bits; //cambiar a un puntero de enteros
    size_t size;
};


//funcion que crea un filtro de tamaño size
bloom_t bloom_create(size_t size) {
	bloom_t res = calloc(1, sizeof(struct bloom_filter));
	res->size = size;
	res->bits = malloc(size);
	return res;
}


//funcion que libera la memoria para destruir el filtro filter
void bloom_free(bloom_t filter) {
	if (filter) {
		while (filter->func) {
			struct bloom_hash *h = filter->func;
			filter->func = h->next;
			free(h);
		}
		free(filter->bits);
		free(filter);
	}
}

//funcion que añade una funcion de hashing func a la lista enlazada del filtro filter
void bloom_add_hash(bloom_t filter, hash_function func) {
	struct bloom_hash *h = calloc(1, sizeof(struct bloom_hash));
	h->func = func;
	struct bloom_hash *last = filter->func;
	while (last && last->next) {
		last = last->next;
	}
	if (last) {
		last->next = h;
	} else {
		filter->func = h;
	}
}

//funcion que añade un elemento item al filtro filter
void bloom_add(bloom_t filter, const void *item) {
	struct bloom_hash *h = filter->func;
	uint8_t *bits = filter->bits;
	while (h) {
		unsigned int hash = h->func(item);
		hash %= filter->size * 8;
		bits[hash / 8] |= 1 << hash % 8; //esto hay que cambiarlo para que en vez de poner a 1 el bit le meta el valor del contador
		h = h->next;
	}
}

//TODO: funcion que elimina el elemento item del filtro filter
void bloom_remove (bloom_t filter, const void *item) {

}


//funcion que evalua la pertenencia del elemento item al filtro filter
bool bloom_test(bloom_t filter, const void *item) {
	struct bloom_hash *h = filter->func;
	uint8_t *bits = filter->bits;
	while (h) {
		unsigned int hash = h->func(item);
		hash %= filter->size * 8;
		if (!(bits[hash / 8] & 1 << hash % 8)) { //esto hay que cambiarlo para que en vez de poner a 1 el bit le meta el valor del contador
			return false;
		}
		h = h->next;
	}
	return true;
}
