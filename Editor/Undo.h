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
        function<cJSON*()> undo;
        function<void(cJSON*)> redo;
        GUID groupId;
        cJSON *state;
    };

    class CUndo
    {
    public:
        CUndo(CScene *scene);
        ~CUndo();
        void Undo();
        void Redo();
        void Reset();
        void AddActor(string name, GUID actorId, GUID groupId = GUID_NULL);
        void DeleteActor(string name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(string name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeScene(string name);
        function<void()> PotentialChangeActor(string name, GUID actorId, GUID groupId);

    private:
        void Add(UndoUnit unit);
        void RunUndo();
        void RunRedo();
        void UpdateMenu();
        void CleanUp();
        cJSON *SaveState(GUID id, function<cJSON*()> save);
        vector<UndoUnit> m_undoUnits;
        size_t m_position;
        CScene *m_scene;
        map<GUID, cJSON *> m_savedStates;
        map<string, tuple<bool, function<void()>>> m_potentials;
    };
}
