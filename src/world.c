#include <stdlib.h>

#include "world.h"
#include "main.h"
#include "block.h"

struct Chunk* chunk__new() {
	struct Chunk* chunk = malloc(sizeof(struct Chunk));
	if (chunk == NULL) {
		return NULL;
	}

	chunk->last_used = 0;

	chunk->chunk = malloc(sizeof(struct Block) * CHUNK_SIZE * CHUNK_SIZE);
	if (chunk->chunk == NULL) {
		return NULL;
	}

	return chunk;
}
void chunk__delete(struct Chunk*);

struct World* world__new() {
	struct World* world = malloc(sizeof(struct World));
	if (world == NULL) {
		return NULL;
	}

	memset(world->_world, 0, sizeof(world->_world));

	return world;
}
void world__delete(struct World* world) {
	for (unsigned int i = 0; i < WORLD_HASHMAP_SIZE; i++) {
		if (world->_world[i].size > 0) {
			free(world->_world[i].chunks);
		}
	}

	free(world);
}

