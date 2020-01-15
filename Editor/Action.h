#pragma once

#include <array>
#include <functional>
#include <vector>

using namespace std;

namespace UltraEd 
{
    class CAction
    {
    public:
        CAction();
        void Undo();
        void Redo();
        void Add(function<void()> undo, function<void()> redo);

    private:
        vector<array<function<void()>, 2>> m_actions;
        int m_position;
    };
}
