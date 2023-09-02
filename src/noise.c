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

#define ITERATE {for (unsigned int i = 0; i < NOISE_SEED_ITERATIONS; i++) {seed = lcg(seed);}}
void noise__populate(struct Seeds* seeds, int base) {
	seeds->base = base;
	int seed = base;
	ITERATE;
	seeds->ground_big = seed;
	ITERATE;
	seeds->ground_small = seed;

	printf("RANDOM_MAX: %d\n", RANDOM_MAX);
}
#undef ITERATE

#define PI ((double)3.14159)
static int interpolate(int a, int b, float t) {
// mu = math.cos(x * math.pi) / 2 + .5
// return a * mu + b * (1 - mu)

	double mu = 1-(cos((double)t * PI) / 2.0 + .5);
	return (int)((double)a * (1. - mu) + (double)b * mu);
}

#define FILTER_HASHMAP_SIZE 256
struct Filter {
	struct {
		unsigned int size;
		struct Filter__Entry {
			int x;
			int value;
		} *entries;
	} entries[FILTER_HASHMAP_SIZE];
} static filter;

struct Filter* filter__new() {
	struct Filter* filter = malloc(sizeof(struct Filter));
	if (filter == NULL) {
		return NULL;
	}

	memset(&filter->entries, 0, sizeof(filter->entries));

	return filter;
}

int _gen(struct Seeds seeds, int x);

int noise__gen_ground(struct Seeds seeds, int x) {
	return _gen(seeds, x);
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
		);
//	return random(seeds.ground_small, x) / rand_div;
}
