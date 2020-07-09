#ifndef _UTIL_H_
#define _UTIL_H_

#define CLSID_LENGTH 40

#include <functional>
#include <rpc.h>
#include <string>
#include <vector>

using namespace std;

namespace UltraEd
{
    class Util
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
        static void RunAction(HWND statusBar, const char *message, const function<void()> &action);

    private:
        Util() {};
    };
}

#endif
