#include "Debug.h"
#include "PubSub.h"

namespace UltraEd
{
    void Debug::Info(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            PubSub::Publish("AppendToConsole", &std::string("Info: ").append(text).append("\n"));
        }
    }

    void Debug::Warning(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            PubSub::Publish("AppendToConsole", &std::string("Warning: ").append(text).append("\n"));
        }
    }

    void Debug::Error(std::string text)
    {
        if (Clean(&text)->size() > 0)
        {
            PubSub::Publish("AppendToConsole", &std::string("Error: ").append(text).append("\n"));
        }
    }

    std::string *Debug::Clean(std::string *text)
    {
        text->erase(std::remove(text->begin(), text->end(), '\n'), text->end());
        text->erase(std::remove(text->begin(), text->end(), '\r'), text->end());
        return text;
    }
}
