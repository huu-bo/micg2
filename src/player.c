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

#define PLAYER_ACC (0.04)
#define PLAYER_DEC (1.1)

void player__update(struct Player* player, unsigned int keys) {
	// wasd

	if (1) {
		if (keys & 0b1000) {
			player->y_v -= PLAYER_ACC;
		}
		if (keys & 0b0010) {
			player->y_v += PLAYER_ACC;
		}
	}

	if (keys & 0b0100) {
		player->x_v -= PLAYER_ACC;
	}
	if (keys & 0b0001) {
		player->x_v += PLAYER_ACC;
	}

	player->x += player->x_v;
	player->y += player->y_v;

	player->x_v /= PLAYER_DEC;
	player->y_v /= PLAYER_DEC;
}

