bloom: bloom.c
	cc -g bloom.c -o bloom.o

lookup: lookup.c bloom.o
	cc -g -o lookup.o -Wall lookup.c -lm -lcrypto -lssl bloom.o 

prediction_algorithm: prediction_algorithm.c bloom.o
	cc -g -o prediction_algorithm.o -Wall prediction_algorithm.c -lm -lcrypto -lssl bloom.o

prediction_algorithm_modified: prediction_algorithm_modified.c bloom.o
	cc -g -o prediction_algorithm_modified.o -Wall prediction_algorithm_modified.c -lm -lcrypto -lssl bloom.o

improve_delta: improve_delta.c bloom.o
	cc -g -o improve_delta.o -Wall improve_delta.c -lm -lcrypto -lssl bloom.o 

clean:
	rm prediction_algorithm.o lookup.o improve_delta.o prediction_algorithm_modified.o
