#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <SDL2/SDL.h>

#include "main.h"
#include "world.h"
#include "block.h"

struct World* world;

#define TITLE_LENGTH 128

#ifdef __EMSCRIPTEN__
 void
#else
 int
#endif
 main_loop(void);

SDL_Renderer* render = NULL;

#if SIZE % 10 != 0
 #warning "size of not factor ten results in ugly pixel art"
#endif

extern int test_world();
int main() {
	if (test_world() != 0) {
		return 1;
	}

	int error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	if (error != 0) {
		const char* e  = SDL_GetError();

		fprintf(stderr, "error initialising SDL\n\t%s", e);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error initialising SDL", e, NULL);
		return 1;
	}

	SDL_Window* window = NULL;
	{
		char* title = malloc(TITLE_LENGTH);
		if (title == NULL) {
			fprintf(stderr, "malloc failed for title\n");
			return 1;
		}

		snprintf(title, TITLE_LENGTH, TITLE " %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SIZE * CHUNK_SIZE, SIZE * CHUNK_SIZE, 0);
	}
	if (window == NULL) {
		const char* e  = SDL_GetError();

		fprintf(stderr, "error creating window\n\t%s", e);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error creating window", e, NULL);
		return 1;
	}

	render = SDL_CreateRenderer(window, -1, 0);  // no flags prefers hardware accelerated
	if (render == NULL) {
		const char* e  = SDL_GetError();

		fprintf(stderr, "error creating renderer\n\t%s", e);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error creating renderer", e, NULL);
		return 1;
	}

	world = world__new(21);  // TODO: allow user to enter seed
	if (world == NULL) {
		fprintf(stderr, "error creating world\n");

		return 1;
	}

	if (load_blocks() != 0) {
		fprintf(stderr, "failed to load blocks\n");
		return 1;
	}

	// if (start_physics() != 0) {
	// 	SDL_Quit();
	// 	fprintf(stderr, "error initialising physics\n");

	// 	return 1;
	// }

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, 1);
#else
	int run = 1;
	while (run) {
		run = main_loop();
		SDL_Delay(16); // TODO: measure time and change delay
	}
#endif

	world__delete(world);
	free_blocks();
	SDL_Quit();
}

int px = 0, py = 0;

#ifdef __EMSCRIPTEN__
 void
#else
 int
#endif
main_loop(void) {
#ifndef __EMSCRIPTEN__
	int run = 1;
#endif
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
#ifdef __EMSCRIPTEN__
				emscripten_cancel_main_loop();
#else
				run = 0;
#endif
			} break;
		}
	}

	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderFillRect(render, NULL);

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_W]) {
		py--;
	}
	if (keys[SDL_SCANCODE_S]) {
		py++;
	}
	if (keys[SDL_SCANCODE_A]) {
		px--;
	}
	if (keys[SDL_SCANCODE_D]) {
		px++;
	}

	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			struct Block* b = world__get(world, x + px, y + py);
			SDL_Rect r = {x * SIZE, y * SIZE, SIZE, SIZE};
			if (b == NULL) {
				SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
				SDL_RenderFillRect(render, &r);
				continue;
			}

			if (b->texture_cache == NULL) {
				SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
				SDL_RenderFillRect(render, &r);
			} else if (b->texture_cache == (void*)1) {
				SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
				SDL_RenderFillRect(render, &r);
			}
		}
	}

	SDL_RenderPresent(render);

#ifndef __EMSCRIPTEN__
	return run;
#endif
}

