#include "Savable.h"

namespace UltraEd
{
    CSavable::CSavable() : m_isDirty(false)
    { }

    map<string, string> CSavable::GetResources() 
    { 
        return m_resources; 
    }

    void CSavable::AddResource(string key, string value)
    {
        m_resources[key] = value;
    }

    bool CSavable::RemoveResource(string key)
    {
        if (m_resources.find(key) != m_resources.end())
        {
            m_resources.erase(key);
            return true;
        }
        return false;
    }

    bool CSavable::IsDirty() 
    { 
        return m_isDirty; 
    }
    
    void CSavable::SetDirty(bool value) 
    { 
        m_isDirty = value; 
    }
}
