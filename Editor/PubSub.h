#ifndef _PUBSUB_H_
#define _PUBSUB_H_

#include <functional>
#include <string>
#include <vector>

namespace UltraEd
{
    typedef struct {
        std::string message;
        std::function<void(void*)> callback;
    } Subscription;

    class PubSub
    {
    private:
        PubSub() {};

    public:
        static void Publish(const std::string &message, void *data = 0);
        static void Subscribe(Subscription subscription);

    private:
        static std::vector<Subscription> m_subscriptions;
    };
}

#endif

