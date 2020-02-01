#include "Undo.h"
#include "Scene.h"

namespace UltraEd
{
    CUndo::CUndo() :
        m_undoUnits(),
        m_position(0)
    { }

    void CUndo::Undo(CScene *scene)
    {
        // Don't adjust actor selections when nothing to undo.
        if (m_position > 0) scene->UnselectAll();
        Undo();
    }

    void CUndo::Redo(CScene *scene)
    {
        // Don't adjust actor selections when nothing to redo.
        if (m_position < m_undoUnits.size()) scene->UnselectAll();
        Redo();
    }

    void CUndo::Undo()
    {
        if (m_position > 0)
        {
            m_position--;
            auto state = m_undoUnits[m_position].undo();
            m_undoUnits[m_position].state = state;
            CDebug::Log("Undo - %s\n", m_undoUnits[m_position].name.c_str());

            // Keep undoing when this unit is a part of a group.
            int nextUndoPos = m_position - 1;
            if (nextUndoPos >= 0
                && m_undoUnits[nextUndoPos].groupId != GUID_NULL
                && m_undoUnits[nextUndoPos].groupId == m_undoUnits[m_position].groupId)
            {
                Undo();
            }
        }
    }

    void CUndo::Redo()
    {
        if (m_position < m_undoUnits.size())
        {
            auto state = m_undoUnits[m_position].state;
            m_undoUnits[m_position].redo(state);
            CDebug::Log("Redo - %s\n", m_undoUnits[m_position].name.c_str());
            m_position++;

            // Keep redoing when this unit is a part of a group.
            if (m_position < m_undoUnits.size()
                && m_undoUnits[m_position].groupId != GUID_NULL
                && m_undoUnits[m_position - 1].groupId == m_undoUnits[m_position].groupId)
            {
                Redo();
            }
        }
    }

    void CUndo::Reset()
    {
        m_undoUnits.clear();
        m_position = 0;
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
    }

    void CUndo::AddActor(string name, CScene *scene, GUID actorId)
    {
        UndoUnit unit = {
            string("Add ").append(name),
            [=]() {
                auto actor = scene->GetActor(actorId);
                auto state = actor->Save();
                scene->Delete(actor);
                return state;
            },
            [=](Savable savable) {
                scene->Restore(savable.object);
                scene->SelectActorById(actorId);
            }
        };
        Add(unit);
    }

    void CUndo::DeleteActor(string name, CScene *scene, GUID actorId, GUID groupId)
    {
        auto state = scene->GetActor(actorId)->Save();
        UndoUnit unit = {
            string("Delete ").append(name),
            [=]() {
                scene->Restore(state.object);
                scene->SelectActorById(actorId);
                return state;
            },
            [=](Savable savable) {
                scene->Delete(scene->GetActor(actorId));
            },
            groupId
        };
        Add(unit);
    }

    void CUndo::ChangeActor(string name, CScene *scene, GUID actorId, GUID groupId)
    {
        auto state = scene->GetActor(actorId)->Save();
        ChangeActor(name, scene, state, actorId, groupId);
    }

    void CUndo::ChangeActor(string name, CScene *scene, Savable actorState, GUID actorId, GUID groupId)
    {
        UndoUnit unit = {
            name,
            [=]() {
                auto actor = scene->GetActor(actorId);
                auto oldState = actor->Save();

                scene->Restore(actorState.object);
                scene->SelectActorById(actorId, false);

                // Return state saved before restore so system can "undo" to this point.
                return oldState;
            },
            [=](Savable savable) {
                scene->Restore(savable.object);
                scene->SelectActorById(actorId, false);
            },
            groupId
        };
        Add(unit);
    }
}
