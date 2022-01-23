void _UER_ActorUpdate(Gfx **display_list, NUContData gamepads[4])
{
    for (int i = 0; i < CVector_Size(_UER_Actors); i++)
    {
        Actor *actor = CVector_Get(_UER_Actors, i);

        if (actor->parent == NULL) 
            CActor_Draw(actor, display_list);

        if (actor->update != NULL) 
            actor->update(actor);

        if (actor->input != NULL) 
            actor->input(actor, gamepads);
    }

    // Handle removal of actors marked as destroyed.
    for (int i = 0; i < CVector_Size(_UER_ActorsPendingRemoval); i++)
    {
        Actor *actorToRemove = CVector_Get(_UER_ActorsPendingRemoval, i);

        for (int j = 0; j < CVector_Size(_UER_Actors); j++)
        {
            Actor *actor = CVector_Get(_UER_Actors, j);

            if (actor->id != actorToRemove->id)
                continue;

            CVector_RemoveAt(_UER_Actors, j);

            // Remove link from parent if any.
            if (actor->parent != NULL)
            {
                for (int k = 0; k < CVector_Size(actor->parent->children); k++)
                {
                    Actor *child = CVector_Get(actor->parent->children, k);

                    if (child->id == actor->id)
                    {
                        CVector_RemoveAt(actor->parent->children, k);
                    }
                }
            }

            break;
        }

        if (actorToRemove->destroy != NULL) 
            actorToRemove->destroy(actorToRemove);

        if (actorToRemove->dynamic != NULL)
            free(actorToRemove->dynamic);

        free(actorToRemove);
    }

    // Clear out vector of actors needing to be removed.
    while (!CVector_IsEmpty(_UER_ActorsPendingRemoval))
    {
        CVector_RemoveAt(_UER_ActorsPendingRemoval, CVector_Size(_UER_ActorsPendingRemoval) - 1);
    }
}
