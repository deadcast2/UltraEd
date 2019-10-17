#ifndef _CORE_H_
#define _CORE_H_

#include "actor.h"
#include "hashtable.h"

#define VECTOR3(X, Y, Z) &(vector3) { X, Y, Z }

actor *FindActorByName(const char *name)
{
    nlist *np = lookup(name);
    if (np == NULL) return NULL;
    return _UER_Actors[np->gameObjectIndex];
}

#endif
