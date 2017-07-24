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
	for (int i = 0; i < BLOCK_SIZE; i++) {
		hash_val[i] = '\0';
	}

	char c;
	int count = 0;
	while (fread(&c, 1, 1, f) == 1) {
		hash_val[count % BLOCK_SIZE] ^= c;
	}
	if (ferror(f)) {
		perror("fread for hash");
	}

	return hash_val;
}
