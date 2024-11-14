#include "GuiManager.h"

#include "states/ApplicationState.h"
#include "commands/CommandFactory.h"

#include <ctime>
#include <filesystem>
#include <iostream>
#include <typeinfo>

#include <raylib.h>

#include "../include/raygui.h"


GuiManager::GuiManager(int screenWidth, int screenHeight)
    : screenWidth(screenWidth)
    , screenHeight(screenHeight)
{
    connectionPanel = std::make_shared<ConnectionPanel>(20, 20, screenHeight, *this);
    queryPanel =
        std::make_shared<QueryPanel>(connectionPanel->getWidth() + 40, 20, screenWidth - (connectionPanel->getWidth() + 60), *this);

    connectionPanel->setQueryPanel(queryPanel);

    resultsPanel = std::make_shared<ResultsPanel>(queryPanel->getStartX(), queryPanel->getHeight() + 40,
                                                  screenWidth - queryPanel->getStartX() - 20,
                                                  screenHeight - queryPanel->getHeight() - 80, screenHeight);
    resultsPanel->setQueryPanel(queryPanel);
    messageSystem = std::make_shared<MessageSystem>(screenHeight);

    erDiagram = std::make_shared<ERDiagram>(queryPanel->getStartX(), queryPanel->getHeight() + 40,
                                            screenWidth - queryPanel->getStartX() - 20, screenHeight - queryPanel->getHeight() - 80);

    resultsPanel->setTableClickCallback([this](const std::string& tableName) {
        handleTableClick(tableName);
    });

    resultsPanel->setTableStructureCallback([this](const std::string& tableName) {
        handleTableStructure(tableName);
    });

    m_state = std::make_unique<DisconnectedState>();
}

GuiManager::~GuiManager() {}

void GuiManager::initialize()
{
    InitWindow(screenWidth, screenHeight, "BodyaSQL");
    SetTargetFPS(60);
}

void GuiManager::handleTableClick(const std::string& name)
{
    if (!getConnected() || !m_query)
        return;

    std::cout << "Object clicked: " << name << std::endl;

    latestTableResult.isVisible = true;
    queryPanel->setObjectsVisibility(true);

    queryPanel->clearQuery();
    std::string query;

    auto objectType = resultsPanel->getCurrentObjectType();

    switch (objectType)
    {
    case ScrollableList::DatabaseObjectType::Table:
        query = "SELECT * FROM " + name;
        break;

    case ScrollableList::DatabaseObjectType::View:
        query = "SELECT * FROM " + name;
        break;

    case ScrollableList::DatabaseObjectType::StoredProcedure:
        query = "SELECT ROUTINE_DEFINITION as '" + name +
                "' "
                "FROM INFORMATION_SCHEMA.ROUTINES "
                "WHERE ROUTINE_SCHEMA = DATABASE() AND ROUTINE_TYPE = 'PROCEDURE' AND ROUTINE_NAME = '" +
                name + "'";
        break;

    case ScrollableList::DatabaseObjectType::Function:
        query = "SELECT ROUTINE_DEFINITION as '" + name +
                "' "
                "FROM INFORMATION_SCHEMA.ROUTINES "
                "WHERE ROUTINE_SCHEMA = DATABASE() AND ROUTINE_TYPE = 'FUNCTION' AND ROUTINE_NAME = '" +
                name + "'";
        break;
    }

    strcpy(queryPanel->getQueryInput(), query.c_str());
    queryPanel->simulateExecuteQuery();
}

void GuiManager::handleTableStructure(const std::string& tableName)
{
    if (getConnected() && m_query)
    {
        try
        {
            std::string query = "DESCRIBE " + tableName;
            auto command = CommandFactory::createQueryCommand(m_query.get(), query, latestResult.data);
            addCommand(std::move(command));
            executePendingCommands();
        }
        catch (const std::exception& e)
        {
            publishEvent(EventType::ErrorOccurred, ErrorData{std::string("Failed to get table structure: ") + e.what(), true});
        }
    }
}

void GuiManager::renderBackground() const
{
    ClearBackground(Color{245, 245, 245, 255});

    DrawRectangleRec(Rectangle{20, 10, (float)connectionPanel->getWidth(), (float)screenHeight - 20}, RAYWHITE);

    DrawRectangleRec(Rectangle{connectionPanel->getWidth() + 40, 10, (float)screenWidth - (connectionPanel->getWidth() + 60), 60},
                     RAYWHITE);

    DrawRectangleRec(Rectangle{connectionPanel->getWidth() + 40, 90, (float)screenWidth - (connectionPanel->getWidth() + 60),
                               (float)screenHeight - 100},
                     RAYWHITE);
}

