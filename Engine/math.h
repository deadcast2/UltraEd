#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <nusys.h>
#include <malloc.h>
#include "n64sdk\ultra\GCC\MIPSE\INCLUDE\MATH.H"
#include "actor.h"

float CMath_Vec3Dist(Vector3 a, Vector3 b);

float CMath_Vec3Dot(Vector3 a, Vector3 b);

float CMath_Vec3Len(Vector3 a, Vector3 b);

Vector3 CMath_Vec3Norm(Vector3 vector);

Vector3 CMath_Vec3Add(Vector3 a, Vector3 b);

Vector3 CMath_Vec3Sub(Vector3 a, Vector3 b);

Vector3 CMath_Vec3Mul(Vector3 vector, float scalar);

Vector3 CMath_Vec3MulMat(Vector3 vector, Mtx mat);

Mtx CMath_MatMulMat(Mtx a, Mtx b);

#endif
