#ifndef BLOCK_H_
#define BLOCK_H_

#include <SDL2/SDL.h>

struct Block;

struct Block {
	unsigned int type;
	SDL_Color* color;
};

#endif // BLOCK_H_
