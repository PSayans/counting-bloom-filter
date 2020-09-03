#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bloom_t filter;
int vectorLen_f;
int vectorLen_t;

//bloom_add_hash(bloom,md5)
 uint64_t md5 (const void *_str, int round) {

	MD5_CTX c;
	MD5_Init(&c);
	int length=strnlen(_str,64);
    unsigned char digest[MD5_DIGEST_LENGTH];

    MD5_Init(&c);
	MD5_Update(&c, _str, length);
    MD5_Final(digest, &c);

	uint64_t hash=0;
	//printf("%s%s\n","salida del digest: ",digest);
	if (round == 0){
		for (int i = 0; i<8; i++) {
			hash  = hash | ((uint64_t)digest[i] << (8*i)); 
		}
	}
	else {
		for (int i = 8; i<16; i++) {
			//hash = hash + digest[i];
			hash  = hash | ((uint64_t)digest[i] << (8*(i-8)));
		}
	}
	//printf("%s%zu%s%d%s","Valor de retorno del hash:",hash," en ronda:",round,"\n");
	return hash;
}

double measure_fpp (char** f, int vectorLen) {
	int fpp_counter=0;

	for (int i=0; i<vectorLen; i++){
		if (bloom_test(filter,f[i])){
			fpp_counter++;
			//printf("%s%f%s","He encontrado un FP! lookup delta: ", lookup_delta, "\n");

		}
	}
	double f_fpp = ((double) fpp_counter / vectorLen);

	return f_fpp;
}

int main(int argc, char* argv[]) {
	
	//generar un vector aleatorio de tamaño 64 bits por elemento y de longitud pasada por parámetro
	vectorLen_f = atoi(argv[1]);
	vectorLen_t = atoi(argv[2]);
	int n_rounds = atoi(argv[3]);
	size_t filter_size = atoi(argv[4]);

	filter = bloom_create(filter_size);
	bloom_add_hash(filter,md5);
	bloom_add_hash(filter,md5);

	char ** f;

	f = malloc(vectorLen_f * sizeof(char*));

	for (int i = 0; i < vectorLen_f; i++)
		f[i] = malloc(8 * sizeof(char));

	//char f [vectorLen][8];
	printf("%s%d%s","Se genera el vector aleatorio F con ", vectorLen_f, " posiciones de 64 bits cada una.\n");
	for (int i = 0; i < vectorLen_f; i++){

		char element[8]="";
		for (int k=0;k<8;k++){
			element[k]=random();
		}
		strncat(f[i],element,8);
		//printf("%s%d%s%s%s","el contenido del array random en pos", i, " es:",f[i],"\n");
	}
	printf("Vector F generado.\n");

	//test de insercion
	/*if (strncmp("true",argv[3],5)==0){
		for (int i = 0; i < vectorLen; i++){
			bloom_add(filter,f[i]);
		}
		filter_dump(filter);
		return 0;
	}
	*/

	//calcular el FPP de este vector sobre un filtro vacio
	//el FPP se calcula midiendo el total de matches/número de elementos probados
		/* for n iteraciones:
		-generar vector aleatorio de longitud X
		-insertar elemento
		-calcular delta
		-comparar con el mejor delta existente
		-si es mejor que el mejor se convierte en el nuevo mejor
		-eliminamos el elemento
	*/
	int rounds_counter = 0;

	while (rounds_counter < n_rounds){
		
		double *delta_max = malloc(sizeof(double));
		double aux = 0;
		delta_max = &aux;
		double fpp_after;

		char * best_element=malloc(8);
		char ** t;
		double fpp_before = measure_fpp(f, vectorLen_f);
		printf("%s%f%s", "El FPP para el vector F es:", fpp_before,"\n");
		
		//we generate a random set of elements T
		//generamos el vector T
		printf("%s%d%s%d%s","Se genera el vector aleatorio T con ", vectorLen_t, " posiciones de 64 bits cada una en la ronda ",
			rounds_counter, ".\n");
		t = malloc(vectorLen_t * sizeof(char*));

		for (int i = 0; i < vectorLen_t; i++) {
			t[i] = malloc(8 * sizeof(char));
		}

		for (int i = 0; i < vectorLen_t; i++){
			char element[8]="";
			for (int k=0;k<8;k++){
				element[k]=random();
			}
			strncat(t[i],element,8);
			//element[k] = random();
			//element[k]|= random() << 32;
			//printf("%s%d%s%s%s","el contenido del array random en pos: ", i, " es:",t[i],"\n");
		}
		printf("Vector T generado.\n");
		best_element=t[0];
		//aplicamos el algoritmo
		for (int i =0; i < vectorLen_t; i++){
			
			char * element = t[i];
			bloom_add(filter, element);
			fpp_after = measure_fpp(f, vectorLen_f);
			if (fpp_before >= 1){
				printf("Filtro polucionado\n");
				return 0;
			}
			double delta = fpp_after-fpp_before;
			//printf("%s%f\n","valor de delta:",delta);
			//printf("%s%f\n","valor de delta_max:",delta_max);

			if (delta > *delta_max){
				printf("hemos encontrado un candidato\n");
				strncpy(best_element,element,8);
				*delta_max=delta;
			}
			bloom_remove(filter,element);

		}
		printf("%s%f%s%d%s", "El FPP para el vector T es:", fpp_after," en la ronda ",rounds_counter, "\n");
		printf("%s%s\n", "Se inserta el elemento ", best_element);
		printf("%s\n",best_element);
		bloom_add(filter,best_element);
		free(best_element);
		free(t);
		rounds_counter++;
	}
	filter_dump(filter);
	double final_fpp = measure_fpp(f, vectorLen_f);
	printf("%s%f%s%d%s", "El FPP para el vector F al final de la ejecución es:", final_fpp," en la ronda ",rounds_counter, "\n");
	bloom_free(filter);
	return 0;
}


