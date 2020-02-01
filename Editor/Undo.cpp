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
        char buffer[1024];
        int lookAhead = m_position + 1;

        // At most recent undo unit so clear redo message.
        if (m_position == m_undoUnits.size())
        {
            sprintf(buffer, "Undo - %s\tCtrl+Z", m_undoUnits[m_position - 1].name.c_str());
            SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_UNDO, reinterpret_cast<LPARAM>(buffer));

            sprintf(buffer, "Redo\tCtrl+Y");
            SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_REDO, reinterpret_cast<LPARAM>(buffer));
        }
        // At last undo unit so clear undo message.
        else if (m_position == 0)
        {
            sprintf(buffer, "Undo\tCtrl+Z");
            SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_UNDO, reinterpret_cast<LPARAM>(buffer));

            sprintf(buffer, "Redo - %s\tCtrl+Y", m_undoUnits[m_position].name.c_str());
            SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_REDO, reinterpret_cast<LPARAM>(buffer));
        }
        // In the middle so update both undo and redo messages.
        else if (m_position < m_undoUnits.size())
        {
            sprintf(buffer, "Undo - %s\tCtrl+Z", m_undoUnits[m_position - 1].name.c_str());
            SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_UNDO, reinterpret_cast<LPARAM>(buffer));

            sprintf(buffer, "Redo - %s\tCtrl+Y", m_undoUnits[m_position].name.c_str());
            SendMessage(m_scene->GetWndHandle(), WM_COMMAND, UPDATE_REDO, reinterpret_cast<LPARAM>(buffer));
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
        UpdateMenu();
    }

    void CUndo::AddActor(string name, GUID actorId)
    {
        UndoUnit unit = {
            string("Add ").append(name),
            [=]() {
                auto actor = m_scene->GetActor(actorId);
                auto state = actor->Save();
                m_scene->Delete(actor);
                return state;
            },
            [=](Savable savable) {
                m_scene->Restore(savable.object);
                m_scene->SelectActorById(actorId);
            }
        };
        Add(unit);
    }

    void CUndo::DeleteActor(string name, GUID actorId, GUID groupId)
    {
        auto state = m_scene->GetActor(actorId)->Save();
        UndoUnit unit = {
            string("Delete ").append(name),
            [=]() {
                m_scene->Restore(state.object);
                m_scene->SelectActorById(actorId);
                return state;
            },
            [=](Savable savable) {
                m_scene->Delete(m_scene->GetActor(actorId));
            },
            groupId
        };
        Add(unit);
    }

    void CUndo::ChangeActor(string name, GUID actorId, GUID groupId)
    {
        auto state = m_scene->GetActor(actorId)->Save();
        ChangeActor(name, state, actorId, groupId);
    }

    void CUndo::ChangeActor(string name, Savable actorState, GUID actorId, GUID groupId)
    {
        UndoUnit unit = {
            name,
            [=]() {
                auto actor = m_scene->GetActor(actorId);
                auto oldState = actor->Save();

                m_scene->Restore(actorState.object);
                m_scene->SelectActorById(actorId, false);

                // Return state saved before restore so system can "undo" to this point.
                return oldState;
            },
            [=](Savable savable) {
                m_scene->Restore(savable.object);
                m_scene->SelectActorById(actorId, false);
            },
            groupId
        };
        Add(unit);
    }
}
