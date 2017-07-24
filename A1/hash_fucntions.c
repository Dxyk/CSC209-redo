#include <stdio.h>

// Complete these two functions according to the assignment specifications

/**
 * compute the hash value
 * @param hash_val   the given hash value
 * @param block_size the given block size
 */
void hash(char *hash_val, long block_size) {
	for (int i = 0; i < block_size; i++) {
		hash_val[i] = '\0';
	}
	char c = '\0';
	int count = 0;
	while (scanf("%s\n", &c) != EOF) {
		hash_val[count % block_size] ^= c;
		count++;
	}
}

int check_hash(const char *hash1, const char *hash2, long block_size) {
	for (int i = 0; i < block_size; i++) {
		if (hash1[i] != hash2[i]) {
			return i;
		}
	}
	return block_size;
}
