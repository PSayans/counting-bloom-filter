#ifndef _BLOOM_H
#define _BLOOM_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


typedef uint64_t (*hash_function)(const void *data, int round);
typedef struct bloom_filter * bloom_t;

/* Creates a new counting bloom filter with no hash functions*/
bloom_t bloom_create(size_t size);

/*Removes an element from the filter*/
void bloom_remove (bloom_t filter, const void *item);

/* Frees a bloom filter. */
void bloom_free(bloom_t filter);

/* Adds a hashing function to the bloom filter. All of the
 * used functions must be added before populating the filter. 
 * This implementation allows to pass the current hashing round to the 
 * function to use different chunks of the digest as positions of the filter*/
void bloom_add_hash(bloom_t filter, hash_function func);

/* Adds an item to the bloom filter. */
void bloom_add(bloom_t filter, const void *item);

/* Tests if an item is in the bloom filter.
 *
 * Returns false if the item has definitely not been added before. Returns true
 * if the item was probably added before. */
bool bloom_test(bloom_t filter, const void *item);

/* DUmps the status of the counters of the filter for debugging purposes*/
void filter_dump(bloom_t filter);

#endif
