bloom: main.c bloom.c
	cc -g -o bloom main.c bloom.c

clean:
	rm bloom
