#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <string>
#include <boost/signals2/signal.hpp>

namespace UltraEd
{
    class Debug
    {
    private:
        Debug() {}

    public:
        static Debug &Instance();
        void Connect(std::function<void(std::string)> slot);
        void Info(std::string text);
        void Warning(std::string text);
        void Error(std::string text);

    private:
        std::string *Clean(std::string *text);
        boost::signals2::signal<void(std::string)> m_signal;
    };
}

#endif
