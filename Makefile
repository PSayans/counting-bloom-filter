bloom: bloom.c
	cc -g bloom.c -o bloom.o

main: main.c bloom.o
	cc -g -o main -Wall main.c -lcrypto -lssl bloom.o 

clean:
	rm main
