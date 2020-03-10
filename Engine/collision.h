#ifndef _COLLISION_H_
#define _COLLISION_H_

#include "actor.h"

int check_collision(actor *a, actor *b);

int sphere_sphere_collision(actor *a, actor *b);

int box_box_collision(actor *a, actor *b);

int box_sphere_collision(actor *a, actor *b);

#endif
