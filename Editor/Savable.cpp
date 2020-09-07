#include "Savable.h"

namespace UltraEd
{
    Savable::Savable() : m_isDirty(false)
    { }

    bool Savable::IsDirty() 
    { 
        return m_isDirty; 
    }
    
    void Savable::SetDirty(bool value) 
    { 
        m_isDirty = value; 
    }
}
