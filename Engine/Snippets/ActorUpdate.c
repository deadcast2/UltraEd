void _UER_Draw(Gfx **display_list, NUContData gamepads[4]) 
{
	for (int i = 0; i < vector_size(_UER_Actors); i++)
	{
		actor *curr = vector_get(_UER_Actors, i);

		if (curr->parent == NULL) modelDraw(curr, display_list);

		if (curr->update != NULL) curr->update(curr);

		if (curr->input != NULL) curr->input(curr, gamepads);
	}
}