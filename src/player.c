#include <stdlib.h>

#include "player.h"
#include "main.h"

struct Player* player__new() {
	struct Player* player = malloc(sizeof(struct Player));
	if (player == NULL) {
		return NULL;
	}

	player->x = 0;
	player->y = 670;
	player->x_v = 0;
	player->y_v = 0;

	return player;
}

void player__delete(struct Player* player) {
	free(player);
}

static int player_fly = 0;

#define KEY_W (1 << 3)
#define KEY_A (1 << 2)
#define KEY_S (1 << 1)
#define KEY_D (1 << 0)

#define PLAYER_ACC (0.07)
#define PLAYER_DEC (1.3)
#define PLAYER_JUMP (1.3)
#define PLAYER_GRAVITY (0.2)

void player__update(struct Player* player, unsigned int keys) {
	// wasd

	if (player_fly) {
		if (keys & KEY_W) {
			player->y_v -= PLAYER_ACC;
		}
		if (keys & KEY_S) {
			player->y_v += PLAYER_ACC;
		}
	} else {
		if (
		    (world__get(world, (int)floorf(player->x), (int)floorf(player->y) + 1)->type != 0
		    || world__get(world, (int)ceilf(player->x), (int)floorf(player->y) + 1)->type != 0)
		    && keys & KEY_W
		) {
			player->y_v -= PLAYER_JUMP;
		}

		player->y_v += PLAYER_GRAVITY;
	}

	if (keys & KEY_A) {
		player->x_v -= PLAYER_ACC;
	}
	if (keys & KEY_D) {
		player->x_v += PLAYER_ACC;
	}

	player->y += player->y_v;

	// printf("%f\n", player->y_v);

	while (  // TODO: this assumes that everything that is not air is solid
	    world__get(world, (int)floorf(player->x), (int)ceilf(player->y))->type != 0
	    || world__get(world, (int)ceilf(player->x), (int)ceilf(player->y))->type != 0
	) {
		player->y_v = 0;
		player->y = (int)ceilf(player->y) - 1;
	}
	// printf("player pos: %f %f, %d %d\n", player->x, player->y, px, py);

	int i = 0;
	player->x += player->x_v;
	while (
	    world__get(world, (int)floorf(player->x), (int)ceilf(player->y))->type != 0
	    || world__get(world, (int)ceilf(player->x), (int)ceilf(player->y))->type != 0
	) {
		if (player->x_v > 0) {
			player->x = (int)ceilf(player->x) - 1;
		} else {
			player->x = (int)floorf(player->x) + 1;
		}
		// player->x = (int)ceilf(player->y) + (player->x_v > 0 ? -1 : 1);
		player->x_v = 0;

		if (i++ > 100) {
			printf("collision overflow\n");
			break;
		}
	}

	player->x_v /= PLAYER_DEC;
	player->y_v /= PLAYER_DEC;
}

