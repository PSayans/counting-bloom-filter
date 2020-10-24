#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "bloom.h"


//linked list containing the pointers to the hashing functions
struct bloom_hash {
    hash_function func;
    struct bloom_hash *next;
};


//BF struct with the list of hashesh, bit array and size
struct bloom_filter {
    struct bloom_hash *func;
    int32_t *counters;
    size_t size;
};


//creates a filter with specified size
bloom_t bloom_create(size_t size) {
	bloom_t res = calloc(1, sizeof(struct bloom_filter));
	res->size = size;
	res->counters = calloc(size, sizeof(int32_t));
	return res;
}


//destroys the filter
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

//adds hash function to the list of hashes of the filter
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

//adds an element to the filter passed by argument
void bloom_add(bloom_t filter, const void *item) {
	struct bloom_hash *h = filter->func;
	int32_t *counters = filter->counters;
	int i = 0;
	while (h) {
		uint64_t hash = h->func(item,i);
		hash = hash % (filter->size);
		counters[hash] = counters[hash] + 1;
		h = h->next;
		i++;
	}
}

//removes an element from the filter
void bloom_remove (bloom_t filter, const void *item) {
	struct bloom_hash *h = filter->func;
	int32_t *counters = filter->counters;
	int i = 0;
	while (h) {
		uint64_t hash = h->func(item,i);
		hash = hash % (filter->size);
		counters[hash] = counters[hash] - 1;
		h = h->next;
		i++;
	}
}

//queries the filter to determine whether the element passed is a member or not
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

//For debugging purposes. Dumps the status of the counters of the filter
void filter_dump(bloom_t filter) {
	int32_t* counters = filter->counters;
	size_t i = 0;
	FILE *fp;
	fp = fopen("filter_output.txt","w+");
	while(i<filter->size){
		fprintf(fp,"%zu%s%d\n", i,",",counters[i]);
		i++;
	}
	fclose(fp);
}
