#pragma once

#include "../core/database/DatabaseManager.h"
#include "../core/database/Query.h"
#include "../core/database/TableManager.h"

#include "../models/Message.h"
#include "../models/Results.h"

#include "commands/CommandExecutor.h"
#include "commands/DatabaseCommand.h"

#include "components/ERDiagram.h"
#include "components/MessageSystem.h"

#include "panels/ConnectionPanel.h"
#include "panels/QueryPanel.h"
#include "panels/ResultsPanel.h"

#include "states/ApplicationState.h"

#include <memory>
#include <vector>
#include <string>

class DatabaseCommand;
class ConnectionPanel;
class QueryPanel;
class DatabaseManager;
class TableManager;
class Query;
class ApplicationState;
class QueryExporter;
struct Message;

struct QueryResult;
struct LatestTableResult;
struct LatestQueryResult; 

class GuiManager
{
public:
    GuiManager(int screenWidth, int screenHeight);
    ~GuiManager();

public:
    void initialize();
    void render(bool& shouldClose, LatestQueryResult& latestResult, LatestTableResult& latestTableResult, Message& errorMsg);

public:
    std::shared_ptr<ConnectionPanel> getConnectionPanel() const { return connectionPanel; }
    std::shared_ptr<QueryPanel> getQueryPanel() const { return queryPanel; }
    std::shared_ptr<ResultsPanel> getResultsPanel() const { return resultsPanel; }
    std::shared_ptr<MessageSystem> getMessageSystem() const { return messageSystem; }
    std::shared_ptr<ERDiagram> getERDiagram() const { return erDiagram; }

public:
    void setState(std::unique_ptr<ApplicationState> newState) { m_state = std::move(newState); }
    bool getConnected() const { return m_connected; }
    void setConnected(bool value) { m_connected = value; }

    void addCommand(std::unique_ptr<DatabaseCommand> command) { m_pendingCommands.push_back(std::move(command)); }

    std::unique_ptr<DatabaseManager>& getDatabaseManager() { return m_dbManager; }
    std::unique_ptr<TableManager>& getTableManager() { return m_tableManager; }
    std::unique_ptr<Query>& getQuery() { return m_query; }
    LatestTableResult& getLatestTableResult() { return latestTableResult; }
    LatestQueryResult& getLatestQueryResult() { return latestResult; }

public:
    void resetDatabaseManager() { m_dbManager.reset(); }
    void resetTableManager() { m_tableManager.reset(); }
    void resetQuery() { m_query.reset(); }


    template <typename T>
    void publishEvent(EventType type, T data)
    {
        EventBus::getInstance().publish(type, std::any(std::move(data)));
    }

public:
    void handleQueryOperations(DatabaseManager* dbManager, Query* query, LatestQueryResult& latestResult,
                               LatestTableResult& latestTableResult, TableManager* tableManager);

    bool verifyDatabaseConnection() const { return m_dbManager && m_tableManager && m_query; }

    void logState() const;

    void executePendingCommands() { CommandExecutor::executeCommands(m_pendingCommands);
    }

private:
    std::vector<std::unique_ptr<DatabaseCommand>> m_pendingCommands;

    void handleTableClick(const std::string& tableName);
    void handleConnection(DatabaseManager* dbManager, TableManager* tableManager, Query* query, LatestTableResult& latestTableResult,
                          LatestQueryResult& latestResult);
    void handleQueryOperations(std::unique_ptr<DatabaseManager>& dbManager, std::unique_ptr<Query>& query,
                               LatestQueryResult& latestResult, LatestTableResult& latestTableResult,
                               std::unique_ptr<TableManager>& tableManager);
    void handleExportOperations(const LatestQueryResult& latestResult);
    void handleStateTransitions();
    void handleTableStructure(const std::string& tableName);
    void handleExitConditions(bool& shouldClose);

private:
    void renderBackground() const;
    void renderPanels(const LatestQueryResult& latestResult, const LatestTableResult& latestTableResult);

private:
    void transitionToQueryState();
    void transitionToConnectedState();
    void transitionToDisconnectedState();
    void updateResults(LatestQueryResult& latestResult, LatestTableResult& latestTableResult);


private:
    std::shared_ptr<ConnectionPanel> connectionPanel;
    std::shared_ptr<QueryPanel> queryPanel;
    std::shared_ptr<ResultsPanel> resultsPanel;
    std::shared_ptr<MessageSystem> messageSystem;
    std::shared_ptr<ERDiagram> erDiagram;

private:
    int screenWidth;
    int screenHeight;
    bool m_connected = false;

private:
    LatestTableResult latestTableResult;
    LatestQueryResult latestResult;

    std::unique_ptr<ApplicationState> m_state;

    std::unique_ptr<DatabaseManager> m_dbManager;
    std::unique_ptr<TableManager> m_tableManager;
    std::unique_ptr<Query> m_query;
};
