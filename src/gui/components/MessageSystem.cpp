#include "MessageSystem.h"

#include "../core/EventData.h"
#include "../core/EventType.h"

#include <memory>

MessageSystem::MessageSystem(int screenHeight) noexcept
{
    float messageBoxHeight = 40;
    float bottomMargin = 30; 
    float messageBoxWidth = 940;
    float messageBoxX = 330;

    messageBox = {messageBoxX,
                  screenHeight - bottomMargin - messageBoxHeight, 
                  messageBoxWidth, messageBoxHeight};

    currentMessage = {"", false, 0.0f, 0.0f};
    setupSubscriptions();
}

MessageSystem::~MessageSystem()
{
    auto& bus = EventBus::getInstance();

    for (auto id : subscriptionIds)
        bus.unsubscribe(EventType::ErrorOccurred, id);
}

void MessageSystem::setupSubscriptions()
{
    auto& bus = EventBus::getInstance();

    subscriptionIds.push_back(bus.subscribe(EventType::ErrorOccurred, [this](const auto& data) {
        auto* errorData = std::any_cast<ErrorData>(&data);

        if (errorData)
            showMessage(errorData->message, true);
    }));

    subscriptionIds.push_back(bus.subscribe(EventType::ServerConnected, [this](const auto&) {
        showMessage("Connected to server", false);
    }));

    subscriptionIds.push_back(bus.subscribe(EventType::DatabaseConnected, [this](const auto& data) {
        auto* connData = std::any_cast<DatabaseConnectedData>(&data);

        if (connData)
            showMessage("Connected to database: " + connData->databaseName, false);
    }));

    subscriptionIds.push_back(bus.subscribe(EventType::QueryExecuted, [this](const auto& data) {
        auto* queryData = std::any_cast<QueryExecutedData>(&data);
        if (queryData)
        {
            if (queryData->success)
                showMessage("Query executed successfully", false);
            else
                showMessage(queryData->error, true);
        }
    }));

    subscriptionIds.push_back(bus.subscribe(EventType::ExportCompleted, [this](const auto& data) {
        auto* exportData = std::any_cast<ErrorData>(&data);

        if (exportData)
            showMessage(exportData->message, exportData->isError);
    }));

    subscriptionIds.push_back(bus.subscribe(EventType::ImportCompleted, [this](const auto& data) {
        auto* importData = std::any_cast<ErrorData>(&data);

        if (importData)
            showMessage(importData->message, importData->isError);
    }));
}

void MessageSystem::showMessage(const std::string& text, bool isError, float duration)
{
    currentMessage.text = text;
    currentMessage.isError = isError;
    currentMessage.timeRemaining = duration;
    currentMessage.alpha = 1.0f;
}

void MessageSystem::render(float deltaTime)
{
    if (currentMessage.timeRemaining > 0.0f)
    {
        if (currentMessage.timeRemaining < FADE_DURATION)
            currentMessage.alpha = currentMessage.timeRemaining / FADE_DURATION;

        Color bgColor = currentMessage.isError ? Color{255, 0, 0, static_cast<unsigned char>(128 * currentMessage.alpha)}
                                               : Color{0, 180, 0, static_cast<unsigned char>(128 * currentMessage.alpha)};

        Color textColor = {255, 255, 255, static_cast<unsigned char>(255 * currentMessage.alpha)};
        Color borderColor = currentMessage.isError ? Color{0, 0, 0, static_cast<unsigned char>(255 * currentMessage.alpha)}
                                                   : Color{0, 100, 0, static_cast<unsigned char>(255 * currentMessage.alpha)};

        DrawRectangleRounded(messageBox, 0.2f, 8, bgColor);
        DrawRectangleRoundedLines(messageBox, 0.2f, 8, 1.0f, borderColor);

        int textWidth = MeasureText(currentMessage.text.c_str(), 18);
        float textX = messageBox.x + (messageBox.width - textWidth) / 2;
        float textY = messageBox.y + (messageBox.height - 18) / 2;

        DrawText(currentMessage.text.c_str(), textX, textY, 18, textColor);

        currentMessage.timeRemaining -= deltaTime;
    }
}

void MessageSystem::clear()
{
    currentMessage.timeRemaining = 0.0f;
    currentMessage.text = "";
    currentMessage.alpha = 0.0f;
}
