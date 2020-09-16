#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


bloom_t filter;
int vectorLen_f;
int vectorLen_t;
unsigned int seed = 0;

struct t_fpp {
	int filter_size;
	int hash_number;
	float fpp;
	struct t_fpp* next;
};

struct t_fpp* load_fpps(char* path){
	FILE *fp;
	fp = fopen(path,"r");
	char * buffer;

	struct t_fpp* last=NULL;
	struct t_fpp* first=NULL;
	int read;
	size_t length = 15;

	while ((read=getline(&buffer,&length,fp) !=-1)){
		char* filter_size = strtok(buffer,",");
		char* hash_number = strtok(NULL,",");
		char* fpp = strtok(NULL,",");

		struct t_fpp *struct_fpp = (struct t_fpp*) malloc(sizeof(struct t_fpp));
		struct_fpp->filter_size=atoi(filter_size);
		struct_fpp->hash_number=atoi(hash_number);
		struct_fpp->fpp=strtod(fpp,NULL);
		struct_fpp->next=0;
		
		if(!last){
			last=struct_fpp;
		}
		if(!first){
			first=struct_fpp;
		}
		last=first;
		while (last->next != 0) {
			last = last->next;
		}
		last->next=struct_fpp;	
		last=struct_fpp;
		last->next=NULL;
	}
	//printf("%d\n",last->next);

	return first;
}

struct t_fpp predict_filter_type(float fpp, struct t_fpp* list){

	struct t_fpp* last=list;
	struct t_fpp best_node;

	float best_delta=fabs(fpp-last->fpp);
	float delta;

	while(last->next != NULL){
		delta=fabs(fpp-last->fpp);
		printf("%s%fl\n","Diferencia:",delta);
		if (delta<best_delta){
			best_delta=delta;
			best_node=*last;
		}
		last= last->next;
	}
	printf("%s%fl\n","Contenido del FPP:",last->fpp);
	delta=fabs(fpp-last->fpp);
	printf("%s%fl\n","Diferencia:",delta);
	if (delta<best_delta){
		best_delta=delta;
		best_node=*last;
	}
	return best_node;
}


