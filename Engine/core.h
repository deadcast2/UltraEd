#ifndef _CORE_H_
#define _CORE_H_

#include <malloc.h>
#include <memory.h>
#include "actor.h"

#define VECTOR3(X, Y, Z) (Vector3) { X, Y, Z }

Actor *FindActorByName(const char *name)
{
    for (int i = 0; i < CVector_Size(_UER_Actors); i++)
    {
        Actor *curr = CVector_Get(_UER_Actors, i);

        if (curr->name == name)
            return curr;
    }

    return NULL;
}

void SetActiveCamera(Actor *camera)
{
    if (camera != NULL && camera->type == TCamera)
    {
        _UER_ActiveCamera = camera;
    }
}

Actor *Clone(Model *model)
{
    if (model == NULL) return NULL;

    Model *clonedModel = (Model *)malloc(sizeof(Model));

    if (clonedModel)
    {
        memcpy(clonedModel, model, sizeof(*clonedModel));

        Actor *clonedActor = (Actor *)clonedModel;

        // Get the largest ID to use as base for next ID.
        int maxId = 0;
        for (int i = 0; i < CVector_Size(_UER_Actors); i++)
        {
            Actor *curr = CVector_Get(_UER_Actors, i);

            if (curr->id > maxId)
                maxId = curr->id;
        }

        // Assign a unique ID.
        clonedActor->id = ++maxId;

        clonedActor->parent = NULL;
       
        // Will clone and re-link any available child actors.
        clonedActor->children = NULL;

        CVector_Add(_UER_Actors, clonedActor);

        if (clonedActor->start != NULL)
            clonedActor->start(clonedActor);

        // Clone and link any present children.
        Actor *actor = (Actor *)model;
        if (actor->children != NULL)
        {
            for (int i = 0; i < CVector_Size(actor->children); i++)
            {
                Actor *child = CVector_Get(actor->children, i);
                Actor *clonedChild = Clone((Model*)child);

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
        for (int i = 0; i < CVector_Size(actor->children); i++)
        {
            Actor *child = CVector_Get(actor->children, i);

            Destroy(child);
        }
    }

    CVector_Add(_UER_ActorsPendingRemoval, actor);
}

#endif
