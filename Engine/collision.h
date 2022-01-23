#ifndef _COLLISION_H_
#define _COLLISION_H_

#include "actor.h"

int CCollision_Test(Actor *a, Actor *b);

int CCollision_SphereSphereTest(Actor *a, Actor *b);

int CCollision_BoxBoxTest(Actor *a, Actor *b);

int CCollision_BoxSphereTest(Actor *a, Actor *b);

#endif
