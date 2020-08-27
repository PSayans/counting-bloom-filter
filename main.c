#include "bloom.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>


//bloom_add_hash(bloom,md5)
unsigned int md5 (const void *_str, int round) {

	MD5_CTX c;
	MD5_Init(&c);
	int length=64;

	int n;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, _str, 512);
        } else {
            MD5_Update(&c, _str, length);
        }
        length -= 512;
        _str += 512;
    }

    MD5_Final(digest, &c);

	unsigned int hash,v1,v2,v3,v4;
	sscanf(digest[0], "%x", &v1);
	sscanf(digest[8], "%x", &v2);
	sscanf(digest[16], "%x", &v3);
	sscanf(digest[24], "%x", &v4);

	if (round == 0){
		hash= v1^v2;
	}
	else {
		hash=v3^v4;
	}

	//some shit to cast it to int
	return hash;
}


int main() {
	
	bloom_t bloom = bloom_create(8);
	bloom_add_hash(bloom,md5);
	printf("Should be 0: %d\n", bloom_test(bloom, "hello world"));
	bloom_add(bloom, "hello world");
	bloom_add(bloom, "mi");
	bloom_add(bloom, "gato");
	filter_dump(bloom);
	bloom_add(bloom, "se");
	bloom_add(bloom, "llama");
	bloom_add(bloom, "guantes");
	filter_dump(bloom);
	printf("Should be 1: %d\n", bloom_test(bloom, "hello world"));
	bloom_remove(bloom, "se");
	bloom_remove(bloom, "llama");
	bloom_remove(bloom, "guantes");
	filter_dump(bloom);
	printf("Should (probably) be 0: %d\n", bloom_test(bloom, "world hello"));
	return 0;
}


