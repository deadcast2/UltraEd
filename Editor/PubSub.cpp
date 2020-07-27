#include "PubSub.h"

namespace UltraEd
{
    void PubSub::Publish(const string &message, void *data)
    {
        for (auto subscription : m_subscriptions)
        {
            if (subscription.message == message && subscription.callback != 0)
            {
                subscription.callback(data);
            }
        }
    }

    void PubSub::Subscribe(Subscription subscription)
    {
        m_subscriptions.push_back(subscription);
    }

    vector<Subscription> PubSub::m_subscriptions = {};
}
