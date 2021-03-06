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
unsigned int seed_t;
unsigned int seed_f;

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
	if (round == 2){
		for (int i = 0; i<8; i++) {
			hash  = hash | ((uint64_t)digest[i] << (8*i)); 
		}
	}
	else if (round == 3){
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

char ** generate_random_vector(int length, char vector_type) {
	char ** vector;

	vector = malloc(length * sizeof(char*));

	for (int i = 0; i < length; i++)
		vector[i] = malloc(8 * sizeof(unsigned char));

	if (vector_type=='t'){
		srand((unsigned int) seed_t);
		seed_t++;
	}
	else if (vector_type=='f'){
		srand((unsigned int) seed_f);
		seed_f++;
	}
	else{
		srand((unsigned int) time(NULL));
	}

	for (int i = 0; i < length; i++){
		unsigned char element[8];
		for (int k=0;k<8;k=k+4){
			unsigned int ran=(unsigned int)random();
			element[k] = (unsigned) (ran >> 24) & 0xff;
			element[k+1] = (unsigned) (ran >> 16) & 0xff;
			element[k+2] = (unsigned) (ran >> 8) & 0xff;
			element[k+3] = (unsigned) ran & 0xff;
		}
		strncpy(vector[i],element,8);
	}

	return vector;
}

void destroy_random_vector(char** vector, int lenght){
	for(int i=0; i<lenght;i++){
			free(vector[i]);
		} 
	free(vector);
}

int main(int argc, char* argv[]) {

	seed_f=30000;
	seed_t=0;
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

	char ** f = generate_random_vector(vectorLen_f,'f');
	int rounds_counter = 0;

	clock_t begin=clock();
    double best_delta = 0;
	while (rounds_counter < n_rounds){
		
		double *delta_max = malloc(sizeof(double));
		double aux = 0;
		delta_max = &aux;
		double fpp_after;
        bool inserted=false;

		char * best_element=malloc(8);
		char ** t;
		double fpp_before = measure_fpp(f, vectorLen_f);
		
		//we generate a random set of elements T
		//generamos el vector T
		//	rounds_counter, ".\n");
		
		t = generate_random_vector(vectorLen_t,'t');

		best_element=t[0];
		//aplicamos el algoritmo
		for (int i =0; i < vectorLen_t; i++){
			
			char * element = t[i];
			bloom_add(filter, element);
			fpp_after = measure_fpp(f, vectorLen_f);
			if (fpp_before >= 1){
				printf("Filtro polucionado\n");
				rounds_counter=n_rounds;
				break;
			}
			double delta = fpp_after-fpp_before;
			if (delta > best_delta){
				strncpy(best_element,element,8);
				best_delta=delta;
                inserted=true;
				break;
			}
            if (delta > *delta_max){
				strncpy(best_element,element,8);
				*delta_max=delta;
			}
			bloom_remove(filter,element);
		}
        if(inserted){
            inserted=false;
        }
        else{
            bloom_add(filter,best_element);
            best_delta = *delta_max;      
            inserted=false;
        }
		//printf("%s%f%s%d%s", "El FPP para el vector F es:", fpp_after," en la ronda ",rounds_counter, "\n");
		//printf("%s%s\n", "el elemento insertado es:",best_element);
		bloom_add(filter,best_element);
		destroy_random_vector(t,vectorLen_t);
		rounds_counter++;
	}

	clock_t end = clock();
	destroy_random_vector(f,vectorLen_f);
	double time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
	printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");	
	filter_dump(filter);
	char** random_vector = generate_random_vector(vectorLen_f,'r');
	double final_fpp = measure_fpp(random_vector, vectorLen_f);
	printf("%s%f%s%d%s", "El FPP para el vector Z al final de la ejecución es:", final_fpp," en la ronda ",rounds_counter, "\n");
	bloom_free(filter);
    free(f);
    FILE *fp;
	fp = fopen(results_file,"a");
	//fprintf(fp,"%s\n","t,n,m,k,fpp,time");
	fprintf(fp,"%d%s%d%s%ld%s%ld%s%fl%s%fl\n", vectorLen_t,",",n_rounds,",",filter_size,",",number_of_hashes,",",final_fpp,",",time_spent);
	fclose(fp);
	return 0;
}