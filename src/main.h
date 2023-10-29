#ifndef MAIN_H_
#define MAIN_H_

#include <SDL2/SDL.h>

#include "world.h"

#define TITLE "micg"
#define SIZE 20

#define VERSION_MAJOR 2
#define VERSION_MINOR 0
#define VERSION_PATCH 0

extern int is_online;
extern int is_server;

extern struct World* world;
extern SDL_Renderer* render;

#endif // MAIN_H_
