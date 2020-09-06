#include "Debug.h"

namespace UltraEd
{
    Debug &Debug::Instance()
    {
        static Debug instance;
        return instance;
    }

    void Debug::Connect(std::function<void(std::string)> slot)
    {
        m_signal.connect(slot);
    }

    void Debug::Info(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
          m_signal(std::string("Info: ").append(text).append("\n"));
        }
    }

    void Debug::Warning(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            m_signal(std::string("Warning: ").append(text).append("\n"));
        }
    }

    void Debug::Error(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            m_signal(std::string("Error: ").append(text).append("\n"));
        }
    }

    std::string *Debug::Clean(std::string *text)
    {
        text->erase(std::remove(text->begin(), text->end(), '\n'), text->end());
        text->erase(std::remove(text->begin(), text->end(), '\r'), text->end());
        return text;
    }
}
