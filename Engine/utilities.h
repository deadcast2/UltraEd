#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <nusys.h>
#include <malloc.h>
#include "actor.h"
#include "n64sdk\ultra\GCC\MIPSE\INCLUDE\MATH.H"

#define RGB5551(R, G, B) (((R >> 3) << 11) | ((G >> 3) << 6) | ((B >> 3) << 1) | 1)

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size);

unsigned short *image_24_to_16(const unsigned char *data, int size_x, int size_y);

float vec3_dist(Vector3 a, Vector3 b);

float vec3_dot(Vector3 a, Vector3 b);

float vec3_len(Vector3 a, Vector3 b);

Vector3 vec3_norm(Vector3 vector);

Vector3 vec3_add(Vector3 a, Vector3 b);

Vector3 vec3_sub(Vector3 a, Vector3 b);

Vector3 vec3_mul(Vector3 vector, float scalar);

Vector3 vec3_mul_mat(Vector3 vector, Mtx mat);

Mtx mat_mul_mat(Mtx a, Mtx b);

#endif