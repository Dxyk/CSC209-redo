#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ftree.h"
#include "hash.h"

/**
 * Helper function that shows the hash
 * @param hash_val   the hash_value
 */
void show_hash(char *hash_val) {
	for (int i = 0; i < BLOCK_SIZE; i++) {
		printf("%.2hhx ", hash_val[i]);
	}
	printf("\n");
}

/**
 * Returns the TreeNode rooted at fname
 * @param  fname  the path name
 * @return       the FTree generated
 */
struct TreeNode *generate_ftree(const char *fname) {
	struct TreeNode *current_node;
	struct stat *stat;
	FILE *f;
	DIR *dirp;
	struct dirent *dirent;

	char fname_cpy[strlen(fname)];

	// malloc for current tree node
	if (!(current_node = malloc(sizeof(struct TreeNode)))) {
		perror("malloc");
		exit(-1);
	}

	// malloc for stat
if (!(stat = malloc(sizeof(struct stat)))) {
	perror("malloc stat");
	exit(-1);
}

if (lstat(fname, stat) != 0) {
	perror("lstat");
	exit(-1);
}

// assign file name and permission and since it is the root it has no next.
strncpy(fname_cpy, fname, strlen(fname));
fname_cpy[strlen(fname)] = '\0';

char *base_fname = basename(fname_cpy);

	current_node->fname = malloc(sizeof(char) * strlen(base_fname));
	strncpy(current_node->fname, base_fname, strlen(base_fname));

	current_node->permissions = 0777 & stat->st_mode;

	current_node->hash = NULL;

	current_node->next = NULL;

	current_node->contents = NULL;

	if (S_ISREG(stat->st_mode) || S_ISLNK(stat->st_mode)) {
		// if the file is regular
		// assign its hash
		if (!(f = fopen(fname, "rb"))) {
			perror("fopen");
			exit(-1);
		}

		// TODO: hash last value??
		char *hash_val = hash(f);
		current_node->hash = malloc(sizeof(char) * (BLOCK_SIZE + 1));
		strncat(current_node->hash, hash_val, BLOCK_SIZE);
		current_node->hash[BLOCK_SIZE + 1] = '\0';
		show_hash(current_node->hash);

		if (fclose(f) != 0) {
			perror("fclose");
		}
	} else if (S_ISDIR(stat->st_mode)) {
		// if the file is a dir
		// set a static flag
		static struct TreeNode *current = NULL;
		static int is_first = 1;

		if (!(dirp = opendir(fname))) {
			perror("opendir");
			exit(-1);
		}

		while ((dirent = readdir(dirp))) {
			// ignore . files
			if (strncmp(dirent->d_name, ".", 1) == 0) {
				continue;
			}

			// generate new path
			char new_path[strlen(fname_cpy) + 2 + strlen(dirent->d_name)];
			strncpy(new_path, fname, strlen(fname));
			new_path[strlen(fname)] = '\0';
			strncat(new_path, "/", 1);
			strncat(new_path, dirent->d_name, strlen(dirent->d_name) + 1);
			new_path[strlen(new_path)] = '\0';

			if (is_first) {
				current_node->contents = generate_ftree(new_path);
				current = current_node->contents;
				is_first = 0;
			} else {
				current->next = generate_ftree(new_path);
				// current_node->next = generate_ftree(new_path);
				current = current->next;
			}
		}

		if (closedir(dirp) != 0) {
			perror("closedir");
		}

	} else {
		fprintf(stderr, "file type not supported\n");
		exit(-1);
	}

	free(stat);
	return current_node;
}

/**
 * Prints the tree path given a root TreeNode
 * @param root the root TreeNode
 */
void print_ftree(struct TreeNode *root) {
	static int depth = 0;
	if (root->hash) {
		// if hash is not null then it is a file
		printf("%*s%s (%o) (%s)\n", depth * 2, "", root->fname,
			   root->permissions, root->hash);
	} else {
		// if hash is null then it is a directory
		printf("%*s===== %s (%o) =====\n", depth * 2, "", root->fname,
			   root->permissions);
		depth++;

		struct TreeNode *current = root->contents;
		while (current != NULL) {
			print_ftree(current);
			current = current->next;
		}

		depth--;
	}
}
