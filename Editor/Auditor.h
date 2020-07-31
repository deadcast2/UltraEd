#ifndef _AUDITOR_H_
#define _AUDITOR_H_

#include <functional>
#include <vector>
#include "Actor.h"

namespace UltraEd
{
    class Scene;

    struct UndoUnit
    {
        std::string name;
        std::function<cJSON*()> undo;
        std::function<void(cJSON*)> redo;
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
        void AddActor(const std::string &name, GUID actorId, GUID groupId = GUID_NULL);
        void DeleteActor(const std::string &name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeActor(const std::string &name, GUID actorId, GUID groupId = GUID_NULL);
        void ChangeScene(const std::string &name);
        std::array<std::string, 2> Titles();
        std::function<void()> PotentialChangeActor(const std::string &name, GUID actorId, GUID groupId);

    private:
        void Add(UndoUnit unit);
        void RunUndo();
        void RunRedo();
        void CleanUp();
        cJSON *SaveState(GUID id, std::function<cJSON*()> save);
        void Lock(std::function<void()> block);

    private:
        std::vector<UndoUnit> m_undoUnits;
        size_t m_position;
        Scene *m_scene;
        std::map<GUID, cJSON *> m_savedStates;
        std::map<std::string, std::tuple<bool, std::function<void()>>> m_potentials;
        const int m_maxUnits;
        bool m_locked;
    };
}

#endif
