#include "PubSub.h"

namespace UltraEd
{
    void PubSub::Publish(const std::string &message, void *data)
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

    std::vector<Subscription> PubSub::m_subscriptions = {};
}
