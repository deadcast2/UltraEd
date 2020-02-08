#pragma once

#include <map>
#include <string>
#include <functional>
#include "deps/DXSDK/include/d3d8.h"
#include "vendor/cJSON.h"

using namespace std;

namespace UltraEd
{
    class CSavable
    {
    public:
        CSavable();
        virtual ~CSavable() { }
        virtual cJSON *Save() = 0;
        virtual bool Load(cJSON *root) = 0;
        map<string, string> GetResources();
        void AddResource(const string &key, const string &value);
        void DeleteResource(const string &key);
        bool IsDirty();

    protected:
        virtual void SetDirty(bool value);
        template<typename T>
        bool Dirty(function<void()> set, const T *var)
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
        map<string, string> m_resources;
        bool m_isDirty;
    };
}
