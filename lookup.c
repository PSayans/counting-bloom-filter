#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <openssl/sha.h>

bloom_t filter;
unsigned int seed_t;
unsigned int seed_f;

/*function that uses openssl implementation of MD5 and obtains an unsigned int which is later passed
  as an unsigned integer to the filter, which converts it to a position. Because MD5 returns a disgest longer than the one needed, it is splitted
  in two and passed as different position when the filter is configured to use this function two times */
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
	else if (round == 1){
		for (int i = 8; i<16; i++) {
			hash  = hash | ((uint64_t)digest[i] << (8*(i-8)));
		}
	}
	return hash;
}

/*function that uses openssl implementation of SHA1 and obtains an unsigned int which is later passed
  as an unsigned integer to the filter, which converts it to a position. Because SHA1 returns a disgest longer than the one needed, it is splitted
  in two and passed as different position when the filter is configured to use this function two times */
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
			hash  = hash | ((uint64_t)digest[i] << (8*(i-8)));
		}
	}
	return hash;
 }

/*This function receives a vector F and its length and returns the FPP for the filter using F*/
float measure_fpp (char** f, int vectorLen) {
	int fpp_counter=0;

	for (int i=0; i<vectorLen; i++){
		if (bloom_test(filter,f[i])){
			fpp_counter++;
		}
	}
	float f_fpp = ((float) fpp_counter / vectorLen);
	return f_fpp;
}

/* Generates a random vector with certain length to use it as F or T. Because of the use of the rand function impplemented in libc, the maximum 
number of random elements with the same seed is aprox. 32000. This is why this function uses the same random number for different elements of the vector*/
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

/* Frees the memory allocated for certain vector */
void destroy_random_vector(char** vector, int lenght){
	for(int i=0; i<lenght;i++){
			free(vector[i]);
		} 
	free(vector);
}

/* Implementation of the lookup attack. For more information,please read the README file */
int main(int argc, char* argv[]) {

	
	if (argc == 2 && strncmp(argv[1],"-h",2)==0){
		printf("%s\n","Expected arguments: ");
		printf("%s\n","1: Length of vector F.");
		printf("%s\n","2: Length of vector T.");
		printf("%s\n","3: Number of rounds.");
		printf("%s\n","4: Size of the Bloom filter.");
		printf("%s\n","5: Number of hashes.");
		printf("%s\n","6: Output file for the results.");
		printf("%s\n","7: Type of progression for T (linear or fixed).");
		printf("%s\n","8: Factor of progression when linear mode selected (just type 0 when fixed legth used).");
	}

	if (argc != 9){
		printf("%s%d\n", "Error: incorrect number of arguments. Expected 8 but received ", argc);
		return -1;
	}

	seed_f=30000;
	seed_t=0;

	int vectorLen_f = atoi(argv[1]);
	double vectorLen_t;
	int n_rounds = atoi(argv[3]);
	size_t filter_size = atoi(argv[4]);
    size_t number_of_hashes = atoi(argv[5]);
	char*results_file=argv[6];

   	filter = bloom_create(filter_size);

    if (number_of_hashes ==2) {
        bloom_add_hash(filter,md5);
	    bloom_add_hash(filter,md5);
    }

	else if (number_of_hashes == 3){
        bloom_add_hash(filter,md5);
	    bloom_add_hash(filter,md5);
        bloom_add_hash(filter,sha1);
    }

    else if (number_of_hashes == 4){
        bloom_add_hash(filter,md5);
	    bloom_add_hash(filter,md5);
        bloom_add_hash(filter,sha1);
        bloom_add_hash(filter,sha1);
    }
	int rounds_counter = 0;
	char ** f = generate_random_vector(vectorLen_f,'f');
	double alfa = +atof(argv[8]);
	bool geometrico;

	if (strncmp(argv[7],"linear",11)==0){
		geometrico=true;
		vectorLen_t=1;
	}
	else{
		geometrico=false;
		vectorLen_t = atof(argv[2]);
	}

	clock_t begin=clock();
	while (rounds_counter < n_rounds){
		
		float delta_max = 0;
		float fpp_after;
		char * best_element=malloc(8);
		char ** t;
		float fpp_before = measure_fpp(f, vectorLen_f);
		
		if (!geometrico){
			t = generate_random_vector((int)vectorLen_t,'t');
		}

		else {
			vectorLen_t = vectorLen_t + alfa;
			t = generate_random_vector((int)vectorLen_t,'t');
		}

		best_element=t[0];
		//aplicamos el algoritmo
		for (int i =0; i < (int)vectorLen_t; i++){
			
			char * element = t[i];
			bloom_add(filter, element);
			fpp_after = measure_fpp(f, vectorLen_f);

			if (fpp_before >= 1){
				printf("Filter polluted.\n");
				rounds_counter=n_rounds;
				break;
			}
			float delta = fpp_after-fpp_before;

			if (delta > delta_max){
				strncpy(best_element,element,8);
				delta_max=delta;
			}
			bloom_remove(filter,element);
		}
		bloom_add(filter,best_element);
		destroy_random_vector(t,(int)vectorLen_t);
		rounds_counter++;
	}

	clock_t end = clock();
	
	destroy_random_vector(f,vectorLen_f);
	float time_spent = (float)(end-begin) / CLOCKS_PER_SEC;
	printf("%s%f%s\n","Execution time: ", time_spent, " seconds");	
	filter_dump(filter);
	char** random_vector = generate_random_vector(vectorLen_f, 't');
	float final_fpp = measure_fpp(random_vector, vectorLen_f);
	printf("%s%f%s%d%s", "The FPP for the random vector at the end of the execution is:", final_fpp," in round",rounds_counter, "\n");
	bloom_free(filter);

	FILE *fp;
	fp = fopen(results_file,"a");
	fprintf(fp,"%d%s%d%s%ld%s%ld%s%s%s%f%s%fl%s%fl\n", (int)vectorLen_t,",",n_rounds,",",filter_size,",",number_of_hashes,",",
		argv[7],",",alfa,",",final_fpp,",",time_spent);
	fclose(fp);
	return 0;
}