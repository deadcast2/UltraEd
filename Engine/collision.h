#ifndef _COLLISION_H_
#define _COLLISION_H_

#include "actor.h"

int check_collision(Actor *a, Actor *b);

int sphere_sphere_collision(Actor *a, Actor *b);

int box_box_collision(Actor *a, Actor *b);

int box_sphere_collision(Actor *a, Actor *b);

#endif
