#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <SDL2/SDL.h>

#include "main.h"
#include "world.h"
#include "physics.h"

struct World* world;

#define TITLE_LENGTH 128

#ifdef __EMSCRIPTEN__
 void
#else
 int
#endif
 main_loop(void);

SDL_Renderer* render = NULL;

int main() {
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

		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, 0);
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

	world = world__new();
	if (world == NULL) {
		fprintf(stderr, "error creating world\n");

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
	SDL_Quit();
}

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

	SDL_Rect r = {10, 10, 100, 100};
	SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &r);

	SDL_RenderPresent(render);

#ifndef __EMSCRIPTEN__
	return run;
#endif
}

