#ifndef _PUBSUB_H_
#define _PUBSUB_H_

#include <functional>
#include <string>
#include <vector>

using namespace std;

namespace UltraEd
{
    typedef struct {
        string message;
        function<void(void*)> callback;
    } Subscription;

    class PubSub
    {
    private:
        PubSub() {};

    public:
        static void Publish(const string &message, void *data = 0);
        static void Subscribe(Subscription subscription);

    private:
        static vector<Subscription> m_subscriptions;
    };
}

#endif

