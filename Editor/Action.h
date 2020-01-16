#pragma once

#include <array>
#include <functional>
#include <vector>
#include "Actor.h"

using namespace std;

namespace UltraEd 
{
    class CScene;

    class CAction
    {
    public:
        CAction();
        void Undo();
        void Redo();
        void AddActor(CScene *scene, shared_ptr<CActor> actor);
        void DeleteActor(CScene *scene, shared_ptr<CActor> actor);

    private:
        void Add(function<void()> undo, function<void()> redo);
        vector<array<function<void()>, 2>> m_actions;
        size_t m_position;
    };
}
