#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "hashtable.h"

#define VECTOR3(X, Y, Z) &(struct vector3) { X, Y, Z }

struct actor *FindActorByName(const char *name)
{
    struct nlist *np = lookup(name);
    if (np == NULL) return NULL;
    return _UER_Actors[np->gameObjectIndex];
}

#endif