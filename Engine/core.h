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

Actor *Instantiate(Actor *other)
{
    if (other == NULL) return NULL;

    Actor *clonedActor = (Actor *)malloc(sizeof(Actor));

    if (clonedActor)
    {
        memcpy(clonedActor, other, sizeof(*clonedActor));

        int maxId = 0;

        for (int i = 0; i < vector_size(_UER_Actors); i++)
        {
            Actor *curr = vector_get(_UER_Actors, i);

            if (curr->id > maxId)
                maxId = curr->id;
        }

        clonedActor->id = ++maxId;

        vector_add(_UER_Actors, clonedActor);

        if (clonedActor->start != NULL)
        {
            clonedActor->start(clonedActor);
        }

        return clonedActor;
    }

    return NULL;
}

void Destroy(Actor *actor)
{
    if (actor == NULL) return;

    for (int i = 0; i < vector_size(_UER_Actors); i++)
    {
        Actor *curr = vector_get(_UER_Actors, i);

        if (curr->id == actor->id)
        {
            vector_remove_at(_UER_Actors, i);

            if (curr->parent != NULL)
            {
                for (int j = 0; j < vector_size(curr->parent->children); j++)
                {
                    Actor *child = vector_get(curr->parent->children, j);

                    if (child->id == curr->id)
                    {
                        vector_remove_at(curr->parent->children, j);
                    }
                }
            }

            free(actor);

            break;
        }
    }
}

#endif
