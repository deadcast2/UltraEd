#pragma once

#include <map>
#include <string>
#include <functional>
#include "deps/DXSDK/include/d3d8.h"
#include "vendor/cJSON.h"

using namespace std;

namespace UltraEd
{
    struct SavableType
    {
        enum Value { View, Actor };
    };

    typedef struct
    {
        cJSON *object;
        SavableType::Value type;
    } Savable;

    class CSavable
    {
    public:
        CSavable() { m_isDirty = false; }
        virtual ~CSavable() { }
        virtual Savable Save() = 0;
        virtual bool Load(IDirect3DDevice8 *device, cJSON *root) = 0;
        map<string, string> GetResources() { return resources; };
        bool IsDirty() { return m_isDirty; }

    protected:
        template<typename T>
        void Dirty(function<void()> set, const T *var)
        {
            const T before = *var;
            set();
            if (before != *var) m_isDirty = true;
        }
        virtual void SetDirty(bool value) { m_isDirty = value; }
        map<string, string> resources;

    private:
        bool m_isDirty;
    };
}
