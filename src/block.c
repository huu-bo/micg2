#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>

#define STBI_MAX_DIMENSIONS 2048
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "block.h"
#include "main.h"

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

unsigned char* resize_image(const unsigned char* image, unsigned int in_size, unsigned int factor);

void format_surface(unsigned char* image, unsigned int size) {
	for (unsigned int i = 0; i < size*size*4; i+=4) {
		unsigned char temp = image[i];
		image[i] = image[i+2];
		image[i+2] = temp;
	}
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
				j--;
			}
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

	fclose(file);

	// TODO: load textures

	{
		char path[BLOCK_PATH_SIZE];
		const char* pre = "mod/blocks/assets/";  // TODO: does not work on windows

		strncpy(path, pre, BLOCK_PATH_SIZE);

		DIR* dp;
		struct dirent* ep;

		if (strlen(type.name) >= sizeof(path) - strlen(pre) + 1 /* +1 for '/' */) {
			fprintf(stderr, "block name '%s' is too long\n", type.name);
		}

		strcat(path, type.name);

		{
			size_t length = strlen(path);
			path[length] = '/';  // bounds check done above
			path[length+1] = 0;
		}

		dp = opendir(path);
		if (dp == NULL) {
			fprintf(stderr, "opendir(\"%s\") [%d %s]\n", path, errno, strerror(errno));

			return 1;
		}

		while ((ep = readdir(dp))) {
			if (ep->d_name[0] == '.') {
				continue;
			}

			size_t type_name_length = strlen(type.name);
			if (strncmp(type.name, ep->d_name, type_name_length) != 0) {
				continue;
			}

			size_t dir_name_length = strlen(ep->d_name);
			if (dir_name_length <= type_name_length) {
				continue;
			}

			char file_path[BLOCK_PATH_SIZE];
			strcpy(file_path, path);

			if (strlen(file_path) + dir_name_length >= sizeof(file_path)) {
				fprintf(stderr, "filename too long\n");
				continue;
			}
			strcat(file_path, ep->d_name);

			int width, height, depth;
			unsigned char* image_raw = stbi_load(file_path, &width, &height, &depth, 4);
			if (image_raw == NULL) {
				fprintf(stderr, "loading image '%s' failed, '%s'\n", file_path, stbi_failure_reason());
				continue;
			}
			printf("\timage size: %d %d, depth: %d\n", width, height, depth);

			unsigned char* image_scaled;
			if (width == SIZE && height == SIZE) {
				if (SIZE != 10) {
					fprintf(stderr, "WARNING: image '%s' does not work on smaller screens\n", file_path);
				}
				image_scaled = image_raw;
			} else if (width == 10 && height == 10) {
				image_scaled = resize_image(image_raw, 10, SIZE / 10);
				free(image_raw);
			} else {
				fprintf(stderr, "image '%s' does not have the right size, it should be 10x10\n", file_path);
				free(image_raw);
				continue;
			}

			format_surface(image_scaled, SIZE);

			// SDL_Surface* image = SDL_CreateRGBSurfaceFrom(image_scaled, SIZE, SIZE, 8*4, 8*4, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
			SDL_Surface* image = SDL_CreateRGBSurfaceFrom(image_scaled, SIZE, SIZE, 8*4, 4*SIZE, 0, 0, 0, 0x000000FF);
			if (image == NULL) {
				fprintf(stderr, "turning image to surface failed for '%s':\n\t%s\n", file_path, SDL_GetError());
				continue;
			}
			SDL_Texture* texture = SDL_CreateTextureFromSurface(render, image);
			SDL_FreeSurface(image);
			if (texture == NULL) {
				fprintf(stderr, "turning surface to texture failed for '%s':\n\t%s\n", file_path, SDL_GetError());
				continue;
			}

			if (ep->d_name[type_name_length] == '.') {
				printf("normal texture '%s'\n", ep->d_name);

				type.texture.single = malloc(sizeof(*type.texture.single));
				if (type.texture.single == NULL) {
					fprintf(stderr, "malloc failed\n");

					SDL_FreeSurface(image);
					SDL_DestroyTexture(texture);
					free(image_scaled);
					continue;
				}

				type.texture.single->texture = texture;
				continue;
			}

			size_t extra_characters = dir_name_length - type_name_length;
		}

		closedir(dp);
	}

	memcpy(&block_types[block_types_size], &type, sizeof(type));
	block_types_size++;
	if (block_types_size > MAX_BLOCKS) {
		fprintf(stderr, "WARNING: too many blocks, increase MAX_BLOCKS\n");
		return 1;
	}

	return 0;
}

unsigned char* resize_image(const unsigned char* in, unsigned int in_size, unsigned int factor) {
	if (in == NULL || factor == 0 || in_size % 10 != 0) {
		return NULL;
	}

	unsigned int size = in_size * factor;

	unsigned char* out = malloc(size*size*4);
	if (out == NULL) {
		return NULL;
	}

	for (unsigned int out_y = 0; out_y < size; out_y++) {
		for (unsigned int out_x = 0; out_x < size; out_x++) {
			// out[out_y*size + out_x] = in[out_y/factor*size + out_x/factor];
			// for (unsigned int i = 0; i < 4; i++) {
			//	out[(out_y*size + out_x)*4+i] = 0xFF;
			// }
			// out[out_y*size + out_x] = 0xFF;
			for (unsigned int i = 0; i < 4; i++) {
				out[(out_y*size + out_x)*4+i] = in[(out_y/factor*in_size + out_x/factor)*4+i];
			}
		}
	}

	return out;
}

void free_blocks() {
	for (size_t i = 1; i < block_types_size; i++) {
		free(block_types[i].name);

		// TODO: free block_types[i].texture
	}
}

