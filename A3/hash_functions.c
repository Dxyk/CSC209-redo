#include <stdio.h>
#include <stdlib.h>

#include "hash.h"

/**
 * Compute the hash of a file
 * @param  f the file open for reading.
 * @return   the hash character.
 */
char *hash(FILE *f) {
	char *hash_val = malloc(sizeof(char) * BLOCK_SIZE);
	char ch;
	int hash_index = 0;

	for (int i = 0; i < BLOCK_SIZE; i++) {
		hash_val[i] = '\0';
	}

	while(fread(&ch, 1, 1, f) != 0) {
        hash_val[hash_index] ^= ch;
        hash_index = (hash_index + 1) % BLOCK_SIZE;
    }
	if (ferror(f)) {
		perror("fread for hash");
	}

	return hash_val;
}

/**
 * Helper function that shows the hash in hex
 * @param hash the hash value
 */
void show_hash(char *hash) {
	printf("[");
	for (int i = 0; i < BLOCK_SIZE; i++) {
		printf("%.2hhx", hash[i]);
	}
	printf("]\n");
}
