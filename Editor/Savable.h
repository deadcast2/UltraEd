#ifndef _SAVABLE_H_
#define _SAVABLE_H_

#include <functional>
#include <nlohmann/json.hpp>

namespace UltraEd
{
    class Savable
    {
    public:
        Savable();
        virtual ~Savable() { }
        virtual nlohmann::json Save() = 0;
        virtual void Load(const nlohmann::json &root) = 0;
        bool IsDirty();

    protected:
        virtual void SetDirty(bool value);
        template<typename T>
        bool Dirty(std::function<void()> set, const T *var)
        {
            const T before = *var;
            
            set();
            
            if (before != *var)
            {
                m_isDirty = true;
                return true;
            }

            return false;
        }

    private:
        bool m_isDirty;
    };
}

#endif
