#include "Undo.h"
#include "Scene.h"
#include "ResourceExt.h"

namespace UltraEd
{
    CUndo::CUndo(CScene *scene) :
        m_undoUnits(),
        m_position(0),
        m_scene(scene)
    { }

    CUndo::~CUndo()
    {
        CleanUp();
    }

    void CUndo::Undo()
    {
        // Don't adjust actor selections when nothing to undo.
        if (m_position > 0) m_scene->UnselectAll();
        RunUndo();
    }

    void CUndo::Redo()
    {
        // Don't adjust actor selections when nothing to redo.
        if (m_position < m_undoUnits.size()) m_scene->UnselectAll();
        RunRedo();
    }

    void CUndo::RunUndo()
    {
        if (m_position > 0)
        {
            m_position--;
            auto state = m_undoUnits[m_position].undo();
            m_undoUnits[m_position].state = state;

            UpdateMenu();

            // Keep undoing when this unit is a part of a group.
            int nextUndoPos = m_position - 1;
            if (nextUndoPos >= 0
                && m_undoUnits[nextUndoPos].groupId != GUID_NULL
                && m_undoUnits[nextUndoPos].groupId == m_undoUnits[m_position].groupId)
            {
                RunUndo();
            }
        }
    }

    void CUndo::RunRedo()
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

    void CUndo::UpdateMenu()
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

        SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_UNDO,
            reinterpret_cast<LPARAM>(undo.append("\tCtrl+Z").c_str()));
        SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_REDO,
            reinterpret_cast<LPARAM>(redo.append("\tCtrl+Y").c_str()));
    }

    void CUndo::CleanUp()
    {
        for (auto state : m_savedStates)
        {
            cJSON_Delete(state.second);
        }
        m_savedStates.clear();
    }

    cJSON *CUndo::SaveState(GUID id, CSavable *savable)
    {
        // Identifying states avoids storing duplicates.
        if (m_savedStates.find(id) == m_savedStates.end())
        {
            auto state = savable->Save();
            m_savedStates[id] = state;
        }
        return m_savedStates[id];
    }

    cJSON *CUndo::SaveState(GUID id, cJSON *state)
    {
        // Identifying states avoids storing duplicates.
        if (m_savedStates.find(id) == m_savedStates.end())
        {
            m_savedStates[id] = state;
        }
        return m_savedStates[id];
    }

    void CUndo::Reset()
    {
        m_undoUnits.clear();
        m_potentials.clear();
        m_position = 0;
        UpdateMenu();
        CleanUp();
    }

    void CUndo::Add(UndoUnit unit)
    {
        // Clear redo history when adding new unit and not at head.
        while (m_undoUnits.size() != m_position)
        {
            m_undoUnits.pop_back();
        }

        m_undoUnits.push_back(unit);
        m_position = m_undoUnits.size();
        UpdateMenu();
    }

    void CUndo::AddActor(string name, GUID actorId, GUID groupId)
    {
        GUID redoStateId = CUtil::NewGuid();
        Add({
            string("Add ").append(name),
            [=]() {
                auto actor = m_scene->GetActor(actorId);
                auto state = SaveState(redoStateId, actor.get());
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

    void CUndo::DeleteActor(string name, GUID actorId, GUID groupId)
    {
        auto state = SaveState(CUtil::NewGuid(), m_scene->GetActor(actorId).get());
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

    void CUndo::ChangeActor(string name, GUID actorId, GUID groupId)
    {
        auto state = SaveState(CUtil::NewGuid(), m_scene->GetActor(actorId).get());
        GUID redoStateId = CUtil::NewGuid();
        Add({
            name,
            [=]() {
                auto actor = m_scene->GetActor(actorId);
                auto oldState = SaveState(redoStateId, actor.get());

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

    function<void()> CUndo::PotentialChangeActor(string name, GUID actorId, GUID groupId)
    {
        if (m_potentials.find(groupId) != m_potentials.end())
            return get<1>(m_potentials[groupId]);

        return get<1>(m_potentials[groupId]) = [=]() {
            if (get<0>(m_potentials[groupId])) return;
            get<0>(m_potentials[groupId]) = true;
            ChangeActor(name, actorId, groupId);
        };
    }

    void CUndo::ChangeScene(string name)
    {
        GUID redoStateId = CUtil::NewGuid();
        auto sceneState = SaveState(CUtil::NewGuid(), m_scene);
        Add({
            name,
            [=]() {
                auto oldState = SaveState(redoStateId, m_scene);

                m_scene->Load(sceneState);

                // Return state saved before restore so system can "undo" to this point.
                return oldState;
            },
            [=](cJSON *oldState) {
                m_scene->Load(oldState);
            }
        });
    }
}
