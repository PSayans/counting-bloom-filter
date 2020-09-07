#include <stdio.h>
#include <math.h>
#include <stdlib.h>

double calculate_ideal_fpp(double k, double m, double n){

	return pow(1 - exp(-k / (m/n)),k);
}


int main(int argc, char* argv[]) {


	int n_rounds = atoi(argv[1]);
   	size_t filter_size = atoi(argv[2]);

    FILE *fp;
	fp = fopen("fpp.txt","w+");

    for (int i=0; i<n_rounds;i++){
        double fpp = calculate_ideal_fpp((double)2,(double)filter_size,(double)i);
		fprintf(fp,"%d%s%f\n", i,",",fpp);
    }

    fclose(fp);
    return 0;

}