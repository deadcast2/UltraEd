#pragma once

#include <tuple>
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
        void Reset();
        void AddActor(CScene *scene, shared_ptr<CActor> actor);
        void DeleteActor(CScene *scene, shared_ptr<CActor> actor);
        void ChangeActor(CScene *scene, GUID actorId);

    private:
        void Add(function<Savable()> undo, function<void(Savable)> redo);
        vector<tuple<function<Savable()>, function<void(Savable)>, Savable>> m_actions;
        size_t m_position;
    };
}
