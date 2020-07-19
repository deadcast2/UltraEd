#include "Auditor.h"
#include "Scene.h"

namespace UltraEd
{
    Auditor::Auditor(Scene *scene) :
        m_undoUnits(),
        m_position(0),
        m_scene(scene),
        m_savedStates(),
        m_potentials(),
        m_maxUnits(100)
    { }

    Auditor::~Auditor()
    {
        CleanUp();
    }

    void Auditor::Undo()
    {
        // Don't adjust actor selections when nothing to undo.
        if (m_position > 0) m_scene->UnselectAll();
        RunUndo();
    }

    void Auditor::Redo()
    {
        // Don't adjust actor selections when nothing to redo.
        if (m_position < m_undoUnits.size()) m_scene->UnselectAll();
        RunRedo();
    }

    void Auditor::RunUndo()
    {
        if (m_position > 0)
        {
            m_position--;
            auto state = m_undoUnits[m_position].undo();
            m_undoUnits[m_position].state = state;

            UpdateMenu();

            // Keep undoing when this unit is a part of a group.
            int nextUndoPos = static_cast<int>(m_position - 1);
            if (nextUndoPos >= 0
                && m_undoUnits[nextUndoPos].groupId != GUID_NULL
                && m_undoUnits[nextUndoPos].groupId == m_undoUnits[m_position].groupId)
            {
                RunUndo();
            }
        }
    }

    void Auditor::RunRedo()
    {
        if (m_position < m_undoUnits.size())
        {
            auto state = m_undoUnits[m_position].state;
            m_undoUnits[m_position].redo(state);
            m_position++;

            UpdateMenu();

            // Keep redoing when this unit is a part of a group.
            if (m_position < m_undoUnits.size()
                && m_undoUnits[m_position].groupId != GUID_NULL
                && m_undoUnits[m_position - 1].groupId == m_undoUnits[m_position].groupId)
            {
                RunRedo();
            }
        }
    }

    void Auditor::UpdateMenu()
    {
        string undo("Undo");
        string redo("Redo");

        if (!m_undoUnits.empty())
        {
            // At most recent undo unit so clear redo message.
            if (m_position == m_undoUnits.size())
            {
                undo.append(" - ").append(m_undoUnits[m_position - 1].name);
            }
            // At last undo unit so clear undo message.
            else if (m_position == 0)
            {
                redo.append(" - ").append(m_undoUnits[m_position].name);
            }
            // In the middle so update both undo and redo messages.
            else if (m_position < m_undoUnits.size())
            {
                undo.append(" - ").append(m_undoUnits[m_position - 1].name);
                redo.append(" - ").append(m_undoUnits[m_position].name);
            }
        }

        //SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_UNDO,
        //    reinterpret_cast<LPARAM>(undo.append("\tCtrl+Z").c_str()));
        //SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_REDO,
        //    reinterpret_cast<LPARAM>(redo.append("\tCtrl+Y").c_str()));
    }

    void Auditor::CleanUp()
    {
        for (auto state : m_savedStates)
        {
            cJSON_Delete(state.second);
        }
        m_savedStates.clear();
    }

    cJSON *Auditor::SaveState(GUID id, function<cJSON*()> save)
    {
        // Identifying states avoids storing duplicates.
        if (m_savedStates.find(id) == m_savedStates.end())
        {
            m_savedStates[id] = save();
        }
        return m_savedStates[id];
    }

    void Auditor::Reset()
    {
        m_undoUnits.clear();
        m_potentials.clear();
        m_position = 0;
        UpdateMenu();
        CleanUp();
    }

    void Auditor::Add(UndoUnit unit)
    {
        // Clear redo history when adding new unit and not at head.
        while (m_undoUnits.size() != m_position)
        {
            m_undoUnits.pop_back();
        }

        // Prevent recorded undo units from growing too large by 
        // shifting all units left and replacing the last with newest unit.
        if (m_undoUnits.size() == m_maxUnits)
        {
            rotate(m_undoUnits.begin(), m_undoUnits.begin() + 1, m_undoUnits.end());
            m_undoUnits.pop_back();
        }

        // Add new unit and set current position to it.
        m_undoUnits.push_back(unit);
        m_position = m_undoUnits.size();
        UpdateMenu();
    }

    void Auditor::AddActor(const string &name, GUID actorId, GUID groupId)
    {
        GUID redoStateId = Util::NewGuid();
        Add({
            string("Add ").append(name),
            [=]() {
                auto actor = m_scene->GetActor(actorId);
                auto state = SaveState(redoStateId, [=]() { return actor->Save(); });
                m_scene->Delete(actor);
                return state;
            },
            [=](cJSON *oldState) {
                m_scene->RestoreActor(oldState);
                m_scene->SelectActorById(actorId);
            },
            groupId
        });
    }

    void Auditor::DeleteActor(const string &name, GUID actorId, GUID groupId)
    {
        auto state = SaveState(Util::NewGuid(), [=]() { return m_scene->GetActor(actorId)->Save(); });
        Add({
            string("Delete ").append(name),
            [=]() {
                m_scene->RestoreActor(state);
                m_scene->SelectActorById(actorId);
                return state;
            },
            [=](cJSON *oldState) {
                m_scene->Delete(m_scene->GetActor(actorId));
            },
            groupId
        });
    }

    void Auditor::ChangeActor(const string &name, GUID actorId, GUID groupId)
    {
        auto state = SaveState(Util::NewGuid(), [=]() { return m_scene->GetActor(actorId)->Save(); });
        GUID redoStateId = Util::NewGuid();
        Add({
            name,
            [=]() {
                auto actor = m_scene->GetActor(actorId);
                auto oldState = SaveState(redoStateId, [=]() { return actor->Save(); });

                m_scene->RestoreActor(state);
                m_scene->SelectActorById(actorId, false);

                // Return state saved before restore so system can "undo" to this point.
                return oldState;
            },
            [=](cJSON *oldState) {
                m_scene->RestoreActor(oldState);
                m_scene->SelectActorById(actorId, false);
            },
            groupId
        });
    }

    function<void()> Auditor::PotentialChangeActor(const string &name, GUID actorId, GUID groupId)
    {
        string uniqueId = Util::GuidToString(actorId).append(Util::GuidToString(groupId));

        if (m_potentials.find(uniqueId) != m_potentials.end())
            return get<1>(m_potentials[uniqueId]);

        return get<1>(m_potentials[uniqueId]) = [=]() {
            if (get<0>(m_potentials[uniqueId])) return;
            get<0>(m_potentials[uniqueId]) = true;
            ChangeActor(name, actorId, groupId);
        };
    }

    void Auditor::ChangeScene(const string &name)
    {
        GUID redoStateId = Util::NewGuid();
        auto sceneState = SaveState(Util::NewGuid(), [=]() { return m_scene->PartialSave(NULL); });
        Add({
            name,
            [=]() {
                auto oldState = SaveState(redoStateId, [=]() { return m_scene->PartialSave(NULL); });

                m_scene->PartialLoad(sceneState);

                // Return state saved before restore so system can "undo" to this point.
                return oldState;
            },
            [=](cJSON *oldState) {
                m_scene->PartialLoad(oldState);
            }
        });
    }
}
