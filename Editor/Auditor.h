#ifndef _AUDITOR_H_
#define _AUDITOR_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <nlohmann/json.hpp>
#include <functional>
#include <map>
#include <set>
#include <tuple>
#include <vector>
#include "Actor.h"

using json = nlohmann::json;
using uuid = boost::uuids::uuid;

namespace UltraEd
{
    class Scene;

    struct UndoUnit
    {
        std::string name;
        std::function<json()> undo;
        std::function<void(const json&)> redo;
        uuid groupId;
        json state;
    };

    class Auditor
    {
    public:
        Auditor(Scene *scene);
        ~Auditor();
        void Undo();
        void Redo();
        void Reset();
        void AddActor(const std::string &name, const uuid &actorId, const uuid &groupId = boost::uuids::nil_uuid());
        void DeleteActor(const std::string &name, const uuid &actorId, const uuid &groupId = boost::uuids::nil_uuid());
        void ChangeActor(const std::string &name, const uuid &actorId, const uuid &groupId = boost::uuids::nil_uuid());
        void ParentActor(const std::string &name, const uuid &actorId, const uuid &groupId = boost::uuids::nil_uuid());
        void ChangeScene(const std::string &name);
        std::array<std::string, 2> Titles();
        std::function<void()> PotentialChangeActor(const std::string &name, const uuid &actorId, const uuid &groupId);

    private:
        void Add(UndoUnit unit);
        void RunUndo();
        void RunRedo();
        void CleanUp();
        json SaveState(const uuid &id, std::function<json()> save);
        void Lock(std::function<void()> block);

    private:
        std::vector<UndoUnit> m_undoUnits;
        size_t m_position;
        Scene *m_scene;
        std::map<uuid, json> m_savedStates;
        std::map<std::string, std::tuple<bool, std::function<void()>>> m_potentials;
        const int m_maxUnits;
        bool m_locked;
    };
}

#endif
