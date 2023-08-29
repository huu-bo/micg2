#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "block.h"

int block__set(struct Block* block, unsigned int type) {}
int block__set_name(struct Block* block, char* type_name) {}

static struct Texture_file* read_dir(const char* path, size_t* size);

struct Texture_file {
	const char* path;
	const char* name;
};
static struct Texture_file* files = NULL;

int load_blocks() {
	size_t len;
	files = read_dir("mod/blocks/assets", &len);
	if (files == NULL) {
		fprintf(stderr, "read_dir() returned NULL\n");
		return 1;
	}

	// for (size_t i = 0; i < len; i++) {
	// 	printf("'%s': '%s'\n", files[i].path, files[i].name);
	// 	// puts(files[i].path);
	// 	// puts(files[i].name);
	// }

	return 0;
}

static struct Texture_file* read_dir(const char* path, size_t* size) {
	if (size == NULL) {
		return NULL;
	}

	size_t files_size = sizeof(struct Texture_file) * 2, files_len = 0;
	struct Texture_file* files = malloc(files_size);

	DIR* dp;
	struct dirent* ep;

	dp = opendir(path);
	if (dp == NULL) {
		fprintf(stderr, "opendir(\"%s\") [%d %s]\n", path, errno, strerror(errno));

		return NULL;
	}

	while ((ep = readdir(dp))) {
		if (ep->d_name[0] != '.') {
			size_t old_path_len = strlen(path);
			size_t path_len = old_path_len;
			path_len += 2;  // for '/' and '\0'
			path_len += strlen(ep->d_name);

			char* new_path = malloc(path_len);

			strcpy(new_path, path);
			new_path[old_path_len] = '/';  // TODO: does not work on windows
			strcpy(new_path + old_path_len + 1, ep->d_name);

			puts(new_path);

			struct stat s;
			if (stat(new_path, &s) == 0) {
				if (S_ISDIR(s.st_mode)) {
					size_t s = 0;
					struct Texture_file* b = read_dir(new_path, &s);
					if (b == NULL) {
						return NULL;
					}

					if (files_len * sizeof(struct Texture_file) + s * sizeof(struct Texture_file) >= files_size) {
						files = realloc(files, files_size + s * sizeof(struct Texture_file));
						if (files == NULL) {
							fprintf(stderr, "malloc failed %s:%d\n", __FILE__, __LINE__);
							return NULL;
						}
					}

					memcpy(files + files_len * sizeof(struct Texture_file), b, s * sizeof(struct Texture_file));
					files_len += s;

					free(b);
				} else {
					if (files_len * sizeof(struct Texture_file) >= files_size) {
						files = realloc(files, files_size + sizeof(struct Texture_file));
						if (files == NULL) {
							fprintf(stderr, "malloc failed %s:%d\n", __FILE__, __LINE__);
							return NULL;
						}
					}

					{
						size_t name_len = strlen(ep->d_name);
						char* name = malloc(name_len);
						if (name == NULL) {
							return NULL;
						}

						size_t i;
						for (i = 0; ep->d_name[i] != '.' && i < name_len; i++) {
							name[i] = ep->d_name[i];
						}
						// strcpy(name, ep->d_name);

						if (i < name_len) {
							name = realloc(name, i+1);
							if (name == NULL) {
								return NULL;
							}
						}

						files[files_len].name = name;
					}
					{
						size_t path_len = strlen(new_path);
						char* path = malloc(path_len);
						if (path == NULL) {
							fprintf(stderr, "malloc failed %s:%d\n", __FILE__, __LINE__);
							return NULL;
						}
						strcpy(path, new_path);

						files[files_len].path = path;
					}
					files_len++;
				}
			} else {
				fprintf(stderr, "stat(\"%s\") [%d %s]\n", path, errno, strerror(errno));
			}

			free(new_path);
		}
	}
	(void) closedir(dp);

	(*size) = files_len;
	return files;
}
