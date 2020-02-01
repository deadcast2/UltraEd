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
        if(m_position > 0) scene->UnselectAll();
        Undo();
    }

    void CUndo::Redo(CScene *scene)
    {
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

    void CUndo::Add(UndoUnit action)
    {
        // Clear redo history when adding new action and not at head.
        while (m_undoUnits.size() != m_position)
        {
            m_undoUnits.pop_back();
        }

        m_undoUnits.push_back(action);
        m_position = m_undoUnits.size();
    }

    void CUndo::AddActor(string name, CScene *scene, GUID actorId)
    {
        UndoUnit action = {
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
        Add(action);
    }

    void CUndo::DeleteActor(string name, CScene *scene, GUID actorId, GUID groupId)
    {
        auto state = scene->GetActor(actorId)->Save();
        UndoUnit action = {
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
        Add(action);
    }

    void CUndo::ChangeActor(string name, CScene *scene, GUID actorId, GUID groupId)
    {
        auto state = scene->GetActor(actorId)->Save();
        ChangeActor(name, scene, state, actorId, groupId);
    }

    void CUndo::ChangeActor(string name, CScene *scene, Savable actorState, GUID actorId, GUID groupId)
    {
        UndoUnit action = {
            name,
            [=]() {
                auto actor = scene->GetActor(actorId);
                auto futureState = actor->Save();
                scene->Restore(actorState.object);
                scene->SelectActorById(actorId, false);
                return futureState;
            },
            [=](Savable savable) {
                scene->Restore(savable.object);
                scene->SelectActorById(actorId, false);
            },
            groupId
        };
        Add(action);
    }
}
