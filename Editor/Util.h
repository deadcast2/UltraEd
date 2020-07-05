#ifndef _UTIL_H_
#define _UTIL_H_

#include <rpc.h>

using namespace std;

namespace UltraEd
{
    class Util
    {
    public:
        static float Lerp(float time, float start, float end);
        static GUID NewGuid();

    private:
        Util() {};
    };
}

#endif
