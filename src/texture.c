#include <stddef.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "block.h"
#include "world.h"
#include "main.h"
#include "math.h"

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
	static const short int connect_idx[8] = {
		// 1, 3, 4, 6
		1, 4, 6, 3,

		2, 7, 5, 0
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

	unsigned int edge = 0;
	for (unsigned int i = 0; i < 4; i++) {
		unsigned int j = connect_idx[i+4];

		edge |= (border[j] != block->type && border[connect_idx[mod(i, 4)]] == block->type && border[connect_idx[mod(i+1, 4)]] == block->type) << (3-i);
	}

	unsigned char cache = edge << 4 | connect;
	if (type->texture.connect != NULL && type->texture.connect->cache[cache] != NULL) {
		return type->texture.connect->cache[cache];
	} else {
		if (type->texture.connect != NULL) {
			// type->texture.connect->cache[cache] = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, SIZE, SIZE);
			//if (type->texture.connect->cache[cache] == NULL) {
			//	fprintf(stderr, "error creating cache texture:\n\t%s\n", SDL_GetError());
			// } else {
			if (cache == 0) {
				if (type->texture.single->texture != NULL) {
					type->texture.connect->cache[0] = type->texture.single->texture;
				}
			} else {
				printf("creating texture %u\n", cache);
				/*
                                 *   😈
                                 */

				SDL_Texture* target = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SIZE, SIZE);
				if (target == NULL) {
					fprintf(stderr, "error creating cache render texture:\n\t%s\n", SDL_GetError());
					goto fail;
				}

				if (SDL_SetRenderTarget(render, target) < 0) {
					fprintf(stderr, "error setting render target\n\t%s\n", SDL_GetError());
					goto fail;
				}

				if (SDL_RenderCopy(render, type->texture.connect->textures[(cache & 0x0F) - 1], NULL, NULL) < 0) {
					fprintf(stderr, "error rendering to target\n\t%s\n", SDL_GetError());
					goto fail;
				}

				// TODO
				if (cache & 0xF0 && (cache & 0xF0) != 0xF0) {
					for (unsigned int i = 0; i < 4; i++) {
						if (cache & (0x10 << (3-i))) {
							if (SDL_RenderCopy(render, type->texture.connect->edge[i], NULL, NULL) < 0) {
								fprintf(stderr, "error rendering to target\n\t%s\n", SDL_GetError());
								goto fail;
							}
						}
					}
				}

				static unsigned char* pixels[SIZE*SIZE*4];
				if (SDL_RenderReadPixels(render, NULL, SDL_PIXELFORMAT_RGBA8888, pixels, SIZE*4) < 0) {
					fprintf(stderr, "error getting pixels\n\t%s\n", SDL_GetError());
					goto fail;
				}

				if (SDL_SetRenderTarget(render, NULL) < 0) {
					fprintf(stderr, "error resetting render target, exiting\n\t%s\n", SDL_GetError());
					exit(1);
				}

				SDL_DestroyTexture(target);

				SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, SIZE, SIZE, 8*4, SIZE*4, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
				if (surface == NULL) {
					fprintf(stderr, "error turning pixels into surface\n\t%s\n", SDL_GetError());
					goto not_fail;
				}

				SDL_Texture* texture = SDL_CreateTextureFromSurface(render, surface);
				if (texture == NULL) {
					fprintf(stderr, "error turning surface into texture\n\t%s\n", SDL_GetError());
					SDL_FreeSurface(surface);
					goto not_fail;
				}

				SDL_FreeSurface(surface);

				type->texture.connect->cache[cache] = texture;

				goto fail;  // did not fail but to reset renderer to window
			}
			// }
		}
	}
	goto not_fail;

    fail:
	if (SDL_SetRenderTarget(render, NULL) < 0) {
		fprintf(stderr, "error resetting render target, quitting\n\t%s\n", SDL_GetError());
		exit(1);
	}
	if (type->texture.connect->cache[cache] != NULL) {
		return type->texture.connect->cache[cache];
	}

    not_fail:

	// TODO: connected textures
	if (connect_idx != 0 && type->texture.connect != NULL) {
		if (type->texture.connect->textures[connect] != NULL) {
			return type->texture.connect->textures[connect-1];
		}
	}
	if (type->texture.single != NULL) {
		return type->texture.single->texture;
	}

	return NULL;
}

