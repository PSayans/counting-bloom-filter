#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

double calculate_ideal_fpp(double k, double m, double n){

	return pow((n*k)/m,k);
}


int main(int argc, char* argv[]) {

    int hash_number = atoi(argv[1]);
	int n_rounds = atoi(argv[2]);
   	size_t filter_size = atoi(argv[3]);
    char * mode = argv[4];
    size_t size_distance= atoi(argv[5]);

    if (strncmp(mode,"query",5)==0){
        double fpp = calculate_ideal_fpp((double)hash_number,(double)filter_size,(double)n_rounds);
		printf("%f\n",fpp);
        return 0;
    }

    FILE *fp;
	fp = fopen("fpp.txt","w+");
	clock_t begin = clock();


    if (strncmp(mode,"dataset",5)==0){
        for (int i=0; i<n_rounds;i++){
            double fpp = calculate_ideal_fpp((double)hash_number,(double)filter_size,(double)i);
		    fprintf(fp,"%d%s%f\n", i,",",fpp);
        }
        return 0;
    }

    for (int i=0; i<n_rounds;i++){
        double fpp = calculate_ideal_fpp((double)hash_number,(double)filter_size,(double)i);
		fprintf(fp,"%d%s%f\n", i,",",fpp);
    }
	clock_t end = clock();

    double time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
    printf("%s%f%s\n","Tiempo de ejecución: ", time_spent, " segundos");	

    fclose(fp);
    return 0;

}