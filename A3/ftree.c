#include <dirent.h>
#include <errno.h>
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
	int process_count = 1;
	DIR *dirp;
	struct dirent *dirent;
	struct stat src_stat;
	int result;

	if (!(dirp = opendir(src))) {
		perror("opendir");
		exit(-1);
	}

	if (lstat(src, &src_stat) != 0) {
		perror("lstat src");
		return -1;
	}

	// get the new dest dir
	char *src_cpy = malloc(strlen(src) + 1);
	strncpy(src_cpy, src, strlen(src));
	src_cpy[strlen(src)] = '\0';
	char dest_dir[strlen(dest) + 2 + strlen(basename(src_cpy))];
	strncpy(dest_dir, dest, strlen(dest));
	dest_dir[strlen(dest)] = '\0';
	strncat(dest_dir, "/", 1);
	strncat(dest_dir, basename(src_cpy), strlen(basename(src_cpy)));
	dest_dir[strlen(dest) + 2 + strlen(basename(src_cpy))] = '\0';
	free(src_cpy);

	// copy dir if directory does not exist already
	if (mkdir(dest_dir, src_stat.st_mode) < 0) {
		if (errno != EEXIST) {
			perror("mkdir dest_dir");
			return -1;
		}
	}

	// traverse directory
	while ((dirent = readdir(dirp))) {
		// ignore . files
		if (strncmp(dirent->d_name, ".", 1) == 0) {
			continue;
		}

		// get new src file or dir name
		char *new_src = malloc(strlen(src) + 2 + strlen(dirent->d_name));
		strncpy(new_src, src, strlen(src));
		new_src[strlen(src)] = '\0';
		strncat(new_src, "/", 2);
		strncat(new_src, dirent->d_name, strlen(dirent->d_name));
		new_src[strlen(src) + 2 + strlen(dirent->d_name)] = '\0';

		// get the status of the source file or dir
		if (lstat(new_src, &src_stat) != 0) {
			free(new_src);
			perror("lstat new src");
			return -1;
		}

		if (S_ISREG(src_stat.st_mode)) {
			// if the source is a file then copy the file
			copy_file(new_src, dest_dir);
		} else if (S_ISDIR(src_stat.st_mode)) {

			// if the source is a directory then fork
			if ((result = fork()) == 0) {
				// child copies the directory recursively and exit with the
				// process number count
				exit(copy_dir(new_src, dest_dir));
			} else if (result > 0) {
				// parent waits for child to finish
				pid_t pid;
				int status;
				if ((pid = wait(&status)) == -1) {
					perror("wait");
					return -1;
				} else {
					if (WIFEXITED(status)) {
						process_count += WEXITSTATUS(status);
					} else {
						fprintf(stderr, "wait: should not get here\n");
					}
				}
			} else {
				perror("fork");
				exit(-1);
			}
		} else {
			free(new_src);
			fprintf(stderr, "%s file type not compatible\n", new_src);
			return -1;
		}
		free(new_src);
	}
	closedir(dirp);

	return process_count;
}

/**
 * Copy the source file to the destination directory
 * @param  src  the relative source file path
 * @param  dest the destination directory path
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

	// if outside while loop then dest either has no file or the file's content
	// is different from the src's content
	rewind(src_f);
	char content;
	// create file, copy and chmod
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
