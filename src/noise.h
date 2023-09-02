#ifndef NOISE_H_
#define NOISE_H_

struct Seeds;

int noise__gen_ground(struct Seeds, int x);
void noise__populate(struct Seeds*, int base);

#define NOISE_SEED_ITERATIONS 10

struct Seeds {
	int base;
	int ground_big;
	int ground_small;
};

#endif // NOISE_H_
