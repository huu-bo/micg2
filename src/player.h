#ifndef PLAYER_H_
#define PLAYER_H_

struct Player {
	float x;
	float y;

	float x_v;
	float y_v;
};

struct Player* player__new();
void player__delete(struct Player*);

void player__update(struct Player*, unsigned int keys); /* Uses global world */

#endif // PLAYER_H_
