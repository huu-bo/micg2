#include <stdio.h>

#include "physics.h"
#include "block.h"
#include "world.h"

#define HASH_SIZE (1 << 22)
#define HASH_ROW_SIZE (1 << 11)

struct {
	struct To_update__Element {
		int bx, by;
		struct Block* block;
	} set[HASH_SIZE];

	struct {
		struct To_update__Element* arr;
		size_t size;
		size_t length;
	} arr;
} static to_update;

struct {
	struct To_update__Element* data;
	size_t size;    // allocated size
	size_t length;  // amount of items
} static to_add;

static size_t hash(int b_x, int b_y) {
	return b_x + b_y * HASH_ROW_SIZE;
}

void init_physics(void) {
	for (size_t i = 0; i < HASH_SIZE; i++) {
		to_update.set[i].block = NULL;
		// to_update.set[i].next_idx = 0;
	}
	to_update.arr.arr = NULL;
	to_update.arr.size = 0;
	to_update.arr.length = 0;

	to_add.data = NULL;
	to_add.size = 0;
	to_add.length = 0;
}

void update_physics(void) {
	// prints on error

	if (to_update.arr.arr && to_update.arr.size > 0) {
		for (size_t i = 0; i < to_update.arr.length; i++) {
			// TODO: update block and remove from set
		}
	}
	to_update.arr.length = 0;

	if (to_add.data && to_add.length > 0) {
		for (size_t i = 0; i < to_add.length; i++) {
			struct To_update__Element* from = &to_add.data[i];
			size_t h = hash(from->bx, from->by);

			if (to_update.set[h].block == NULL) {
				memcpy(&to_update.set[h], from, sizeof(*from));
			}
		}
	}
	to_add.length = 0;
}

void add_to_physics_update(struct World* world, int bx, int by) {
	if (to_add.length >= to_add.size) {
		if (to_add.data == NULL) {
			to_add.size = 0x4;
			to_add.data = malloc(to_add.size);
			if (to_add.data == NULL) {
				fprintf(stderr, "malloc failed %s%d\n", __FILE__, __LINE__);
				exit(1);
			}
		} else {
			to_add.size <<= 1;
			to_add.data = realloc(to_add.data, to_add.size);
			if (to_add.data == NULL) {
				fprintf(stderr, "realloc failed %s%d\n", __FILE__, __LINE__);
				exit(1);
			}
		}
	}

	struct Block* b = world__get(world, bx, by);

	to_add.data[to_add.length++] = ((struct To_update__Element) {.bx = bx, .by = by, .block = b});
}

