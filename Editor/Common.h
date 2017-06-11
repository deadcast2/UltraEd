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

inline void Log(const char *format, ...)
{
  va_list args;
  va_start(args, format);
 
  char buffer[256];
  vsprintf(buffer, format, args);
  OutputDebugString(buffer);
  
  va_end(args);
}

#endif