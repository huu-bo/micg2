#include <math.h>

#include "noise.h"
#include "math.h"

static int random(int seed, int x) {
	return mod((seed + x) * 6969, 420);
}

static unsigned long int lcg(unsigned long int seed) {
// m = 4294967296
// a = 1664525
// c = 1
// seed = (self.a * seed + self.c) % self.m

	return (1664525 * seed + 1) % 4294967296;
}

#define PI ((double)3.14159)
static int interpolate(int a, int b, float t) {
// mu = math.cos(x * math.pi) / 2 + .5
// return a * mu + b * (1 - mu)

	double mu = sin((double)t * PI) / 2.0 + .5;
	return (int)((double)a * mu + (double)b * (1. - mu));
}

int noise__gen_ground(int seed, int x) {
	return random(seed, x) / 10;
}
