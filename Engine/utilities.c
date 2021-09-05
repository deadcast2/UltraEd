#include "utilities.h"

#include <malloc.h>

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size)
{
    // If size is odd-numbered, cannot send over PI, so make it even.
    if (seq_size & 0x00000001) seq_size++;
    nuPiReadRom((u32)from_addr, to_addr, seq_size);
}

unsigned short *image_24_to_16(const unsigned char *data, int size_x, int size_y)
{
    unsigned short *temp = (unsigned short *)malloc(size_x * size_y * 2);

    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {
            temp[x + y * size_y] = RGB5551(data[x * 3 + y * size_y * 3],
                data[x * 3 + y * size_y * 3 + 1],
                data[x * 3 + y * size_y * 3 + 2]);
        }
    }

    return temp;
}

float vec3_dot(Vector3 a, Vector3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float vec3_len(Vector3 a, Vector3 b)
{
    return sqrtf(vec3_dot(a, b));
}

Vector3 vec3_norm(Vector3 vector)
{
    const float len = vec3_len(vector, vector);
    return (Vector3) {
        vector.x / len, vector.y / len, vector.z / len
    };
}

Vector3 vec3_add(Vector3 a, Vector3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

Vector3 vec3_sub(Vector3 a, Vector3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

Vector3 vec3_mul(Vector3 vector, float scalar)
{
    vector.x *= scalar;
    vector.y *= scalar;
    vector.z *= scalar;
    return vector;
}

Vector3 vec3_mul_mat(Vector3 vector, Mtx mat)
{
    float x, y, z;
    guMtxXFML(&mat, vector.x, vector.y, vector.z, &x, &y, &z);
    return (Vector3) {
        x, y, z
    };
}

Mtx mat_mul_mat(Mtx a, Mtx b)
{
    Mtx mat;
    guMtxCatL(&a, &b, &mat);
    return mat;
}
