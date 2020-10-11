#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <string>
#include <filesystem>
#include <fstream>
#include <boost/signals2/signal.hpp>

namespace UltraEd
{
    class Debug
    {
    private:
        Debug();

    public:
        static Debug &Instance();
        void Connect(std::function<void(std::string)> slot);
        void Info(std::string text);
        void Warning(std::string text);
        void Error(std::string text);

    private:
        std::string *Clean(std::string *text);
        void WriteToLog(const std::string &str);

    private:
        boost::signals2::signal<void(std::string)> m_signal;
        std::ofstream m_logFile;
    };
}

#endif
