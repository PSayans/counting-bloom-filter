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
    int32_t *counters; //cambiar a un puntero de enteros
    size_t size;
};


//funcion que crea un filtro de tamaño size
bloom_t bloom_create(size_t size) {
	//puntero al comienzo del array de bloom_filters (que solo hay 1)
	bloom_t res = calloc(1, sizeof(struct bloom_filter));
	res->size = size;
	//se reservan size bytes de memoria y se devuelve un puntero al principio
	res->counters = calloc(size, sizeof(int32_t));
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
		free(filter->counters);
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
	//variable que contiene el vector m de contadores
	int32_t *counters = filter->counters;
	//se itera sobre las funciones de hashing
	int i = 0;
	while (h) {
		//se obtiene el valor del hash
		uint64_t hash = h->func(item,i);
		//hash= hash % filter.size
		hash = hash % (filter->size);
		//printf("Se incrementa el contador de la posicion %ld\n",hash);
		//se incrementa el contador
		counters[hash] = counters[hash] + 1;
		h = h->next;
		i++;
	}
}

//TODO: funcion que elimina el elemento item del filtro filter
void bloom_remove (bloom_t filter, const void *item) {
	struct bloom_hash *h = filter->func;
	//variable que contiene el vector m de contadores
	int32_t *counters = filter->counters;
	//se itera sobre las funciones de hashing
	int i = 0;
	while (h) {
		//se obtiene el valor del hash
		uint64_t hash = h->func(item,i);
		//hash= hash % filter.size
		hash = hash % (filter->size);
		//se incrementa el contador
		//printf("Se incrementa el contador de la posicion %d\n",hash);
		counters[hash] = counters[hash] - 1;
		h = h->next;
		i++;
	}
}


//funcion que evalua la pertenencia del elemento item al filtro filter
bool bloom_test(bloom_t filter, const void *item) {
	struct bloom_hash *h = filter->func;
	int32_t *counters = filter->counters;
	int i = 0;
	while (h) {
		uint64_t hash = h->func(item,i);
		hash %= filter->size;
		if (counters[hash] == 0) { 
			return false;
		}
		h = h->next;
		i++;
	}
	return true;
}

void filter_dump(bloom_t filter) {
	int32_t* counters = filter->counters;
	size_t i = 0;
	while(i<filter->size){
		printf("The counter at position %zu has the value %d\n", i,counters[i]);
		i++;
	}
}
