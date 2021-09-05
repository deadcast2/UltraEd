#ifndef _CORE_H_
#define _CORE_H_

#include "actor.h"
#include "hashtable.h"

#define VECTOR3(X, Y, Z) (Vector3) { X, Y, Z }

Actor *FindActorByName(const char *name)
{
    nlist *np = lookup(name);

    if (np == NULL) return NULL;

    return vector_get(_UER_Actors, np->gameObjectIndex);
}

void SetActiveCamera(Actor *camera)
{
    if (camera != NULL && camera->type == Camera)
    {
        _UER_ActiveCamera = camera;
    }
}

Actor *Instantiate(Actor *other)
{
    if (other == NULL) return NULL;

    Actor *clonedActor = (Actor *)malloc(sizeof(Actor));

    if (clonedActor)
    {
        memcpy(clonedActor, other, sizeof(*clonedActor));

        vector_add(_UER_Actors, clonedActor);

        if (clonedActor->start != NULL)
        {
            clonedActor->start(clonedActor);
        }

        return clonedActor;
    }

    return NULL;
}

#endif
