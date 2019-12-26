#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <nusys.h>
#include "actor.h"

#define RGB5551(R, G, B) (((R >> 3) << 11) | ((G >> 3) << 6) | ((B >> 3) << 1) | 1)

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size);

unsigned short *image_24_to_16(const unsigned char *data, const int size_x, const int size_y);

float dot(vector3 a, vector3 b);

vector3 vec3_sub(vector3 a, vector3 b);

#endif