#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "block.h"

#define BLOCK_PATH_SIZE 128

int block__set(struct Block* block, unsigned int type) {
	block->type = type;
	if (type == 0) {
		block->texture_cache = (void*)1;
	} else {
		if (type > MAX_BLOCKS || type > block_types_size) {
			fprintf(stderr, "unknown block type %d\n", type);
			return 1;
		}

		block->texture_cache = NULL;
	}

	return 0;
}
int block__set_name(struct Block* block, char* type_name) {
	for (size_t i = 1; i < block_types_size; i++) {
		if (strcmp(type_name, block_types[i].name) == 0) {
			return block__set(block, i);
		}
	}

	return 1;
}

static struct Texture_file* read_dir(const char* path, size_t* size);
static int parse_block(const char* path);

struct Block_type block_types[MAX_BLOCKS];
size_t block_types_size = 1;  // the first one is air

struct Texture_file {
	const char* path;
	const char* name;
};
static struct Texture_file* texture_files = NULL;

int load_blocks() {
	size_t len;
	texture_files = read_dir("mod/blocks/assets", &len);
	if (texture_files == NULL) {
		fprintf(stderr, "read_dir() returned NULL\n");
		return 1;
	}

	// for (size_t i = 0; i < len; i++) {
	// 	printf("'%s': '%s'\n", files[i].path, files[i].name);
	// 	// puts(files[i].path);
	// 	// puts(files[i].name);
	// }

	DIR* dp;
	struct dirent* ep;

	const char* path = "mod/blocks";
	dp = opendir(path);
	if (dp == NULL) {
		fprintf(stderr, "opendir(\"%s\") [%d %s]\n", path, errno, strerror(errno));

		return 1;
	}

	puts("parsing block files");

	while ((ep = readdir(dp))) {
		if (ep->d_name[0] != '.') {
			char* new_path;
			{
				size_t old_path_len = strlen(path);
				size_t path_len = old_path_len;
				path_len += 2;  // for '/' and '\0'
				path_len += strlen(ep->d_name);

				new_path = malloc(path_len);
				if (new_path == NULL) {
					return 1;
				}

				strcpy(new_path, path);
				new_path[old_path_len] = '/';  // TODO: does not work on windows
				strcpy(new_path + old_path_len + 1, ep->d_name);
			}

			puts(new_path);

			struct stat s;
			if (stat(new_path, &s) == 0) {
				if (!S_ISDIR(s.st_mode)) {
					if (parse_block(new_path) != 0) {
						fprintf(stderr, "parsing of block '%s' failed\n", new_path);
						return 1;
					}
				}
			} else {
				fprintf(stderr, "stat(\"%s\") [%d %s]\n", new_path, errno, strerror(errno));
			}
		}
	}
	closedir(dp);

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
			if (new_path == NULL) {
				return NULL;
			}

			strcpy(new_path, path);
			new_path[old_path_len] = '/';  // TODO: does not work on windows
			strcpy(new_path + old_path_len + 1, ep->d_name);

			// puts(new_path);

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
				fprintf(stderr, "stat(\"%s\") [%d %s]\n", new_path, errno, strerror(errno));
			}

			free(new_path);
		}
	}
	(void) closedir(dp);

	(*size) = files_len;
	return files;
}

static int parse_block(const char* path) {
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		fprintf(stderr, "parse_block: fopen(\"%s\", \"r\") [%d %s]\n", path, errno, strerror(errno));

		return 1;
	}

#define PARSE_LINE_SIZE 128
	char line[PARSE_LINE_SIZE];

	char lhs[PARSE_LINE_SIZE];
	char rhs[PARSE_LINE_SIZE];

	struct Block_type type;

		type.max_support = -1;

		memset(&type.texture, 0, sizeof(type.texture));

	{
		size_t name_size = 32;
		char* name = malloc(name_size);

		size_t path_len = strlen(path);

		enum {EXTENSION, NAME} state = EXTENSION;
		name[name_size-1] = 0;
		size_t j = name_size - 2;
		for (size_t i = path_len-1; i > 0; i--) {
			if (path_len-i >= name_size) {
				fprintf(stderr, "name too long\n");
				return 1;
			}

			if (state == EXTENSION) {
				if (path[i] == '.') {
					state = NAME;
				}
			} else if (state == NAME) {
				if (path[i] == '/') {
					break;
				}

				name[j] = path[i];
			}

			j--;
		}
		printf("name = '%s'\n", name + j + 1);

		type.name = malloc(path_len /* - j */);
		if (type.name == NULL) {
			return 1;
		}

		strcpy(type.name, name + j + 1);
		// type.name = name + j + 1;
		free(name);
	}
	while(fgets(line, PARSE_LINE_SIZE-1, file)) {
		// printf("parse: %s", line);

		enum {LHS, RHS} state = LHS;
		size_t i = 0, j = 0;
		for (i = 0; line[i+1] != 0 && i < PARSE_LINE_SIZE-1; i++) {
			if (line[i] == ' ') {
				j = i;
				state = RHS;
				continue;
			}

			if (state == LHS) {
				lhs[i] = line[i];
			} else {
				rhs[i-j-1] = line[i];
			}
		}

		lhs[j] = 0;
		rhs[i-j-1] = 0;

		// printf("parse: '%s' '%s'\n", lhs, rhs);

		if (strcmp(lhs, "max_support") == 0) {
			type.max_support = atoi(rhs);
		} else if (strcmp(lhs, "solid") == 0) {
			type.solid = atoi(rhs);
		} else if (strcmp(lhs, "fluid") == 0) {
			type.fluid = atoi(rhs);
		} else {
			fprintf(stderr, "unknown key '%s' in file '%s'\n", lhs, path);
			return 1;
		}
	}

	memcpy(&block_types[block_types_size], &type, sizeof(type));
	block_types_size++;
	if (block_types_size > MAX_BLOCKS) {
		fprintf(stderr, "WARNING: too many blocks, increase MAX_BLOCKS\n");
		return 1;
	}

	fclose(file);

	// TODO: load textures

	{
		char path[BLOCK_PATH_SIZE];
		const char* pre = "mod/blocks/assets/grass";  // TODO: does not work on windows

		strncpy(path, pre, BLOCK_PATH_SIZE);

		DIR* dp;
		struct dirent* ep;

		dp = opendir(path);
		if (dp == NULL) {
			fprintf(stderr, "opendir(\"%s\") [%d %s]\n", path, errno, strerror(errno));

			return 1;
		}

		while ((ep = readdir(dp))) {
			if (ep->d_name[0] == '.') {
				continue;
			}

			puts(ep->d_name);
		}

		closedir(dp);
	}

	return 0;
}

void free_blocks() {
	for (size_t i = 1; i < block_types_size; i++) {
		free(block_types[i].name);

		// TODO: free block_types[i].texture
	}
}

SDL_Surface* block_type__get_texture(unsigned int neighbours, size_t* len);

