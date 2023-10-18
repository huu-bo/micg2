#ifndef BLOCK_H_
#define BLOCK_H_

#include <stdlib.h>
#include <SDL2/SDL.h>

#define MAX_BLOCKS 10

struct Block;
struct Block_type;
struct Block_texture;

// struct Transition_texture;
struct Connect_texture;
struct Single_texture;

int load_blocks();
void free_blocks();

struct Block {
	unsigned int type;

	unsigned int support;

	SDL_Texture* texture_cache;
};

// struct Block* block__new(char* type_name);
int block__set(struct Block*, unsigned int type);
int block__set_name(struct Block*, char* type_name);

struct Block_texture {
	struct Connect_texture* (*transition)[MAX_BLOCKS]; // a pointer to an array of pointers to struct Transition_texture
	struct Connect_texture* connect;
	struct Single_texture* single;
	SDL_Color* color;
};

struct Block_type {
	char* name;

	struct Block_texture texture;

	int max_support;

	int solid;
	int fluid;
};

extern struct Block_type block_types[MAX_BLOCKS];
extern size_t block_types_size;

#include "world.h"

// block_types is types of neighbours, 0 is above and block_type is type of block to be rendered.
SDL_Texture* block_type__get_texture(struct World*, int x, int y);

struct Single_texture {
	SDL_Texture* texture;
};

#endif // BLOCK_H_
