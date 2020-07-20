#ifndef _AUDITOR_H_
#define _AUDITOR_H_

#include <functional>
#include <vector>
#include "Actor.h"

using namespace std;

namespace UltraEd
{
    class Scene;

    struct UndoUnit
    {
        string name;
        function<cJSON*()> undo;
        function<void(cJSON*)> redo;
        GUID groupId;
        cJSON *state;
    };

    class Auditor
    {
    public:
        Auditor(Scene *scene);
        ~Auditor();
        void Undo();
        void Redo();
        void Reset();
        void AddActor(const string &name, GUID actorId, GUID groupId = GUID_NULL);
        void DeleteActor(const string &name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(const string &name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeScene(const string &name);
        array<string, 2> Titles();
        function<void()> PotentialChangeActor(const string &name, GUID actorId, GUID groupId);

    private:
        void Add(UndoUnit unit);
        void RunUndo();
        void RunRedo();
        void CleanUp();
        cJSON *SaveState(GUID id, function<cJSON*()> save);
        vector<UndoUnit> m_undoUnits;
        size_t m_position;
        Scene *m_scene;
        map<GUID, cJSON *> m_savedStates;
        map<string, tuple<bool, function<void()>>> m_potentials;
        const int m_maxUnits;
    };
}

#endif
