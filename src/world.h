#ifndef WORLD_H_
#define WORLD_H_

#include <stddef.h>

#include "block.h"
#include "noise.h"

struct World;
struct Chunk;

#define WORLD_HASHMAP_SIZE 1024  // 1024
#define WORLD_HASH_ROW 10  // 10
#define CHUNK_SIZE 40

struct Chunk {
	unsigned int last_used;
	struct Block chunk[CHUNK_SIZE * CHUNK_SIZE];
};

struct Chunk* chunk__new();
void chunk__delete(struct Chunk*);

struct World {
	// long int seed;
	struct Seeds seeds;

	struct {
		size_t size;
		struct World__Hash_entry {
			int x, y;
			struct Chunk chunk;
		} *chunks;
	} _world[WORLD_HASHMAP_SIZE];
};

struct World* world__new(long int seed);
void world__delete(struct World*);

struct Block* world__get(struct World*, int x, int y);

#endif // WORLD_H_
