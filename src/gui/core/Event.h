#pragma once

#include "EventType.h"

#include <any>

template <typename T>
struct Event
{
    EventType type;
    T data;

    Event(EventType t, T d)
        : type(t)
        , data(std::move(d))
    {
    }
};

using GenericEvent = Event<std::any>;
