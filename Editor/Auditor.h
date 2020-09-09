#ifndef _AUDITOR_H_
#define _AUDITOR_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <cJSON/cJSON.h>
#include <functional>
#include <map>
#include <tuple>
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
        boost::uuids::uuid groupId;
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
        void AddActor(const std::string &name, const boost::uuids::uuid &actorId, 
            const boost::uuids::uuid &groupId = boost::uuids::nil_uuid());
        void DeleteActor(const std::string &name, const boost::uuids::uuid &actorId, 
            const boost::uuids::uuid &groupId = boost::uuids::nil_uuid());
        void ChangeActor(const std::string &name, const boost::uuids::uuid &actorId, 
            const boost::uuids::uuid &groupId = boost::uuids::nil_uuid());
        void ChangeScene(const std::string &name);
        std::array<std::string, 2> Titles();
        std::function<void()> PotentialChangeActor(const std::string &name, 
            const boost::uuids::uuid &actorId, const boost::uuids::uuid &groupId);

    private:
        void Add(UndoUnit unit);
        void RunUndo();
        void RunRedo();
        void CleanUp();
        cJSON *SaveState(const boost::uuids::uuid &id, std::function<cJSON*()> save);
        void Lock(std::function<void()> block);

    private:
        std::vector<UndoUnit> m_undoUnits;
        size_t m_position;
        Scene *m_scene;
        std::map<boost::uuids::uuid, cJSON *> m_savedStates;
        std::map<std::string, std::tuple<bool, std::function<void()>>> m_potentials;
        const int m_maxUnits;
        bool m_locked;
    };
}

#endif
