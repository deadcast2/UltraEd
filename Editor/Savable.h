#ifndef _SAVABLE_H_
#define _SAVABLE_H_

#include <map>
#include <string>
#include <functional>
#include <cJSON/cJSON.h>

namespace UltraEd
{
    class Savable
    {
    public:
        Savable();
        virtual ~Savable() { }
        virtual cJSON *Save() = 0;
        virtual bool Load(cJSON *root) = 0;
        std::map<std::string, std::string> GetResources();
        void AddResource(const std::string &key, const std::string &value);
        void DeleteResource(const std::string &key);
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
        std::map<std::string, std::string> m_resources;
        bool m_isDirty;
    };
}

#endif
