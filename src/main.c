#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <SDL2/SDL.h>

#include "main.h"
#include "world.h"
#include "block.h"
#include "math.h"

#ifndef GIT_VERSION
 #warning "no git commit id"
 #define GIT_VERSION ""
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
 #warning "big endian not supported, colors may render wrong"
#endif

struct World* world;
struct Player* player;

#define TITLE_LENGTH 128

#ifdef __EMSCRIPTEN__
 void
#else
 int
#endif
 main_loop(void);

SDL_Renderer* render = NULL;
SDL_Window* window = NULL;

#if SIZE % 10 != 0
 #warning "size of not factor ten results in textures not loading"
#endif

int is_online = 0;
int is_server = 0;

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

	{
		char* title = malloc(TITLE_LENGTH);
		if (title == NULL) {
			fprintf(stderr, "malloc failed for title\n");
			return 1;
		}

		snprintf(title, TITLE_LENGTH, TITLE " %d.%d.%d " GIT_VERSION, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SIZE * CHUNK_SIZE, SIZE * CHUNK_SIZE, 0);
	}
	if (window == NULL) {
		const char* e  = SDL_GetError();

		fprintf(stderr, "error creating window\n\t%s\n", e);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error creating window", e, NULL);
		return 1;
	}

	render = SDL_CreateRenderer(window, -1, 0);  // no flags prefers hardware accelerated
	if (render == NULL) {
		const char* e  = SDL_GetError();

		fprintf(stderr, "error creating renderer\n\t%s\n", e);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error creating renderer", e, NULL);
		return 1;
	}
	// SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

	{
		struct SDL_RendererInfo info;
		if (SDL_GetRendererInfo(render, &info) < 0) {
			fprintf(stderr, "error getting render info\n\t%s\n\tAssuming renderer does not support SDL_RENDERER_TARGETTEXTURE\n", SDL_GetError());
		} else {
			printf("using renderer '%s', %s\n", info.name, info.flags & SDL_RENDERER_ACCELERATED ? "hardware" : "software");
			if (!(info.flags & SDL_RENDERER_TARGETTEXTURE)) {
				fprintf(stderr, "renderer does not support SDL_RENDERER_TARGETTEXTURE\n");
			} else {
				render_supports_targettex = 1;
			}
		}
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

	player = player__new();
	if (player == NULL) {
		fprintf(stderr, "error creating player\n");
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
		unsigned int ticks = SDL_GetTicks();
		run = main_loop();

		{
			unsigned int time_took = SDL_GetTicks() - ticks;

			// printf("frame time: %d\n", time_took);

			unsigned int delay = 16 - time_took;
			if (delay > 100) {
				delay = 100;
			}
			SDL_Delay(delay);

			// printf("FPS: %d\n", (int) (1 / ((float)(16 - delay) / 10.0)));
		}
	}
#endif

	world__delete(world);
	free_blocks();
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

	SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
	SDL_RenderFillRect(render, NULL);

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	player__update(player,
		keys[SDL_SCANCODE_W] << 3 |
		keys[SDL_SCANCODE_A] << 2 |
		keys[SDL_SCANCODE_S] << 1 |
		keys[SDL_SCANCODE_D] << 0
	);

	int offset_x = (int)(fmod(player->x, 1) * SIZE);
	int offset_y = (int)(fmod(player->y, 1) * SIZE);

	// printf("offset: %d %d, player_pos: %f %f\n", offset_x, offset_y, player->x, player->y);

	// int offset_y = mod((int)(player->y * SIZE), SIZE);
	for (int y = -1; y < CHUNK_SIZE+1; y++) {
		for (int x = -1; x < CHUNK_SIZE+1; x++) {
			int bx = x + (int)player->x;
			int by = y + (int)player->y;

			struct Block* b = world__get(world, bx, by);
			SDL_Rect r = {x * SIZE - offset_x, y * SIZE - offset_y, SIZE, SIZE};
			if (b == NULL) {
				SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
				SDL_RenderFillRect(render, &r);
				continue;
			}

			// SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
			// SDL_RenderFillRect(render, &r);

			// r.x = x * SIZE;
			// r.y = y * SIZE;
			// r.w = r.h = SIZE;

			if (b->texture_cache == NULL) {
				b->texture_cache = world__get_texture(world, bx, by);

				SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
				SDL_RenderFillRect(render, &r);
			} else if (b->texture_cache != (void*)1) {
				SDL_RenderCopy(render, b->texture_cache, NULL, &r);
			}
		}
	}

	SDL_RenderPresent(render);

#ifndef __EMSCRIPTEN__
	return run;
#endif
}

