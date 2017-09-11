#ifndef _COMMON_H_
#define _COMMON_H_

#define APP_NAME "UltraEd v0.1"
#define APP_FILE_EXT ".ultra"
#define APP_FILE_FILTER "UltraEd (*.ultra)\0*.ultra"

inline bool operator<(const GUID &first, const GUID &second)
{
  return memcmp(&first, &second, sizeof(GUID)) < 0;
}

#endif