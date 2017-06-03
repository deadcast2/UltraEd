#ifndef _COMMON_H_
#define _COMMON_H_

inline bool operator<(const GUID &first, const GUID &second)
{
  return memcmp(&first, &second, sizeof(GUID)) < 0;
}

inline float Lerp(float t, float a, float b)
{
  return (1 - t) * a + t * b;
}

#endif