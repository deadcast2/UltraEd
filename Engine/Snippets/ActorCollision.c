void _UER_ActorCollision() 
{ 
    for (int i = 0; i < CVector_Size(_UER_Actors); i++) 
    { 
        Actor *outerActor = CVector_Get(_UER_Actors, i); 
        
        if (outerActor->collider == TNone) continue; 
        
        for (int j = i + 1; j < CVector_Size(_UER_Actors); j++)
        {
            Actor *innerActor = CVector_Get(_UER_Actors, j);
            
            if (innerActor->collider == TNone) continue;
            
            if (check_collision(outerActor, innerActor))
            { 
                if (outerActor->collide != NULL) outerActor->collide(outerActor, innerActor);
                
                if (innerActor->collide != NULL) innerActor->collide(innerActor, outerActor);
            }
        } 
    }
}