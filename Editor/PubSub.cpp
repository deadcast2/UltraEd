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

    /// <summary>
    /// Returns back a function that can be used to unsubscribe.
    /// </summary>
    /// <param name="subscription"></param>
    /// <returns></returns>
    std::function<void()> PubSub::Subscribe(Subscription subscription)
    {
        m_subscriptions.push_back(subscription);
        return [subscription]() {
            m_subscriptions.erase(std::find(m_subscriptions.begin(), m_subscriptions.end(), subscription));
        };
    }

    std::vector<Subscription> PubSub::m_subscriptions = {};
}
