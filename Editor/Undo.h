#pragma once

#include <functional>
#include <vector>
#include "Actor.h"

using namespace std;

namespace UltraEd
{
    class CScene;

    struct UndoUnit
    {
        string name;
        function<Savable()> undo;
        function<void(Savable)> redo;
        GUID groupId;
        Savable state;
    };

    class CUndo
    {
    public:
        CUndo(CScene *scene);
        void Undo();
        void Redo();
        void Reset();
        void AddActor(string name, GUID actorId, GUID groupId = GUID_NULL);
        void DeleteActor(string name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(string name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(string name, Savable actorState, GUID actorId, GUID groupId = GUID_NULL);

    private:
        void Add(UndoUnit unit);
        void RunUndo();
        void RunRedo();
        void UpdateMenu();
        vector<UndoUnit> m_undoUnits;
        size_t m_position;
        CScene *m_scene;
    };
}
