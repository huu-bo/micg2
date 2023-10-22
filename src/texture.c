#include <stddef.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "block.h"
#include "world.h"

SDL_Texture* world__get_texture(struct World* world, int x, int y) {
	struct Block* block = world__get(world, x, y);
	if (block == NULL) {
		return NULL;
	}

	struct Block_type* type = &block_types[block->type];

	// printf("get_texture: %s\n", type->name);

	static const short int border_idx[8][3] = {
		{-1, -1, 1},
		{ 0, -1, 0},
		{ 1, -1, 1},
		{-1,  0, 0},
		{ 1,  0, 0},
		{-1,  1, 1},
		{ 0,  1, 0},
		{ 1,  1, 1}
	};
	static const short int connect_idx[4] = {
		// 1, 3, 4, 6
		1, 4, 6, 3
	};

	unsigned int border[8] = {0};
	for (unsigned int i = 0; i < 8; i++) {
		struct Block* bb = world__get(world, x + border_idx[i][0], y + border_idx[i][1]);
		if (bb == NULL) {
			return NULL;
		}

		border[i] = bb->type;
	}

	unsigned int connect = 0;
	for (unsigned int i = 0; i < 4; i++) {
		unsigned int j = connect_idx[i];

		connect |= (border[j] == block->type) << (3-i);
	}
	// if (connect_idx != 15) {
	// 	printf("%u\n", connect_idx);
	// 	for (unsigned int i = 1; i < 8; i+=2) {
	// 		printf("\t%u\n", border[i]);
	// 	}
	// }

	// TODO: connected textures
	if (connect_idx != 0 && type->texture.connect != NULL) {
		if (type->texture.connect->textures[connect] != NULL) {
			return type->texture.connect->textures[connect];
		}
	}
	if (type->texture.single != NULL) {
		return type->texture.single->texture;
	}

	return NULL;
}

