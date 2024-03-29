#include <stdlib.h>

#include "world.h"
#include "main.h"
#include "block.h"
#include "noise.h"
#include "test.h"
#include "math.h"

int render_supports_targettex = 0;

struct Chunk* chunk__new() {
	struct Chunk* chunk = malloc(sizeof(struct Chunk));
	if (chunk == NULL) {
		return NULL;
	}

	chunk->last_used = 0;

	memset(&chunk->chunk, 0, sizeof(chunk->chunk));

	// chunk->chunk = malloc(sizeof(struct Block) * CHUNK_SIZE * CHUNK_SIZE);
	// if (chunk->chunk == NULL) {
	// 	return NULL;
	// }

	return chunk;
}
void chunk__delete(struct Chunk*);

struct World* world__new(long int seed) {
	struct World* world = malloc(sizeof(struct World));
	if (world == NULL) {
		return NULL;
	}

	memset(world->_world, 0, sizeof(world->_world));

	noise__populate(&world->seeds, seed);
	// world->seed = seed;

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

static size_t world__hash(int x, int y) {
	return mod((x + y * WORLD_HASH_ROW), WORLD_HASHMAP_SIZE);
}
struct Chunk* world__get_chunk(struct World* world, int x, int y) {
	if (world == NULL) {
		return NULL;
	}

	size_t v = world__hash(x, y);

	for (size_t i = 0; i < world->_world[v].size; i++) {
		struct World__Hash_entry* e = &world->_world[v].chunks[i];
		if (e == NULL) {
			return NULL;
		}

		if (e->x == x && e->y == y) {
			// printf("chunk %d %d %lu\n", x, y, v);
			return &e->chunk;
		}
	}

	return NULL;
}

struct Chunk* world__gen_chunk(struct World* world, int x, int y) {
	struct Chunk* chunk = chunk__new();
	if (chunk == NULL) {
		return NULL;
	}

	printf("generating chunk %d %d\n", x, y);
	for (size_t bx_i = 0; bx_i < CHUNK_SIZE; bx_i++) {
		int bx = x * CHUNK_SIZE + (signed)bx_i;
		int height = noise__gen_ground(world->seeds, bx);
		for (size_t by_i = 0; by_i < CHUNK_SIZE; by_i++) {
			int by = y * CHUNK_SIZE + (signed)by_i;
			// if (by_i == 1) { printf("\tbx: %d height: %d\n", bx, height); }

			if (by > height) {
				struct Block* b = &chunk->chunk[by_i * CHUNK_SIZE + bx_i];
				block__set_name(b, "grass");
				b->support = by - height - 1;
			} else {
				block__set(&chunk->chunk[by_i * CHUNK_SIZE + bx_i], 0);
			}
		}
	}

	return chunk;
}

static void world__set_chunk(struct World* world, struct Chunk* c, int x, int y) {
	size_t v = world__hash(x, y);
	size_t size = world->_world[v].size;
	struct World__Hash_entry** e = &world->_world[v].chunks;

	printf("re-allocating %lu to %lu\n", size, size+1); // TODO: a standardised way to report hash-map performance

	(*e) = realloc(*e, (size+1) * sizeof(struct World__Hash_entry));
	if ((*e) == NULL) {
		fprintf(stderr, "malloc failed %s:%d\n", __FILE__, __LINE__);
		return;
	}

	(*e)[size].x = x;
	(*e)[size].y = y;
	memcpy(&(*e)[size].chunk /* + size*sizeof(struct World__Hash_entry) */ , c, sizeof(*c));

	world->_world[v].size++;
}

struct Block* world__get(struct World* world, int x, int y) {
	int cx = div_rd(x, CHUNK_SIZE);
	int cy = div_rd(y, CHUNK_SIZE);

	struct Chunk* c = world__get_chunk(world, cx, cy);

	if (c == NULL) {
		c = world__gen_chunk(world, cx, cy);
		if (c == NULL) {
			return NULL;
		}
		world__set_chunk(world, c, cx, cy);
	}

	return &c->chunk[mod(x, CHUNK_SIZE) + mod(y, CHUNK_SIZE) * CHUNK_SIZE];
}

void world__set(struct World* world, int x, int y, struct Block block) {
	struct Block* block_p = world__get(world, x, y);

	memcpy(block_p, &block, sizeof(block));
}
// returns 1 if invalid id, 0 otherwise
int world__set_by_id(struct World* world, int x, int y, unsigned int id) { // TODO: code duplication
	struct Block* block = world__get(world, x, y);
	if (block == NULL) {
		return 1;
	}
	if (block__set(block, id)) {
		return 1;
	}

	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			struct Block* block = world__get(world, x + dx, y + dy);
			if (block == NULL) {
				return 1;
			}
			if (block->texture_cache != (SDL_Texture*)1) {
				block->texture_cache = NULL;
			}
		}
	}
	return 0;
}
// returns 1 if invalid id, 0 otherwise
int world__set_by_name(struct World* world, int x, int y, const char* name) {
	struct Block* block = world__get(world, x, y);
	if (block == NULL) {
		return 1;
	}
	if (block__set_name(block, name)) {
		return 1;
	}

	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			struct Block* block = world__get(world, x + dx, y + dy);
			if (block == NULL) {
				return 1;
			}
			if (block->texture_cache != (SDL_Texture*)1) {
				block->texture_cache = NULL;
			}
		}
	}
	return 0;
}

int test_world() {
	TEST_EQ(9, mod(-1, 10));
	TEST_EQ(9, mod(-11, 10));
	TEST_EQ(9, mod(-21, 10));
	TEST_EQ(9, mod(-31, 10));
	TEST_EQ(0, mod(0, 10));
	TEST_EQ(1, mod(-9, 10));
	TEST_EQ(1, mod(-19, 10));
	TEST_EQ(1, mod(-29, 10));

	TEST_EQ(-1, div_rd(-1, 10));
	TEST_EQ(-1, div_rd(-10, 10));
	TEST_EQ(-2, div_rd(-11, 10));
	TEST_EQ(-3, div_rd(-21, 10));
	TEST_EQ(-4, div_rd(-31, 10));
	TEST_EQ(-5, div_rd(-41, 10));
	TEST_EQ(4, div_rd(41, 10));
	TEST_EQ(10, div_rd(109, 10));
	TEST_EQ(3, div_rd(35, 10));

	return 0;
}

