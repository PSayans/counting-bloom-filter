#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bloom_t filter;
int vectorLen;

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
	//printf("%s%zu%s%d%s","Valor de retorno del hash:",hash," en ronda:",round,"\n");
	//some shit to cast it to int
	return hash;
}

double measure_fpp (char** f, double reference_delta) {
	int fpp_counter=0;

	for (int i=0; i<vectorLen; i++){
		clock_t begin = clock();
		bloom_test(filter,f[i]);
		clock_t end = clock();
		double lookup_delta = (double) (end - begin) / CLOCKS_PER_SEC;
		if (lookup_delta>=reference_delta){
			fpp_counter++;
			//printf("%s%f%s","He encontrado un FP! lookup delta: ", lookup_delta, "\n");

		}
	}
	double f_fpp = ((double) fpp_counter / vectorLen) * 100;

	return f_fpp;
}

int main(int argc, char* argv[]) {
	
	filter = bloom_create(10);
	bloom_add_hash(filter,md5);
	bloom_add_hash(filter,md5);
	
	//generar un vector aleatorio de tamaño 64 bits por elemento y de longitud pasada por parámetro
	vectorLen = atoi(argv[1]);
	int n_rounds = atoi(argv[2]);

	char ** f;

	f = malloc(vectorLen * sizeof(char*));

	for (int i = 0; i < vectorLen; i++)
		f[i] = malloc(8 * sizeof(char));

	//char f [vectorLen][8];
	printf("%s%d%s","Se genera el vector aleatorio F con ", vectorLen, " posiciones de 64 bits cada una.\n");
	for (int i = 0; i < vectorLen; i++){

		char element[8];
		for (int k=0;k<8;k++){
			element[k]=random();
		}
		strncpy(f[i],element,8);
		//printf("%s%d%s%s%s","el contenido del array random en pos", i, " es:",f[i],"\n");
	}
	printf("Vector F generado.\n");
	//calcular el FPP de este vector sobre un filtro vacio
	//el FPP se calcula midiendo el total de matches/número de elementos probados
	bloom_add(filter,"123456789");
	clock_t begin = clock();
	bloom_test(filter,"123456789");
	clock_t end = clock();
	double reference_delta = (double) (end - begin) / CLOCKS_PER_SEC;
	printf("%s%f%s","reference delta: ", reference_delta, "\n");
	bloom_remove(filter,"123456789");
	double fpp_before = measure_fpp(f, reference_delta);
	printf("%s%f%s", "El FPP para el vector F es:", fpp_before,"\n");
	
	/* for n iteraciones:
		-generar vector aleatorio de longitud X
		-insertar elemento
		-calcular delta
		-comparar con el mejor delta existente
		-si es mejor que el mejor se convierte en el nuevo mejor
		-eliminamos el elemento
	*/
	int rounds_counter = 0;
	float delta_max = 0;

	while (rounds_counter < n_rounds){
		
		char * best_element=malloc(8);
		//we generate a random set of elements T
		char ** t;

		t = malloc(vectorLen * sizeof(char*));

		for (int i = 0; i < vectorLen; i++)
			t[i] = malloc(8 * sizeof(char));

		printf("%s%d%s%d%s","Se genera el vector aleatorio T con ", vectorLen, " posiciones de 64 bits cada una en la ronda ",
			rounds_counter, ".\n");
		for (int i = 0; i < vectorLen; i++){
			char element[8];
			for (int k=0;k<8;k++){
				element[k]=random();
			}
			strncpy(t[i],element,8);
			//printf("%s%d%s%s%s","el contenido del array random en pos: ", i, " es:",t[i],"\n");
		}
		printf("Vector T generado.\n");
		
		for (int i =0; i < vectorLen; i++){
			
			char * element = t[i];
			bloom_add(filter, element);
			double fpp_after = measure_fpp(f,reference_delta);
			printf("%s%f%s%d%s", "El FPP para el vector T es:", fpp_after," en la ronda ",rounds_counter, "\n");
			double delta = fpp_after-fpp_before;
			if (delta >= delta_max){
				printf("muero\n");
				strncpy(best_element,element,8);
				delta_max=delta;
			}
			bloom_remove(filter,element);

		}
		printf("%s%s\n", "Se inserta el elemento ", best_element);
		bloom_add(filter,best_element);
		free(best_element);
		rounds_counter++;
	}

	double final_fpp = measure_fpp(f,reference_delta);
	printf("%s%f%s%d%s", "El FPP para el vector F al final de la ejecución es:", final_fpp," en la ronda ",rounds_counter, "\n");

	return 0;
}


