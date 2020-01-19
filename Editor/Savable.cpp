#include "Savable.h"

namespace UltraEd
{
    CSavable::CSavable() : m_isDirty(false)
    { }

    map<string, string> CSavable::GetResources() 
    { 
        return m_resources; 
    }

    void CSavable::AddResource(const string &key, const string &value)
    {
        m_resources[key] = value;
    }

    void CSavable::DeleteResource(const string &key)
    {
        m_resources.erase(key);
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
