#ifndef _COMMON_H_
#define _COMMON_H_

inline bool operator<(const GUID &first, const GUID &second)
{
  return memcmp(&first, &second, sizeof(GUID)) < 0;
}

#endif