void GuiManager::handleExportOperations(const LatestQueryResult& latestResult)
{
    if (!getConnected())
        return;

    bool hasData = !latestResult.data.columns.empty();
    if (hasData && queryPanel->shouldExportCSV())
    {
        try
        {
            std::string filename = "exports/query_result_" + std::to_string(std::time(nullptr)) + ".csv";
            std::filesystem::create_directories("exports");
            QueryExporter::exportToCSV(latestResult.data, filename);
            messageSystem->showMessage("Saved to " + filename, false);
        }
        catch (const std::exception& e)
        {
            messageSystem->showMessage("Failed to save CSV: " + std::string(e.what()), true);
        }
    }
}

void GuiManager::renderPanels(const LatestQueryResult& latestResult, const LatestTableResult& latestTableResult)
{
    bool hasData = !latestResult.data.columns.empty();

    connectionPanel->render();

    if (connectionPanel->getState() == ConnectionPanel::ConnectionState::DATABASE_CONNECTED)
    {
        queryPanel->render(hasData);
        resultsPanel->renderQueryResults(latestResult.data);

        if (latestTableResult.isVisible)
        {
            std::cout << "Rendering table list with " << latestTableResult.tableNames.size() << " tables" << std::endl;
            resultsPanel->renderTableList(latestTableResult.tableNames);
        }
    }
}

void GuiManager::handleStateTransitions()
{
    auto currentConnectionState = connectionPanel->getState();

    if (currentConnectionState == ConnectionPanel::ConnectionState::DATABASE_CONNECTED)
        transitionToQueryState();
    else if (currentConnectionState == ConnectionPanel::ConnectionState::SERVER_CONNECTED)
        transitionToConnectedState();
    else if (currentConnectionState == ConnectionPanel::ConnectionState::DISCONNECTED)
        transitionToDisconnectedState();
}

void GuiManager::transitionToQueryState()
{
    if (!dynamic_cast<QueryState*>(m_state.get()))
    {
        std::cout << "Transitioning to QueryState from " << typeid(*m_state).name() << "\n";
        if (verifyDatabaseConnection())
        {
            setState(std::make_unique<QueryState>());
        }
        else
        {
            std::cout << "Failed to transition to QueryState - database objects not initialized\n";
            connectionPanel->setState(ConnectionPanel::ConnectionState::SERVER_CONNECTED);
        }
    }
}

void GuiManager::transitionToConnectedState()
{
    if (!dynamic_cast<ConnectedState*>(m_state.get()))
    {
        std::cout << "Transitioning to ConnectedState\n";
        setState(std::make_unique<ConnectedState>());
    }
}

void GuiManager::transitionToDisconnectedState()
{
    if (!dynamic_cast<DisconnectedState*>(m_state.get()))
    {
        std::cout << "Transitioning to DisconnectedState\n";
        setState(std::make_unique<DisconnectedState>());
        resetDatabaseManager();
        resetTableManager();
        resetQuery();
    }
}

void GuiManager::updateResults(LatestQueryResult& latestResult, LatestTableResult& latestTableResult)
{
    latestResult = this->latestResult;
    latestTableResult = this->latestTableResult;
}

void GuiManager::handleExitConditions(bool& shouldClose)
{
    if (connectionPanel->handleExit() || WindowShouldClose())
        shouldClose = true;
}

void GuiManager::render(bool& shouldClose, LatestQueryResult& latestResult, LatestTableResult& latestTableResult, Message& errorMsg)
{
    logState();

    this->latestResult = latestResult;
    this->latestTableResult = latestTableResult;

    BeginDrawing();
    {
        renderBackground();

        handleStateTransitions();

        m_state->render(*this);
        m_state->handleInput(*this);

        executePendingCommands();

        renderPanels(this->latestResult, this->latestTableResult);

        if (queryPanel->getERDiagramVisibility())
        {
            std::cout << "ER Diagram is visible, rendering..." << std::endl;
            erDiagram->update();
            erDiagram->render();
        }

        handleExitConditions(shouldClose);

        messageSystem->render(GetFrameTime());
    }
    EndDrawing();

    updateResults(latestResult, latestTableResult);
}

void GuiManager::logState() const
{
    std::cout << "Current state: " << (m_state ? typeid(*m_state).name() : "null") << "\n"
              << "DatabaseManager: " << (m_dbManager ? "initialized" : "null") << "\n"
              << "TableManager: " << (m_tableManager ? "initialized" : "null") << "\n"
              << "Query: " << (m_query ? "initialized" : "null") << "\n";
}
