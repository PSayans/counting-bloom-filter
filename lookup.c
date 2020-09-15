#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <openssl/sha.h>


bloom_t filter;
int vectorLen_f;
int vectorLen_t;
unsigned int seed=0;

struct t_fpp {
	size_t position;
	double fpp;
	struct t_fpp* next;
};


double calculate_ideal_fpp(double k, double m, double n){

	//return pow(1 - exp(-k / (m/n)),k);
	return pow(n*k/m,k);
}

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

 uint64_t sha1 (const void *_str, int round) {
	 
	int length=strnlen(_str,64);
	unsigned char digest[20];
	
	SHA1(_str,length,digest);
	uint64_t hash=0;
	if (round == 3){
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
	return hash;
 }

double measure_fpp (char** f, int vectorLen) {
	int fpp_counter=0;

	for (int i=0; i<vectorLen; i++){
		if (bloom_test(filter,f[i])){
			fpp_counter++;

		}
	}
	double f_fpp = ((double) fpp_counter / vectorLen);

	return f_fpp;
}

char ** generate_random_vector(int length) {
	char ** vector;

	vector = malloc(length * sizeof(char*));

	for (int i = 0; i < length; i++)
		vector[i] = malloc(8 * sizeof(char));

	srand((unsigned int) seed);
	seed++;

	for (int i = 0; i < length; i++){
		char element[8];
		for (int k=0;k<8;k++){
			element[k]=random();
		}
		strncpy(vector[i],element,8);
	}

	return vector;
}

int main(int argc, char* argv[]) {

	//generar un vector aleatorio de tamaño 64 bits por elemento y de longitud pasada por parámetro
	vectorLen_f = atoi(argv[1]);
	vectorLen_t = atoi(argv[2]);
	int n_rounds = atoi(argv[3]);
	size_t filter_size = atoi(argv[4]);
    size_t number_of_hashes = atoi(argv[5]);
	char*results_file=argv[6];

   	filter = bloom_create(filter_size);

    if (number_of_hashes ==2) {
        bloom_add_hash(filter,md5);
	    bloom_add_hash(filter,md5);
    }

	else if (number_of_hashes ==3){
        bloom_add_hash(filter,md5);
	    bloom_add_hash(filter,md5);
        bloom_add_hash(filter,sha1);
    }

    else if (number_of_hashes ==4){
        bloom_add_hash(filter,md5);
	    bloom_add_hash(filter,md5);
        bloom_add_hash(filter,sha1);
        bloom_add_hash(filter,sha1);
    }
	char ** f = generate_random_vector(vectorLen_f);

	printf("Vector F generado.\n");

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

	clock_t begin=clock();
	while (rounds_counter < n_rounds){
		
		double *delta_max = malloc(sizeof(double));
		double aux = 0;
		delta_max = &aux;
		double fpp_after;

		char * best_element=malloc(8);
		char ** t;
		double fpp_before = measure_fpp(f, vectorLen_f);
		
		//we generate a random set of elements T
		//generamos el vector T
		//	rounds_counter, ".\n");
		
		t = generate_random_vector(vectorLen_t);

		best_element=t[0];
		//aplicamos el algoritmo
		for (int i =0; i < vectorLen_t; i++){
			
			char * element = t[i];
			bloom_add(filter, element);
			fpp_after = measure_fpp(f, vectorLen_f);
			if (fpp_before >= 1){
				clock_t end = clock();
				printf("Filtro polucionado\n");
				double time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
				printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");	
				filter_dump(filter);
				char** random_vector = generate_random_vector(vectorLen_f);
				double final_fpp = measure_fpp(random_vector, vectorLen_f);
				printf("%s%f%s%d%s", "El FPP para el vector Z al final de la ejecución es:", fpp_before," en la ronda ",rounds_counter, "\n");
				bloom_free(filter);
				FILE *fp;
				fp = fopen(results_file,"a");
				//fprintf(fp,"%s\n","t,n,m,k,fpp,time");
				fprintf(fp,"%d%s%d%s%d%s%d%s%fl%s%fl\n", vectorLen_t,",",n_rounds,",",filter_size,",",number_of_hashes,",",final_fpp,",",time_spent);
				fclose(fp);
				return 0;
			}
			double delta = fpp_after-fpp_before;
			//double ideal_fpp = calculate_ideal_fpp((double)2,(double)filter_size,(double)n_rounds);

			if (delta > *delta_max){
				strncpy(best_element,element,8);
				*delta_max=delta;
			}
			bloom_remove(filter,element);

		}
		//printf("%s%f%s%d%s", "El FPP para el vector F es:", fpp_after," en la ronda ",rounds_counter, "\n");
		//printf("%s%s\n", "el elemento insertado es:",best_element);
		bloom_add(filter,best_element);
		free(best_element);
		free(t);
		rounds_counter++;
	}

	clock_t end = clock();

	double time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
	printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");	
	filter_dump(filter);
	char** random_vector = generate_random_vector(vectorLen_f);
	double final_fpp = measure_fpp(random_vector, vectorLen_f);
	printf("%s%f%s%d%s", "El FPP para el vector Z al final de la ejecución es:", final_fpp," en la ronda ",rounds_counter, "\n");
	bloom_free(filter);

	FILE *fp;
	fp = fopen(results_file,"a");
	//fprintf(fp,"%s\n","t,n,m,k,fpp,time");
	fprintf(fp,"%d%s%d%s%d%s%d%s%fl%s%fl\n", vectorLen_t,",",n_rounds,",",filter_size,",",number_of_hashes,",",final_fpp,",",time_spent);
	fclose(fp);
	return 0;
}