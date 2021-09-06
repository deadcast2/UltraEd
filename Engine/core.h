#ifndef _CORE_H_
#define _CORE_H_

#include <malloc.h>
#include <memory.h>
#include "actor.h"

#define VECTOR3(X, Y, Z) (Vector3) { X, Y, Z }

Actor *FindActorByName(const char *name)
{
    for (int i = 0; i < vector_size(_UER_Actors); i++)
    {
        Actor *curr = vector_get(_UER_Actors, i);

        if (curr->name == name)
            return curr;
    }

    return NULL;
}

void SetActiveCamera(Actor *camera)
{
    if (camera != NULL && camera->type == Camera)
    {
        _UER_ActiveCamera = camera;
    }
}

Actor *Clone(Actor *actor)
{
    if (actor == NULL) return NULL;

    Actor *clonedActor = (Actor *)malloc(sizeof(Actor));

    if (clonedActor)
    {
        memcpy(clonedActor, actor, sizeof(*clonedActor));

        // Get the largest ID to use as base for next ID.
        int maxId = 0;
        for (int i = 0; i < vector_size(_UER_Actors); i++)
        {
            Actor *curr = vector_get(_UER_Actors, i);

            if (curr->id > maxId)
                maxId = curr->id;
        }

        // Assign a unique ID.
        clonedActor->id = ++maxId;

        // Will clone and re-link any available child actors.
        clonedActor->children = NULL;

        vector_add(_UER_Actors, clonedActor);

        if (clonedActor->start != NULL)
            clonedActor->start(clonedActor);

        // Clone and link any present children.
        if (actor->children != NULL)
        {
            for (int i = 0; i < vector_size(actor->children); i++)
            {
                Actor *child = vector_get(actor->children, i);
                Actor *clonedChild = Clone(child);

                if (clonedChild != NULL)
                {
                    CActor_LinkChildToParent(_UER_Actors, clonedChild, clonedActor);
                }
            }
        }

        return clonedActor;
    }

    return NULL;
}

void Destroy(Actor *actor)
{
    if (actor == NULL) return;

    if (actor->children != NULL)
    {
        for (int i = 0; i < vector_size(actor->children); i++)
        {
            Actor *child = vector_get(actor->children, i);

            Destroy(child);
        }
    }

    vector_add(_UER_ActorsPendingRemoval, actor);
}

#endif
