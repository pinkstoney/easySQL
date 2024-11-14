#pragma once

#include "../../models/QueryResult.h"

#include <memory>

class GuiManager;

class ApplicationState
{
public:
    virtual ~ApplicationState() = default;
    virtual void render(GuiManager& manager) = 0;
    virtual void handleInput(GuiManager& manager) = 0;
};

class DisconnectedState : public ApplicationState
{
public:
    void render(GuiManager& manager) override;
    void handleInput(GuiManager& manager) override;
};

class ConnectedState : public ApplicationState
{
public:
    void render(GuiManager& manager) override;
    void handleInput(GuiManager& manager) override;
};

class QueryState : public ApplicationState
{
public:
    void render(GuiManager& manager) override;
    void handleInput(GuiManager& manager) override;
};
