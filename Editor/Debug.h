#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <string>

namespace UltraEd
{
    class Debug
    {
    public:
        static void Info(std::string text);
        static void Warning(std::string text);
        static void Error(std::string text);

    private:
        static std::string *Clean(std::string *text);
    };
}

#endif
