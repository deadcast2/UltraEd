#include "utilities.h"

#include <malloc.h>

void rom_2_ram(void *from_addr, void *to_addr, s32 seq_size)
{
    // If size is odd-numbered, cannot send over PI, so make it even.
    if (seq_size & 0x00000001) seq_size++;
    nuPiReadRom((u32)from_addr, to_addr, seq_size);
}

unsigned short *image_24_to_16(const unsigned char *data, const int size_x, const int size_y)
{
    int x, y;
    unsigned short *temp = (unsigned short *)malloc(size_x * size_y * 2);

    for (y = 0; y < size_y; y++)
    {
        for (x = 0; x < size_x; x++)
        {
            temp[x + y * size_y] = RGB5551(data[x * 3 + y * size_y * 3],
                data[x * 3 + y * size_y * 3 + 1],
                data[x * 3 + y * size_y * 3 + 2]);
        }
    }

    return temp;
}

float dot(vector3 a, vector3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

vector3 vec3_add(vector3 a, vector3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

vector3 vec3_sub(vector3 a, vector3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

vector3 vec3_rot(vector3 a, Mtx mat)
{
    float fMat[4][4];
    guMtxL2F(fMat, &mat);
    vector3 newVector;
    newVector.x = (a.x * fMat[0][0]) + (a.y * fMat[1][0]) + (a.z * fMat[2][0]);
    newVector.y = (a.x * fMat[0][1]) + (a.y * fMat[1][1]) + (a.z * fMat[2][1]);
    newVector.z = (a.x * fMat[0][2]) + (a.y * fMat[1][2]) + (a.z * fMat[2][2]);
    return newVector;
}
