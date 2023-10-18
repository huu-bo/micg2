#include <stddef.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "block.h"
#include "world.h"

SDL_Texture* block_type__get_texture(struct World* world, int x, int y) {
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

	unsigned int border[8] = {0};
	for (unsigned int i = 0; i < 8; i++) {
		struct Block* bb = world__get(world, x + border_idx[i][0], y + border_idx[i][1]);
		if (bb == NULL) {
			return NULL;
		}

		border[i] = bb->type;
	}

	// TODO: connected textures
	if (type->texture.single != NULL) {
		return type->texture.single->texture;
	}

	return NULL;
}

