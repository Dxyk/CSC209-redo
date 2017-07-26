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

int copy_file(const char *src, const char *dest);
int copy_dir(const char *src, const char *dest);

/**
 * Function for copying a file tree rooted at src to dest
 * Returns < 0 on error. The magnitude of the return value
 * is the number of processes involved in the copy and is
 * at least 1.
 * @param  src  the source path
 * @param  dest the destination path
 * @return      the number of processes used
 */
int copy_ftree(const char *src, const char *dest) {
	int process_count = 1;
	struct stat src_stat, dest_stat;

	// check if the destination is a directory
	if (lstat(dest, &dest_stat) != 0) {
		perror("lstat");
		exit(-1);
	}
	if (!S_ISDIR(dest_stat.st_mode)) {
		fprintf(stderr, "Dest should be a directory");
		return -1;
	}


	if (lstat(src, &src_stat) != 0) {
		perror("lstat");
		exit(-1);
	}

	if (S_ISREG(src_stat.st_mode)) {
		// TODO: file
		process_count = copy_file(src, dest);
	} else if (S_ISDIR(src_stat.st_mode)) {
		process_count = copy_dir(src, dest);
	} else {
		fprintf(stderr, "Not supported file fomat\n");
		return -1;
	}

	return process_count;
}

/**
 * copy the source directory to the destination directory
 * @param  src  the source directory
 * @param  dest the destination directory
 * @return      the number of processes used
 */
int copy_dir(const char *src, const char *dest) {
	return 0;
}

/**
 * Copy the source file to the destination directory
 * @param  src  the source file
 * @param  dest the destination directory
 * @return      the number of processes used
 */
int copy_file(const char *src, const char *dest) {
	struct stat src_stat, dest_stat;
	FILE *src_f, *new_dest_f;
	DIR *dirp;
	struct dirent *dirent;
	char *src_hash, *new_dest_hash;

	char src_cpy[strlen(src) + 1];
	strncpy(src_cpy, src, strlen(src) + 1);

	char new_dest[strlen(dest) + strlen(src) + 2];
	strncpy(new_dest, dest, strlen(dest) + 1);
	strncat(new_dest, "/", 2);
	strncat(new_dest, basename(src_cpy), strlen(basename(src_cpy)) + 1);
	printf("%s %lu\n", new_dest, strlen(new_dest));

	if (lstat(src, &src_stat) != 0) {
		perror("lstat src");
		return -1;
	}

	if (!(src_f = fopen(src, "rb"))) {
		perror("fopen src");
		return -1;
	}

	if (!(dirp = opendir(dest))) {
		perror("opendir");
		return -1;
	}

	while ((dirent = readdir(dirp))) {
		if (strcmp(src, dirent->d_name) == 0) {
			// check hash. if hash is the same then return
			if (lstat(new_dest, &dest_stat) != 0) {
				perror("lstat dest");
				return -1;
			}
			if (!S_ISREG(dest_stat.st_mode)) {
				fprintf(stderr, "the destination is not a regular file.\n");
			}
			if (!(new_dest_f = fopen(new_dest, "rb"))) {
				perror("fopen new_dest");
				return -1;
			}
			src_hash = hash(src_f);
			new_dest_hash = hash(new_dest_f);
			if (strncmp(src_hash, new_dest_hash, BLOCK_SIZE) == 0) {
				// change mode no matter what
				chmod(new_dest, src_stat.st_mode);
				return 1;
			}
		}
	}

	// if out side while loop then dest either has no file or the file's content
	// is different from the src's content
	rewind(src_f);
	char content;
	// create file and copy
	if (!(new_dest_f = fopen(new_dest, "wb"))) {
		perror("fopen new_dest");
		return -1;
	}
	while (fread(&content, 1, 1, src_f) != 0) {
		if (fwrite(&content, 1, 1, new_dest_f) != 1) {
			fprintf(stderr, "fwrite\n");
		}
	}

	chmod(new_dest, src_stat.st_mode);

	return 1;
}
