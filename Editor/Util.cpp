#include "Util.h"

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

