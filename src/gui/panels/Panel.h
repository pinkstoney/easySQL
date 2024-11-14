#pragma once

#include "../core/EventBus.h"

#include <vector>

#include <raylib.h>

class Panel
{
public:
    Panel() = default;
    virtual ~Panel()
    {
        auto& bus = EventBus::getInstance();

        for (const auto& sub : subscriptionIds)
            bus.unsubscribe(sub.first, sub.second);
    }

    virtual void render(bool hasData = false) = 0;

protected:
    Rectangle bounds;
    std::vector<std::pair<EventType, EventBus::SubscriberId>> subscriptionIds;
};
