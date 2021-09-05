void _UER_Collide() 
{ 
    for (int i = 0; i < vector_size(_UER_Actors); i++) 
    { 
        actor *outerActor = vector_get(_UER_Actors, i); 
        
        if (outerActor->collider == None) continue; 
        
        for (int j = i + 1; j < vector_size(_UER_Actors); j++)
        {
            actor *innerActor = vector_get(_UER_Actors, j);
            
            if (innerActor->collider == None) continue;
            
            if (check_collision(outerActor, innerActor))
            { 
                if (outerActor->collide != NULL) outerActor->collide(innerActor);
                
                if (innerActor->collide != NULL) innerActor->collide(outerActor);
            }
        } 
    }
}