#ifndef _UTIL_H_
#define _UTIL_H_

#include <rpc.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

#define CLSID_LENGTH 40

class CUtil
{
public:
  static float Lerp(float time, float start, float end);
  static GUID NewGuid();
  static GUID StringToGuid(const char *guid);
  static string GuidToString(GUID guid);
  static string RootPath();
  static string NewResourceName(int count);
  static char *ReplaceString(const char *str, const char *from, const char *to);
  static vector<string> SplitString(const char *str, const char delimiter);

private:
  CUtil() {};
};

#endif