#include <stdio.h>
#include <pthread.h>
#include <time.h>

#include "physics.h"

static void* test(void*);

int start_physics() {
	pthread_t t;
	int result = pthread_create(&t, NULL, test, NULL);
	if (result != 0) {
		return 1;
	}

	return 0;
}

#define NANO_IN_SEC ((long int)1e9)
#define TICK_RATE 60
static void* test(void* args) {
	printf("hello from other thread\n");

	// double max_delay = NANO_IN_SEC / TICK_RATE;
	long long int frame = 0;
	while (1) {
		struct timespec start_time, end_time;
		clock_gettime(CLOCK_MONOTONIC, &start_time);

		clock_gettime(CLOCK_MONOTONIC, &end_time);

		long int delta_nano = (end_time.tv_sec * NANO_IN_SEC + end_time.tv_nsec) - (start_time.tv_sec * NANO_IN_SEC + start_time.tv_nsec);

		long int delay_int = NANO_IN_SEC / TICK_RATE - delta_nano;
		struct timespec delay, delay_left;

		delay.tv_sec = 0;
		delay.tv_nsec = delay_int % NANO_IN_SEC;

		printf("max tick rate: %lu\n", NANO_IN_SEC / delta_nano);

		if (nanosleep(&delay, &delay_left) < 0) {
			nanosleep(&delay_left, NULL);
		}

		frame++;
	}

	return NULL;
}
