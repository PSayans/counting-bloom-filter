#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>

//bloom_add_hash(bloom,md5)
 uint64_t md5 (const void *_str, int round) {

	MD5_CTX c;
	MD5_Init(&c);
	int length=strnlen(_str,64);
    char digest[16];

    MD5_Init(&c);
	MD5_Update(&c, _str, length);
    MD5_Final(digest, &c);

	uint64_t hash=0;

	if (round == 0){
		for (int i = 0; i<8; i++) {
			hash = hash + digest[i];
		}
	}
	else {
		for (int i = 8; i<16; i++) {
			hash = hash + digest[i];
		}
	}
	printf("%s%zu%s%d%s","Valor de retorno del hash:",hash," en ronda:",round,"\n");
	//some shit to cast it to int
	return hash;
}

int main() {
	bloom_t bloom = bloom_create(128);
	bloom_add_hash(bloom,md5);
	bloom_add_hash(bloom,md5);
	printf("Should be 0: %d\n", bloom_test(bloom, "hello world"));
	bloom_add(bloom, "hello world");
	bloom_add(bloom, "mi");
	bloom_add(bloom, "gato");
	filter_dump(bloom);
	bloom_add(bloom, "se");
	bloom_add(bloom, "llama");
	bloom_add(bloom, "guantes");
	filter_dump(bloom);
	printf("Should be 1: %d\n", bloom_test(bloom, "hello world"));
	bloom_remove(bloom, "se");
	bloom_remove(bloom, "llama");
	bloom_remove(bloom, "guantes");
	filter_dump(bloom);
	printf("Should (probably) be 0: %d\n", bloom_test(bloom, "world hello"));



	//generar un vector aleatorio de tamaño 64 bits por elemento y de longitud pasada por parámetro
	//calcular el FPP de este vector sobre un filtro vacio
	/* for n iteraciones:
		-generar vector aleatorio de longitud X
		-insertar elemento
		-calcular delta
		-comparar con el mejor delta existente
		-si es mejor que el mejor se convierte en el nuevo mejor
		-eliminamos el elemento
	*/

	return 0;
}


