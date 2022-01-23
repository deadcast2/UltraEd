#ifndef _UTIL_H_
#define _UTIL_H_

#include <nusys.h>
#include <malloc.h>

#define RGB5551(R, G, B) (((R >> 3) << 11) | ((G >> 3) << 6) | ((B >> 3) << 1) | 1)

void CUtil_Rom2Ram(void *from_addr, void *to_addr, s32 seq_size);

unsigned short *CUtil_Image24To16(const unsigned char *data, int size_x, int size_y);

#endif
