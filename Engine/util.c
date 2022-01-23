#include "util.h"

void CUtil_Rom2Ram(void *from_addr, void *to_addr, s32 seq_size)
{
    // If size is odd-numbered, cannot send over PI, so make it even.
    if (seq_size & 0x00000001) seq_size++;

    nuPiReadRom((u32)from_addr, to_addr, seq_size);
}

unsigned short *CUtil_Image24To16(const unsigned char *data, int size_x, int size_y)
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
