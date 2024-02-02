#ifndef PHYSICS_H_
#define PHYSICS_H_

#include "world.h"

void init_physics(void);
void update_physics(void);

void add_to_physics_update(struct World*, int bx, int by);

#endif // PHYSICS_H_
