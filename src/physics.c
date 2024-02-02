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
	size_t size;    // allocated size in items
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
	to_update.arr.size = 4;
	to_update.arr.arr = malloc(to_update.arr.size * sizeof(*to_update.arr.arr));
	if (to_update.arr.arr == NULL) {
		fprintf(stderr, "malloc failed %s:%d\n", __FILE__, __LINE__);
		exit(1);
	}
	to_update.arr.length = 0;

	to_add.data = NULL;
	to_add.size = 0;
	to_add.length = 0;

	printf("init physics\n");

	// raise(SIGINT);
}

void update_physics(struct World* world) {
	// prints on error

	unsigned int updates = 0, added = 0;

	if (to_update.arr.arr && to_update.arr.size > 0) {
		updates += to_update.arr.length;
		for (size_t i = 0; i < to_update.arr.length; i++) {
			struct To_update__Element* e = &to_update.arr.arr[i];

			// TODO: update block

			// TODO: block velocity
			int bx = e->bx, by = e->by;
			if (block_types[e->block->type].solid) {
				if (world__get(world, bx, by + 1)->type == 0) {
					by += 1;
				}
			}

			if (e->bx != bx || e->by != by) {
				world__set(world, bx, by, *e->block);
				world__set_by_id(world, e->bx, e->by, 0);

				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						add_to_physics_update(world, e->bx + i, e->by + j);
						add_to_physics_update(world, bx + i, by + j);
					}
				}
			}

			size_t h = hash(e->bx, e->by);

			to_update.set[h].block = NULL;
		}
	}
	to_update.arr.length = 0;

	if (to_add.data && to_add.length > 0) {
		for (size_t i = 0; i < to_add.length; i++) {
			struct To_update__Element* from = &to_add.data[i];
			size_t h = hash(from->bx, from->by);

			if (to_update.set[h].block == NULL) {
				memcpy(&to_update.set[h], from, sizeof(*from));
				added++;

				if (to_update.arr.length >= to_update.arr.size) {
					to_update.arr.size <<= 1;
					to_update.arr.arr = realloc(to_update.arr.arr, to_update.arr.size * sizeof(*to_update.arr.arr));
					if (to_update.arr.arr == NULL) {
						fprintf(stderr, "realloc failed at %s:%d\n", __FILE__, __LINE__);
						exit(1);
					}
				}

				to_update.arr.arr[to_update.arr.length++] = (struct To_update__Element) {
					.bx = from->bx,
					.by = from->by,
					.block = from->block
				};
			}
		}
	}
	to_add.length = 0;

	if (updates != 0 || added != 0) {
		printf("updated physics: updates: %u; added: %u\n", updates, added);  // TODO: print time took
	}
}

void add_to_physics_update(struct World* world, int bx, int by) {
	if (to_add.data == NULL) {
		to_add.size = 0x4;
		to_add.data = malloc(to_add.size * sizeof(*to_add.data));
		if (to_add.data == NULL) {
			fprintf(stderr, "malloc failed %s%d\n", __FILE__, __LINE__);
			exit(1);
		}
	} else if (to_add.size != 0 && to_add.length >= to_add.size) {
		// raise(SIGINT);
		to_add.size <<= 1;
		to_add.data = realloc(to_add.data, to_add.size * sizeof(*to_add.data));
		if (to_add.data == NULL) {
			fprintf(stderr, "realloc failed %s%d\n", __FILE__, __LINE__);
			exit(1);
		}
	}

	struct Block* b = world__get(world, bx, by);
	if (b->texture_cache != (void*)1) {
		b->texture_cache = NULL;
	}

	to_add.data[to_add.length++] = ((struct To_update__Element) {.bx = bx, .by = by, .block = b});
}

