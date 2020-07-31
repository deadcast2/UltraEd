#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <string>

namespace UltraEd
{
    class Dialog
    {
    public:
        static bool Open(const char *title, const char *filter, std::string &file);
        static bool Save(const char *title, const char *filter, std::string &file);

    private:
        Dialog() {}
    };
}

#endif
