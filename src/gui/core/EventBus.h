#pragma once

#include "EventType.h"

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class EventBus
{
public:
    using EventCallback = std::function<void(const std::any&)>;
    using SubscriberId = size_t;

    static EventBus& getInstance();

    SubscriberId subscribe(EventType type, EventCallback callback);
    void unsubscribe(EventType type, SubscriberId id);
    void publish(EventType type, const std::any& data);

private:
    EventBus() = default;
    ~EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    struct Subscription
    {
        SubscriberId id;
        EventCallback callback;
    };

    std::unordered_map<EventType, std::vector<Subscription>> m_subscribers;
    std::mutex m_subscribersMutex;
};
