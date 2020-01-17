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
            auto state = get<0>(m_actions[m_position])();
            get<2>(m_actions[m_position]) = state;
        }
    }

    void CAction::Redo()
    {
        if (m_position < m_actions.size())
        {
            auto state = get<2>(m_actions[m_position]);
            get<1>(m_actions[m_position])(state);
            m_position++;
        }
    }

    void CAction::Add(function<Savable()> undo, function<void(Savable)> redo)
    {
        // Clear redo history when adding new action and not at head.
        while (m_actions.size() != m_position)
        {
            m_actions.pop_back();
        }

        Savable emptyState;
        m_actions.push_back({ undo, redo, emptyState });
        m_position = m_actions.size();
    }

    void CAction::AddActor(CScene *scene, shared_ptr<CActor> actor)
    {
        auto state = actor->Save();
        Add([=]() {
            scene->Delete(actor);
            return state;
        }, [=](Savable savable) {
            scene->Restore(state.object);
        });
    }

    void CAction::DeleteActor(CScene *scene, shared_ptr<CActor> actor)
    {
        auto state = actor->Save();
        Add([=]() {
            scene->Restore(state.object);
            return state;
        }, [=](Savable savable) {
            scene->Delete(actor);
        });
    }

    void CAction::ChangeActor(CScene *scene, shared_ptr<CActor> actor)
    {
        auto state = actor->Save();
        Add([=]() {
            auto futureState = actor->Save();
            scene->Restore(state.object);
            return futureState;
        }, [=](Savable savable) {
            scene->Restore(savable.object);
        });
    }
}
