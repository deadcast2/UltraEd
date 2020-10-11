#include "Debug.h"
#include <ctime>

namespace UltraEd
{
    Debug &Debug::Instance()
    {
        static Debug instance;
        return instance;
    }

    Debug::Debug() : m_logFile(std::filesystem::current_path() / "applog.txt", std::ofstream::app)
    { }

    void Debug::Connect(std::function<void(std::string)> slot)
    {
        m_signal.connect(slot);
    }

    void Debug::Info(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            auto str = std::string("Info: ").append(text).append("\n");
            m_signal(str);

            WriteToLog(str);
        }
    }

    void Debug::Warning(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            auto str = std::string("Warning: ").append(text).append("\n");
            m_signal(str);
            
            WriteToLog(str);
        }
    }

    void Debug::Error(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            auto str = std::string("Error: ").append(text).append("\n");
            m_signal(str);
            
            WriteToLog(str);
        }
    }

    std::string *Debug::Clean(std::string *text)
    {
        text->erase(std::remove(text->begin(), text->end(), '\n'), text->end());
        text->erase(std::remove(text->begin(), text->end(), '\r'), text->end());
        return text;
    }

    void Debug::WriteToLog(const std::string &str)
    {
        auto dateTime = std::time(nullptr);
        m_logFile << std::string(str).insert(0, std::ctime(&dateTime));
    }
}
