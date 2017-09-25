#include "Util.h"

float CUtil::Lerp(float time, float start, float end)
{
  return (1 - time) * start + time * end;
}

GUID CUtil::NewGuid()
{
  GUID guid;
  if(CoCreateGuid(&guid) == S_OK)
  {
    return guid;
  }
  return GUID_NULL;
}

GUID CUtil::StringToGuid(const char *guid)
{
  GUID _guid;
  wchar_t wideString[CLSID_LENGTH];

  mbstowcs(wideString, guid, CLSID_LENGTH);

  if(IIDFromString(wideString, &_guid) == S_OK)
  {
    return _guid;
  }

  return GUID_NULL;
}

string CUtil::GuidToString(GUID guid)
{
  char guidBuffer[CLSID_LENGTH];
  wchar_t guidWide[CLSID_LENGTH];

  StringFromGUID2(guid, guidWide, CLSID_LENGTH);
  wcstombs(guidBuffer, guidWide, CLSID_LENGTH);

  return string(guidBuffer);
}

string CUtil::RootPath()
{
    char pathBuffer[MAX_PATH];
    GetModuleFileName(NULL, pathBuffer, MAX_PATH);
    string pathString(pathBuffer);
    pathString = pathString.substr(0, pathString.find_last_of("\\/"));
    pathString.append("\\Library");
    return pathString;
}

