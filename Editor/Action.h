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
        GUID groupId;
        Savable state;
    };

    class CAction
    {
    public:
        CAction();
        void Undo();
        void Redo();
        void Reset();
        void AddActor(string name, CScene *scene, GUID actorId);
        void DeleteActor(string name, CScene *scene, GUID actorId);
        void ChangeActor(string name, CScene *scene, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(string name, CScene *scene, Savable actorState, GUID actorId, GUID groupId = GUID_NULL);

    private:
        void Add(Action action);
        vector<Action> m_actions;
        size_t m_position;
    };
}
