#include "Action.h"
#include "Scene.h"

namespace UltraEd
{
    CAction::CAction() :
        m_actions(),
        m_position(0)
    { }

    void CAction::Undo()
    {
        if (m_position > 0)
        {
            m_position--;
            auto state = m_actions[m_position].undo();
            m_actions[m_position].state = state;
            CDebug::Log("Undo - %s\n", m_actions[m_position].name.c_str());
        }
    }

    void CAction::Redo()
    {
        if (m_position < m_actions.size())
        {
            auto state = m_actions[m_position].state;
            m_actions[m_position].redo(state);
            CDebug::Log("Redo - %s\n", m_actions[m_position].name.c_str());
            m_position++;
        }
    }

    void CAction::Reset()
    {
        m_actions.clear();
        m_position = 0;
    }

    void CAction::Add(Action action)
    {
        // Clear redo history when adding new action and not at head.
        while (m_actions.size() != m_position)
        {
            m_actions.pop_back();
        }

        m_actions.push_back(action);
        m_position = m_actions.size();
    }

    void CAction::AddActor(string name, CScene *scene, shared_ptr<CActor> actor)
    {
        auto state = actor->Save();
        Action action = {
            string("Add ").append(name),
            [=]() {
                scene->Delete(actor);
                return state;
            },
            [=](Savable savable) {
                scene->Restore(state.object);
            }
        };
        Add(action);
    }

    void CAction::DeleteActor(string name, CScene *scene, shared_ptr<CActor> actor)
    {
        auto state = actor->Save();
        Action action = {
            string("Delete ").append(name),
            [=]() {
                scene->Restore(state.object);
                return state;
            },
            [=](Savable savable) {
                scene->Delete(actor);
            }
        };
        Add(action);
    }

    void CAction::ChangeActor(string name, CScene *scene, GUID actorId)
    {
        auto state = scene->GetActor(actorId)->Save();
        Action action = {
            name,
            [=]() {
                auto actor = scene->GetActor(actorId);
                auto futureState = actor->Save();
                scene->Restore(state.object);
                return futureState;
            },
            [=](Savable savable) {
                auto actor = scene->GetActor(actorId);
                scene->Restore(savable.object);
            }
        };
        Add(action);
    }

    void CAction::SelectActor(string name, CScene *scene, GUID actorId)
    {
        auto state = scene->GetActor(actorId)->Save();
        Action action = {
            name,
            [=]() {
                scene->SelectActorById(actorId, false, false);
                return state;
            },
            [=](Savable savable) {
                scene->SelectActorById(actorId, false, false);
            }
        };
        Add(action);
    }
}