float calculate_ideal_fpp(float k, float m, float n){

	return pow((n*k)/m,k);
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
	for (int i = 0; i<8; i++) {
		hash  = hash | ((uint64_t)digest[i] << (8*i)); 
	}
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

char ** generate_random_vector(int length) {
	char ** vector;

	vector = malloc(length * sizeof(char*));

	for (int i = 0; i < length; i++)
		vector[i] = malloc(8 * sizeof(char));

	srand((unsigned int) seed);
	seed++;

	for (int i = 0; i < length; i++){

		char element[8]="";
		for (int k=0;k<8;k++){
			element[k]=random();
		}
		strncat(vector[i],element,8);
	}

	return vector;
}

int main(int argc, char* argv[]) {

	//generar un vector aleatorio de tamaño 64 bits por elemento y de longitud pasada por parámetro
	vectorLen_f = atoi(argv[1]);
	vectorLen_t = atoi(argv[2]);
	int n_rounds = atoi(argv[3]);
	size_t filter_size = atoi(argv[4]);
	int lookup_rounds=atoi(argv[5]);
	size_t number_of_hashes = atoi(argv[6]);
	char*results_file=argv[7];

	//printf("%f\n",ideal_fpp);

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

	printf("Preparamos el dataset de FPPs ideales");
	
	//leer el dataset de fpps
	struct t_fpp* list = load_fpps("fpp.txt");

	/*------------------Inicio Algoritmo Lookup-----------------*/

	int rounds_counter = 0;
	clock_t begin=clock();
	while (rounds_counter < lookup_rounds){
		
		float *delta_max = malloc(sizeof(float));
		float aux = 0;
		delta_max = &aux;
		float fpp_after;

		char * best_element=malloc(8);
		char ** t;
		float fpp_before = measure_fpp(f, vectorLen_f);
		
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
			if (fpp_after >= 1){
				clock_t end = clock();
				float time_spent = (float)(end-begin) / CLOCKS_PER_SEC;
				printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");
				printf("Filtro polucionado\n");
				filter_dump(filter);
				bloom_free(filter);
				return 0;
			}
			float delta = fpp_after-fpp_before;
			//float ideal_fpp = calculate_ideal_fpp((float)2,(float)filter_size,(float)rounds_counter);
			//printf("%f\n",ideal_fpp);
			if (delta > *delta_max){
			//if (fpp_after >= ideal_fpp){

				strncpy(best_element,element,8);
				*delta_max=delta;
			}
			bloom_remove(filter,element);

		}
		printf("%s%f%s%d%s", "El FPP para el vector F es:", fpp_after," en la ronda ",rounds_counter, "\n");
		bloom_add(filter,best_element);
		free(best_element);
		free(t);
		rounds_counter++;
	}

	/*------------------Fin Algoritmo Lookup-----------------*/

	char** random_vector = generate_random_vector(vectorLen_f);
	float final_lookup_fpp = measure_fpp(random_vector, vectorLen_f);
	struct t_fpp p_predicted_filter = predict_filter_type(final_lookup_fpp,list);
	
	struct t_fpp* predicted_filter = &p_predicted_filter;
	printf("%s%d%s%d\n", "El filtro detectado corresponde a k=", predicted_filter->hash_number, ", m=", predicted_filter->filter_size);

	/*------------------Inicio Algoritmo fpp ideal-----------------*/

	float ideal_fpp_before = calculate_ideal_fpp((float)predicted_filter->hash_number,(float)predicted_filter->filter_size,
				(float)rounds_counter-1);

	float ideal_fpp_after;
	float fpp_before;
	float fpp_after;
	bool inserted= false;
	float delta_array[predicted_filter->hash_number];


	//definimos un array con tantos elementos tenga K para comprobar las deltas


	while (rounds_counter < n_rounds){
			
		char * best_element=malloc(8);
		char ** t;
		float *delta_max = malloc(sizeof(float));
		float best_k_delta=0;
		float aux = 0;
		delta_max = &aux;
		int k_delta_counter=0;

		fpp_before=measure_fpp(f, vectorLen_f);

		ideal_fpp_after = calculate_ideal_fpp((float)predicted_filter->hash_number,(float)predicted_filter->filter_size,
				(float)rounds_counter);

		float ideal_delta=ideal_fpp_after-ideal_fpp_before;

		t = generate_random_vector(vectorLen_t);
		best_element=t[0];
		//aplicamos el algoritmo
		for (int i =0; i < vectorLen_t; i++){
			
			char * element = t[i];
			bloom_add(filter, element);
			fpp_after = measure_fpp(f, vectorLen_f);
			if (fpp_after >= 1){
				clock_t end = clock();
				float time_spent = (float)(end-begin) / CLOCKS_PER_SEC;
				printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");
				printf("Filtro polucionado\n");
				filter_dump(filter);
				bloom_free(filter);
				return 0;
			}
			float delta=fpp_after-fpp_before;
			//printf("%f\n",ideal_fpp);
			if (delta>=ideal_delta){
				strncpy(best_element,element,8);
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
			inserted=false;
		}
		printf("%s%f%s%d%s", "El FPP para el vector F es:", fpp_after," en la ronda ",rounds_counter, "\n");
		free(best_element);
		free(t);
		rounds_counter++;
		ideal_fpp_before=ideal_fpp_after;
	}

	/*------------------Fin Algoritmo fpp ideal-----------------*/


	clock_t end = clock();

	char** random_vector2 = generate_random_vector(vectorLen_f);
	float final_fpp = measure_fpp(random_vector2, vectorLen_f);

	float time_spent = (float)(end-begin) / CLOCKS_PER_SEC;
	printf("%s%d%s%d\n", "El filtro detectado corresponde a k=", predicted_filter->hash_number, ", m=", predicted_filter->filter_size);
	printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");	
	filter_dump(filter);
	printf("%s%f%s%d%s", "El FPP para el vector Z al final de la ejecución es:", final_fpp," en la ronda ",rounds_counter, "\n");
	bloom_free(filter);
	free(f);
    FILE *fp;
	fp = fopen("results_prediction.txt","a");
	fprintf(fp,"%d%s%d%s%d%s%d%s%fl%s%fl\n", vectorLen_t,",",n_rounds,",",filter_size,",",number_of_hashes,",",final_fpp,",",time_spent);
	fclose(fp);
	return 0;
}


