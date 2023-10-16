#include <math.h>

#include "noise.h"
#include "math.h"
#include "world.h"
#include "test.h"

static int random(int seed, int x) {
	// return mod((seed + x) * 6969, 420);
	return abs(mod((seed + x) * 653, 401)*2
	     + mod((seed + x) * 509, 389)*3
	     - mod((seed + x) * 11, 97)*10);
}
#define RANDOM_MAX (400*2 + 388*3)
static const unsigned int rand_div = RANDOM_MAX/CHUNK_SIZE;

static unsigned long int lcg(unsigned long int seed) {
// m = 4294967296
// a = 1664525
// c = 1
// seed = (self.a * seed + self.c) % self.m

	return (1664525 * seed + 1) % 4294967296;
}

#define PI ((double) 3.14159265358979323846264338327)
static int interpolate(int a, int b, float t) {
// mu = math.cos(x * math.pi) / 2 + .5
// return a * mu + b * (1 - mu)

	double mu = 1-(cos((double)t * PI) / 2.0 + .5);
	return (int)((double)a * (1. - mu) + (double)b * mu);
}

#define FILTER_HASHMAP_SIZE (2<<12)
struct Filter {
	struct Filter__Entries {
		unsigned int size;
		struct Filter__Entry {
			int x;
			int value;
		} *entries;
	} entries[FILTER_HASHMAP_SIZE];

	int (*gen) (struct Seeds, int);
};

struct Filter* filter__new(int (*gen) (struct Seeds, int)) {
	struct Filter* filter = malloc(sizeof(struct Filter));
	if (filter == NULL) {
		return NULL;
	}

	memset(&filter->entries, 0, sizeof(filter->entries));
	filter->gen = gen;

	return filter;
}
static unsigned int filter__hash(int x) {
	return mod(x, FILTER_HASHMAP_SIZE);
}

// #define FILTER_HASHMAP_DEBUG
static int filter__get(struct Filter* filter, struct Seeds seeds, int x) {
//	if (x < 10 && x > -10) {
//		return filter->gen(seeds, x);
//	}

	unsigned int hash;

	int prev_x;
	if (x > 0) {
		prev_x = x-1;
	} else {
		prev_x = x+1;
	}
	hash = filter__hash(prev_x);

	int found = 0;
	int prev_value = 0;

#ifdef FILTER_HASHMAP_DEBUG
	printf("size: %d @ %u (x: %i)\n", filter->entries[hash].size, hash, x);
#endif
	for (unsigned int i = 0; i < filter->entries[hash].size; i++) {
#ifdef FILTER_HASHMAP_DEBUG
		printf("\tx: %d\n", filter->entries[hash].entries[i].x);
#endif
		if (filter->entries[hash].entries[i].x == prev_x) {
			found = 1;
			prev_value = filter->entries[hash].entries[i].value;
			break;
		}
	}
	if (!found) {
		if (x < 0) {
			prev_value = filter__get(filter, seeds, prev_x);  // TODO: make this not be recursive
		} else {
			fprintf(stderr, "filter: value does not exist\n");
			prev_value = filter->gen(seeds, x);
		}
	}

	int value = filter->gen(seeds, x);
	if (value > prev_value) {
		if (value - prev_value > 1) {
			value = prev_value + 1;
		}
	} else {
		if (prev_value - value > 1) {
			value = prev_value - 1;
		}
	}

	hash = filter__hash(x);
#ifdef FILTER_HASHMAP_DEBUG
	printf("new hash: %u\n", hash);
#endif
	struct Filter__Entries* entries = &filter->entries[hash];

	if (entries->size == 0) {
		entries->entries = malloc(sizeof(entries->entries[0]));
		entries->size++;
	} else {
		entries->entries = realloc(entries->entries, sizeof(entries->entries[0]) * ++entries->size);
	}

	struct Filter__Entry* entry = &entries->entries[entries->size-1];
	entry->x = x;
	entry->value = value;

	return value;
}

static struct Filter* noise__filter;

int _gen(struct Seeds seeds, int x);

#define ITERATE {for (unsigned int i = 0; i < NOISE_SEED_ITERATIONS; i++) {seed = lcg(seed);}}
void noise__populate(struct Seeds* seeds, int base) {
	seeds->base = base;
	int seed = base;
	ITERATE;
	seeds->ground_big = seed;
	ITERATE;
	seeds->ground_small = seed;

	noise__filter = filter__new(_gen);

	printf("RANDOM_MAX: %d\n", RANDOM_MAX);
}
#undef ITERATE

int noise__gen_ground(struct Seeds seeds, int x) {
//	return _gen(seeds, x);
	return filter__get(noise__filter, seeds, x);
}

int _gen(struct Seeds seeds, int x) {
	// if if there is ever a rocky biome, it could be made by adding noise to the height, which will be filtered by the LPF

	float t = (float)mod(x, CHUNK_SIZE) / (float)CHUNK_SIZE;
	float big_t = (float)mod(x, CHUNK_SIZE*CHUNK_SIZE) / (float)(CHUNK_SIZE*CHUNK_SIZE);

	// printf("%d %d\n", x, div_rd(x, CHUNK_SIZE));
	// printf("%d %d %f\n", random(seeds.ground_small, x), random(seeds.ground_small, x) / rand_div, t);
	// printf("%d %d == %d\n", div_rd(x, CHUNK_SIZE), div_rd(x, CHUNK_SIZE) + 1, div_rd(x + CHUNK_SIZE, CHUNK_SIZE));

	TEST_EQ(random(seeds.ground_small, div_rd(x, CHUNK_SIZE)), random(seeds.ground_small, div_rd(x, CHUNK_SIZE)));
	// TEST_EQ(random(seeds.ground_small, div_rd(x, CHUNK_SIZE)), random(seeds.ground_small, div_rd(x, CHUNK_SIZE) + 1));

	return interpolate(
			random(seeds.ground_small, div_rd(x, CHUNK_SIZE*CHUNK_SIZE)) * CHUNK_SIZE / rand_div,
			random(seeds.ground_small, div_rd(x, CHUNK_SIZE*CHUNK_SIZE) + 1) * CHUNK_SIZE / rand_div,
			big_t
		)
	      + interpolate(
			random(seeds.ground_small, div_rd(x, CHUNK_SIZE)) / rand_div,
			random(seeds.ground_small, div_rd(x, CHUNK_SIZE) + 1) / rand_div,
			t
		) /* + x%10 */;
//	return random(seeds.ground_small, x) / rand_div;
}
