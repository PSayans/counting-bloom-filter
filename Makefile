bloom: bloom.c
	cc -g bloom.c -o bloom.o

lookup: lookup.c bloom.o
	cc -g -o lookup -Wall lookup.c -lm -lcrypto -lssl bloom.o 

prediction_algorithm: prediction_algorithm.c bloom.o
	cc -g -o prediction_algorithm -Wall prediction_algorithm.c -lm -lcrypto -lssl bloom.o 

improve_delta: improve_delta.c bloom.o
	cc -g -o improve_delta -Wall improve_delta.c -lm -lcrypto -lssl bloom.o 

clean:
	rm prediction_algorithm lookup improve_delta
