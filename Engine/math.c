#include "math.h"

float CMath_Vec3Dist(Vector3 a, Vector3 b)
{
    return sqrtf((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z));
}

float CMath_Vec3Dot(Vector3 a, Vector3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float CMath_Vec3Len(Vector3 a, Vector3 b)
{
    return sqrtf(CMath_Vec3Dot(a, b));
}

Vector3 CMath_Vec3Norm(Vector3 vector)
{
    const float len = CMath_Vec3Len(vector, vector);
    return (Vector3) {
        vector.x / len, vector.y / len, vector.z / len
    };
}

Vector3 CMath_Vec3Add(Vector3 a, Vector3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

Vector3 CMath_Vec3Sub(Vector3 a, Vector3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

Vector3 CMath_Vec3Mul(Vector3 vector, float scalar)
{
    vector.x *= scalar;
    vector.y *= scalar;
    vector.z *= scalar;
    return vector;
}

Vector3 CMath_Vec3MulMat(Vector3 vector, Mtx mat)
{
    float x, y, z;
    guMtxXFML(&mat, vector.x, vector.y, vector.z, &x, &y, &z);
    return (Vector3) {
        x, y, z
    };
}

Mtx CMath_MatMulMat(Mtx a, Mtx b)
{
    Mtx mat;
    guMtxCatL(&a, &b, &mat);
    return mat;
}
