#ifndef WORLD_H_
#define WORLD_H_

#include <stdlib.h>

#include "block.h"

struct World;
struct Chunk;

#define WORLD_HASHMAP_SIZE 1024
#define CHUNK_SIZE 40

struct Chunk {
	unsigned int last_used;
	struct Block* chunk;
};

struct Chunk* chunk__new();
void chunk__delete(struct Chunk*);

struct World {
	struct {
		size_t size;
		struct {
			int x, y;
			struct Chunk chunk;
		} *chunks;
	} _world[WORLD_HASHMAP_SIZE];
};

struct World* world__new();
void world__delete(struct World*);

#endif // WORLD_H_
