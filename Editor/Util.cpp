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

string CUtil::NewResourceName(int count)
{
  char alpha = 'A';
  string name("UER_");
  for(int i = 0; i < count; i++)
  {
    alpha++;
  }
  name += alpha;
  return name;
}

char *CUtil::ReplaceString(const char *str, const char *from, const char *to)
{
  /* http://creativeandcritical.net/str-replace-c */
  /* Portable C string and wide string replacement functions: repl_str and repl_wcs */

  /* Adjust each of the below values to suit your needs. */
  /* Increment positions cache size initially by this number. */
  size_t cache_sz_inc = 16;
  /* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
  const size_t cache_sz_inc_factor = 3;
  /* But never increment capacity by more than this number. */
  const size_t cache_sz_inc_max = 1048576;
  
  char *pret, *ret = NULL;
  const char *pstr2, *pstr = str;
  size_t i, count = 0;
#if (__STDC_VERSION__ >= 199901L)
  uintptr_t *pos_cache_tmp, *pos_cache = NULL;
#else
  ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
#endif
  size_t cache_sz = 0;
  size_t cpylen, orglen, retlen, tolen, fromlen = strlen(from);
  
  /* Find all matches and cache their positions. */
  while ((pstr2 = strstr(pstr, from)) != NULL) {
    count++;
    
    /* Increase the cache size when necessary. */
    if (cache_sz < count) {
      cache_sz += cache_sz_inc;
      pos_cache_tmp = (int*)realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
      if (pos_cache_tmp == NULL) {
        goto end_repl_str;
      } else pos_cache = pos_cache_tmp;
      cache_sz_inc *= cache_sz_inc_factor;
      if (cache_sz_inc > cache_sz_inc_max) {
        cache_sz_inc = cache_sz_inc_max;
      }
    }
    
    pos_cache[count-1] = pstr2 - str;
    pstr = pstr2 + fromlen;
  }
  
  orglen = pstr - str + strlen(pstr);
  
  /* Allocate memory for the post-replacement string. */
  if (count > 0) {
    tolen = strlen(to);
    retlen = orglen + (tolen - fromlen) * count;
  } else	retlen = orglen;
  ret = (char*)malloc(retlen + 1);
  if (ret == NULL) {
    goto end_repl_str;
  }
  
  if (count == 0) {
    /* If no matches, then just duplicate the string. */
    strcpy(ret, str);
  } else {
		/* Otherwise, duplicate the string whilst performing
    * the replacements using the position cache. */
    pret = ret;
    memcpy(pret, str, pos_cache[0]);
    pret += pos_cache[0];
    for (i = 0; i < count; i++) {
      memcpy(pret, to, tolen);
      pret += tolen;
      pstr = str + pos_cache[i] + fromlen;
      cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - fromlen;
      memcpy(pret, pstr, cpylen);
      pret += cpylen;
    }
    ret[retlen] = '\0';
  }
  
end_repl_str:
/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
  free(pos_cache);
  return ret;
}

