#include "Savable.h"

namespace UltraEd
{
    Savable::Savable() : m_isDirty(false)
    { }

    std::map<std::string, std::string> Savable::GetResources()
    { 
        return m_resources; 
    }

    void Savable::AddResource(const std::string &key, const std::string &value)
    {
        m_resources[key] = value;
    }

    void Savable::DeleteResource(const std::string &key)
    {
        m_resources.erase(key);
    }

    bool Savable::IsDirty() 
    { 
        return m_isDirty; 
    }
    
    void Savable::SetDirty(bool value) 
    { 
        m_isDirty = value; 
    }
}
