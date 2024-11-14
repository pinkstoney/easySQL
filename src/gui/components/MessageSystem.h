#pragma once

#include "../core/EventBus.h"

#include <string>
#include <vector>

#include <raylib.h>

class MessageSystem
{
public:
    MessageSystem(int screenHeight) noexcept;
    ~MessageSystem();

    void showMessage(const std::string& text, bool isError, float duration = 3.0f);
    void render(float deltaTime);
    void clear();

private:
    struct Message
    {
        std::string text;
        bool isError;
        float timeRemaining;
        float alpha;
    };

    void setupSubscriptions();

private:
    Message currentMessage;
    Rectangle messageBox;
    std::vector<EventBus::SubscriberId> subscriptionIds;

    static constexpr float FADE_DURATION = 0.5f;
};
