const int _UER_ActorCount = 3;
struct actor *_UER_Actors[3];

void _UER_Load() {
	_UER_Actors[0] = (struct actor*)load_model_with_texture(_UER_0_MSegmentRomStart, _UER_0_MSegmentRomEnd, _UER_0_TSegmentRomStart, _UER_0_TSegmentRomEnd, 0.000000, 0.721796, 0.000000, 0.000000, 0.999034, 0.000000, 185.036774, 1.000000, 1.000000, 1.000000);

	_UER_Actors[1] = (struct actor*)load_model_with_texture(_UER_1_MSegmentRomStart, _UER_1_MSegmentRomEnd, _UER_1_TSegmentRomStart, _UER_1_TSegmentRomEnd, -0.000005, 0.000000, 0.000000, 0.000000, 0.998682, 0.000000, 185.884323, 1.000000, 1.000000, 1.000000);

	_UER_Actors[2] = (struct actor*)create_camera(-1.365871, 1.969212, -3.932650, 0.170791, 0.000000, 0.000000, 19.667625);
}

void _UER_Draw(Gfx **display_list) {
	model_draw(_UER_Actors[0], display_list);

	model_draw(_UER_Actors[1], display_list);
}