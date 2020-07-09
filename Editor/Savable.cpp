#include "Savable.h"

namespace UltraEd
{
    Savable::Savable() : m_isDirty(false)
    { }

    map<string, string> Savable::GetResources() 
    { 
        return m_resources; 
    }

    void Savable::AddResource(const string &key, const string &value)
    {
        m_resources[key] = value;
    }

    void Savable::DeleteResource(const string &key)
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
