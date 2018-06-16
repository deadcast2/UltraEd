#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <windows.h>
#include <string>

using namespace std;

class CSettings
{
public:
  static bool Set(const char *key, const char *value);
  static bool Get(const char *key, string &value);

private:
  static const char *m_key;
};

#endif