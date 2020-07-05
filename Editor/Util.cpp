#include "Util.h"

namespace UltraEd
{
    float Util::Lerp(float time, float start, float end)
    {
        return (1 - time) * start + time * end;
    }

    GUID Util::NewGuid()
    {
        GUID guid;
        if (CoCreateGuid(&guid) == S_OK)
        {
            return guid;
        }
        return GUID_NULL;
    }
}
