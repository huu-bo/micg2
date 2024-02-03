#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <SDL2/SDL.h>

#include "main.h"
#include "world.h"
#include "block.h"
#include "math.h"
#include "physics.h"
#include "number.h"

#ifndef GIT_VERSION
 #warning "no git commit id"
 #define GIT_VERSION ""
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
 #warning "big endian not supported, colors may render wrong"
#endif

struct World* world;
struct Player* player;

/* TODO:
 * saving:
 *   physics.to_add
 *   world
 *   player pos
 */

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

float camera_x = 0.0;
float camera_y = 0.0;

#define MACRO_VALUE_STR(x) STR(x)
#define MACRO_STR(x) #x

extern int test_world();
int main() {
	// printf("SDL_BUTTON: '%s'\n", MACRO_VALUE_STR(SDL_BUTTON(1)));

	if (test_world() != 0) {
		fprintf(stderr, "world testing failed\n");
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

	init_physics();

	if (number__init() != 0) {
		fprintf(stderr, "initialising number renderer failed\n");
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

			printf("frame time: %d\n", time_took);

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
	number__del();
	SDL_Quit();
}

int mouse_x = 0, mouse_y = 0;

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

	update_physics(world);  // TODO: do this on seperate thread if not compiling for web

	SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
	SDL_RenderFillRect(render, NULL);

	const uint8_t* keys = SDL_GetKeyboardState(NULL);
	uint32_t mouse_press = SDL_GetMouseState(&mouse_x, &mouse_y);

	player__update(player,
		keys[SDL_SCANCODE_W] << 3 |
		keys[SDL_SCANCODE_A] << 2 |
		keys[SDL_SCANCODE_S] << 1 |
		keys[SDL_SCANCODE_D] << 0
	);

	camera_x += (player->x - camera_x) / 2.0;
	camera_y += (player->y - camera_y) / 2.0;

	float player_offset_x = 20.0 + (player->x - camera_x);
	float player_offset_y = 20.0 + (player->y - camera_y);

	int offset_x = (int)roundf(fmod(player->x - player_offset_x, 1) * SIZE);
	int offset_y = (int)roundf(fmod(player->y - player_offset_y, 1) * SIZE);

	// printf("offset: %d %d, player_pos: %f %f\n", offset_x, offset_y, player->x, player->y);

	// int offset_y = mod((int)(player->y * SIZE), SIZE);
	for (int y = -1; y < CHUNK_SIZE+1; y++) {
		for (int x = -1; x < CHUNK_SIZE+1; x++) {
			int bx = x + (int)(player->x - player_offset_x);
			int by = y + (int)(player->y - player_offset_y);

			struct Block* b = world__get(world, bx, by);
			SDL_Rect r = {x * SIZE - offset_x, y * SIZE - offset_y, SIZE, SIZE};

			if (mouse_x >= r.x && mouse_x < r.x + r.w
			    && mouse_y >= r.y && mouse_y < r.y + r.h) {
				if (mouse_press & SDL_BUTTON(1)) {
					if (world__set_by_name(world, bx, by, "grass") == 1) {
						fprintf(stderr, "invalid id\n");
						// TODO:  crash
					}
				} else if (mouse_press & SDL_BUTTON(3)) {
					if (world__set_by_id(world, bx, by, 0) == 1) {
						fprintf(stderr, "air does not exist\n");
						return 1;
					}
				}
				for (short i = -1; i <= 1; i++) {
					for (short j = -1; j <= 1; j++) {
						add_to_physics_update(world, bx + i, by + j);
					}
				}
			}

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

				if (b->texture_cache == NULL) {
					SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
					SDL_RenderFillRect(render, &r);
				}
			}
			if (b->texture_cache != NULL && b->texture_cache != (void*)1) {
				SDL_RenderCopy(render, b->texture_cache, NULL, &r);

				// number__render(render, r.x, r.y, b->support);
			}
		}
	}

	{
		SDL_Rect rect = {
			(int)roundf(player_offset_x * (float)SIZE), (int)roundf(player_offset_y * (float)SIZE),
			SIZE, SIZE
		};
		SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
		SDL_RenderFillRect(render, &rect);
	}

	SDL_RenderPresent(render);

#ifndef __EMSCRIPTEN__
	return run;
#endif
}

