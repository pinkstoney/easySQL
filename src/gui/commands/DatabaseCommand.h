#pragma once

#include "../../core/database/DatabaseManager.h"
#include "../../core/database/Query.h"
#include "../../core/database/TableManager.h"

#include "../../core/export/QueryExporter.h"

#include "../../models/DatabaseConnectionInfo.h"
#include "../../models/Results.h"

#include "../core/EventBus.h"
#include "../core/EventType.h"

#include "../panels/ConnectionPanel.h"
#include "../panels/QueryPanel.h"

#include "../states/ApplicationState.h"

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
class GuiManager;
struct Message;
struct QueryResult;
struct LatestTableResult;
struct LatestQueryResult; 

class DatabaseCommand
{
public:
    virtual ~DatabaseCommand() = default;
    virtual void execute() = 0;

protected:
    template <typename T>
    void publishEvent(EventType type, T data)
    {
        EventBus::getInstance().publish(type, std::any(std::move(data)));
    }
};

class ConnectToServerCommand : public DatabaseCommand
{
public:
    ConnectToServerCommand(std::unique_ptr<DatabaseManager>& dbManager, const DatabaseConnectionInfo& connInfo,
                           ConnectionPanel& connectionPanel);
    void execute() override;

private:
    std::unique_ptr<DatabaseManager>& m_dbManager;
    DatabaseConnectionInfo m_connInfo;
    ConnectionPanel& m_connectionPanel;
};

class ConnectToDatabaseCommand : public DatabaseCommand
{
public:
    ConnectToDatabaseCommand(std::unique_ptr<DatabaseManager>& dbManager, std::unique_ptr<TableManager>& tableManager,
                             std::unique_ptr<Query>& query, const std::string& dbName);
    void execute() override;

private:
    std::unique_ptr<DatabaseManager>& m_dbManager;
    std::unique_ptr<TableManager>& m_tableManager;
    std::unique_ptr<Query>& m_query;
    std::string m_dbName;
};

class ExecuteQueryCommand : public DatabaseCommand
{
public:
    ExecuteQueryCommand(Query* query, const std::string& queryText, QueryResult& resultOutput);
    void execute() override;

private:
    Query* m_query;
    std::string m_queryText;
    QueryResult& m_resultOutput;
};

class LoadTablesCommand : public DatabaseCommand
{
public:
    LoadTablesCommand(std::unique_ptr<TableManager>& tableManager, std::vector<std::string>& tableNames);
    void execute() override;

private:
    std::unique_ptr<TableManager>& m_tableManager;
    std::vector<std::string>& m_tableNames;
    std::vector<std::string> m_viewNames;
    std::vector<std::string> m_procedureNames;
    std::vector<std::string> m_functionNames;
};

class ExportQueryResultCommand : public DatabaseCommand
{
public:
    explicit ExportQueryResultCommand(const QueryResult& result);
    void execute() override;

private:
    QueryResult m_result;
};

class DisconnectCommand : public DatabaseCommand
{
public:
    DisconnectCommand(std::unique_ptr<DatabaseManager>& dbManager, std::unique_ptr<TableManager>& tableManager,
                      std::unique_ptr<Query>& query, GuiManager& guiManager);
    void execute() override;

private:
    std::unique_ptr<DatabaseManager>& m_dbManager;
    std::unique_ptr<TableManager>& m_tableManager;
    std::unique_ptr<Query>& m_query;
    GuiManager& m_guiManager;
};

class ClearStateCommand : public DatabaseCommand
{
public:
    ClearStateCommand(LatestTableResult& tableResult, LatestQueryResult& queryResult, QueryPanel& queryPanel);
    void execute() override;

private:
    LatestTableResult& m_tableResult;
    LatestQueryResult& m_queryResult;
    QueryPanel& m_queryPanel;
};
