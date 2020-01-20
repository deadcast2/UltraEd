#pragma once

#include <tuple>
#include <functional>
#include <vector>
#include "Actor.h"

using namespace std;

namespace UltraEd 
{
    class CScene;

    struct Action
    {
        string name;
        function<Savable()> undo;
        function<void(Savable)> redo;
        Savable state;
    };

    class CAction
    {
    public:
        CAction();
        void Undo();
        void Redo();
        void Reset();
        void AddActor(string name, CScene *scene, shared_ptr<CActor> actor);
        void DeleteActor(string name, CScene *scene, shared_ptr<CActor> actor);
        void ChangeActor(string name, CScene *scene, GUID actorId);
        void SelectActor(string name, CScene *scene, GUID actorId);

    private:
        void Add(Action action);
        vector<Action> m_actions;
        size_t m_position;
    };
}
