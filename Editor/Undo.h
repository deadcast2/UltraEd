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
        CUndo();
        void Undo(CScene *scene);
        void Redo(CScene *scene);
        void Reset();
        void AddActor(string name, CScene *scene, GUID actorId);
        void DeleteActor(string name, CScene *scene, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(string name, CScene *scene, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(string name, CScene *scene, Savable actorState, GUID actorId, GUID groupId = GUID_NULL);

    private:
        void Add(UndoUnit unit);
        void Undo();
        void Redo();
        vector<UndoUnit> m_undoUnits;
        size_t m_position;
    };
}
