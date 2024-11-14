#include "EventBus.h"

#include <iostream>

EventBus& EventBus::getInstance()
{
    static EventBus instance;
    return instance;
}

EventBus::SubscriberId EventBus::subscribe(EventType type, EventCallback callback)
{
    std::lock_guard<std::mutex> lock(m_subscribersMutex);
    static SubscriberId nextId = 0;
    SubscriberId id = nextId++;
    m_subscribers[type].push_back({id, std::move(callback)});
    return id;
}

void EventBus::unsubscribe(EventType type, SubscriberId id)
{
    std::lock_guard<std::mutex> lock(m_subscribersMutex);
    if (auto it = m_subscribers.find(type); it != m_subscribers.end())
    {
        auto& subs = it->second;
        subs.erase(std::remove_if(subs.begin(), subs.end(),
                                  [id](const auto& sub) {
                                      return sub.id == id;
                                  }),
                   subs.end());
    }
}

void EventBus::publish(EventType type, const std::any& data)
{
    std::lock_guard<std::mutex> lock(m_subscribersMutex);
    if (auto it = m_subscribers.find(type); it != m_subscribers.end())
    {
        auto subs = it->second;
        for (const auto& sub : subs)
        {
            try
            {
                sub.callback(data);
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error in event callback: " << e.what() << std::endl;
            }
        }
    }
}
