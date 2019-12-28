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
        enum Value { Actor, Scene, View };
    };

    typedef struct
    {
        cJSON *object;
        SavableType::Value type;
    } Savable;

    class CSavable
    {
    public:
        CSavable();
        virtual ~CSavable() { }
        virtual Savable Save() = 0;
        virtual bool Load(IDirect3DDevice8 *device, cJSON *root) = 0;
        map<string, string> GetResources();
        void AddResource(const string &key, const string &value);
        void RemoveResource(const string &key);
        bool IsDirty();

    protected:
        virtual void SetDirty(bool value);
        template<typename T>
        void Dirty(function<void()> set, const T *var)
        {
            const T before = *var;
            set();
            if (before != *var) m_isDirty = true;
        }

    private:
        map<string, string> m_resources;
        bool m_isDirty;
    };
}
