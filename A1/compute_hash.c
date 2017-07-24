#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Hash manipulation functions in hash_functions.c
void hash(char *hash_val, long block_size);
int check_hash(const char *hash1, const char *hash2, long block_size);

#ifndef MAX_BLOCK_SIZE
#define MAX_BLOCK_SIZE 1024
#endif

/* Converts hexstr, a string of hexadecimal digits, into hash_val, an
 * array of char.  Each pair of digits in hexstr is converted to its
 * numeric 8-bit value and stored in an element of hash_val.
 * Preconditions:
 *    - hash_val must have enough space to store block_size elements
 *    - hexstr must be block_size * 2 characters in length
 */

void xstr_to_hash(char *hash_val, char *hexstr, int block_size) {
	for (int i = 0; i < block_size * 2; i += 2) {
		char str[3];
		str[0] = hexstr[i];
		str[1] = hexstr[i + 1];
		str[2] = '\0';
		hash_val[i / 2] = strtol(str, NULL, 16);
	}
}

// Print the values of hash_val in hex
void show_hash(char *hash_val, long block_size) {
	for (int i = 0; i < block_size; i++) {
		printf("%.2hhx ", hash_val[i]);
	}
	printf("\n");
}

/**
 * main function
 *
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char const *argv[]) {
	long block_size;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: compute_hash BLOCK_SIZE [ COMPARISON_HASH ]\n");
		return -1;
	}
	block_size = strtol(argv[1], NULL, 10);
	char hash_val[block_size];

	if (block_size <= 0 || block_size > MAX_BLOCK_SIZE) {
		fprintf(stderr,
				"The block size should be a positive integer less than %d",
				MAX_BLOCK_SIZE);
		return -1;
	}

	hash(hash_val, block_size);
	printf("\nthe hash is:\n%s\n", hash_val);
	show_hash(hash_val, block_size);

	if (argc == 3) {
		char given_hash[block_size];
		strncpy(given_hash, argv[2], block_size);
		char hash_val_input[block_size];
		xstr_to_hash(hash_val_input, given_hash, block_size);
		printf("The two hash values are different at index %d",
			   check_hash(hash_val, hash_val_input, block_size));
	}


	return 0;
}
