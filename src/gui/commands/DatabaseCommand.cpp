#include "DatabaseCommand.h"

#include "../GuiManager.h"
#include "../core/EventData.h"

#include <filesystem>
#include <iostream>

ConnectToServerCommand::ConnectToServerCommand(std::unique_ptr<DatabaseManager>& dbManager, const DatabaseConnectionInfo& connInfo,
                                               ConnectionPanel& connectionPanel)
    : m_dbManager(dbManager)
    , m_connInfo(connInfo)
    , m_connectionPanel(connectionPanel)
{
}

void ConnectToServerCommand::execute()
{
    try
    {
        int portNum = std::stoi(m_connInfo.port);
        m_dbManager = std::make_unique<DatabaseManager>(m_connInfo.host, portNum, m_connInfo.user, m_connInfo.password);

        auto databases = m_dbManager->getDatabases();
        m_connectionPanel.setAvailableDatabases(databases);

        publishEvent(EventType::ServerConnected, DatabaseConnectedData{"", true});
    }
    catch (const std::exception& e)
    {
        publishEvent(EventType::ErrorOccurred, ErrorData{e.what(), true});
        m_dbManager.reset();
    }
}

ConnectToDatabaseCommand::ConnectToDatabaseCommand(std::unique_ptr<DatabaseManager>& dbManager,
                                                   std::unique_ptr<TableManager>& tableManager, std::unique_ptr<Query>& query,
                                                   const std::string& dbName)
    : m_dbManager(dbManager)
    , m_tableManager(tableManager)
    , m_query(query)
    , m_dbName(dbName)
{
}

void ConnectToDatabaseCommand::execute()
{
    try
    {
        std::cout << "Executing ConnectToDatabaseCommand for database: " << m_dbName << std::endl;

        if (!m_dbManager)
            throw std::runtime_error("DatabaseManager is not initialized");

        m_dbManager->connectToDatabase(m_dbName);

        std::cout << "Initializing TableManager..." << std::endl;
        m_tableManager = std::make_unique<TableManager>(m_dbManager->getSession(), m_dbName);

        if (!m_tableManager)
            throw std::runtime_error("Failed to initialize TableManager");

        std::cout << "Initializing Query object..." << std::endl;
        m_query = std::make_unique<Query>(m_dbManager->getSession(), *m_tableManager);

        if (!m_query)
            throw std::runtime_error("Failed to initialize Query object");

        auto tables = m_tableManager->getTableNames();
        publishEvent(EventType::TablesLoaded, TablesLoadedData{tables, std::vector<std::string>(), std::vector<std::string>(),
                                                               std::vector<std::string>(), true});

        std::cout << "Database connection and initialization successful" << std::endl;
        publishEvent(EventType::DatabaseConnected, DatabaseConnectedData{m_dbName, true});
    }
    catch (const std::exception& e)
    {
        std::cerr << "Database connection failed: " << e.what() << "\n";
        publishEvent(EventType::ErrorOccurred, ErrorData{e.what(), true});
        m_tableManager.reset();
        m_query.reset();
        publishEvent(EventType::StateReset, std::any());
    }
}

ExecuteQueryCommand::ExecuteQueryCommand(Query* query, const std::string& queryText, QueryResult& resultOutput)
    : m_query(query)
    , m_queryText(queryText)
    , m_resultOutput(resultOutput)
{
}

void ExecuteQueryCommand::execute()
{
    try
    {
        if (!m_query)
            throw std::runtime_error("Query object is not initialized");

        std::cout << "Executing query: " << m_queryText << std::endl;

        if (m_queryText.empty())
        {
            publishEvent(EventType::ErrorOccurred, ErrorData{"Query cannot be empty", false});
            return;
        }

        auto result = m_query->execute(m_queryText);
        m_resultOutput = result;
        publishEvent(EventType::QueryExecuted, QueryExecutedData{result, true, ""});
        std::cout << "Query executed successfully." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = std::string("Query execution failed: ") + e.what();
        publishEvent(EventType::ErrorOccurred, ErrorData{errorMsg, true});
        m_resultOutput = QueryResult();
        std::cerr << errorMsg << std::endl;
    }
}

LoadTablesCommand::LoadTablesCommand(std::unique_ptr<TableManager>& tableManager, std::vector<std::string>& tableNames)
    : m_tableManager(tableManager)
    , m_tableNames(tableNames)
{
}

void LoadTablesCommand::execute()
{
    try
    {
        if (!m_tableManager)
        {
            throw std::runtime_error("TableManager is not initialized");
        }

        std::cout << "Loading database objects..." << std::endl;

        auto tables = m_tableManager->getTableNames();
        auto views = m_tableManager->getViewNames();
        auto procedures = m_tableManager->getProcedureNames();
        auto functions = m_tableManager->getFunctionNames();

        if (tables.empty() && views.empty() && procedures.empty() && functions.empty())
        {
            publishEvent(EventType::ErrorOccurred, ErrorData{"No database objects found", false});
        }
        else
        {
            m_tableNames = tables;
            m_viewNames = views;
            m_procedureNames = procedures;
            m_functionNames = functions;

            publishEvent(EventType::TablesLoaded, TablesLoadedData{tables, views, procedures, functions, true});
        }
    }
    catch (const std::exception& e)
    {
        std::string error = "Failed to load database objects: " + std::string(e.what());
        std::cerr << error << std::endl;
        publishEvent(EventType::ErrorOccurred, ErrorData{error, true});
    }
}

ExportQueryResultCommand::ExportQueryResultCommand(const QueryResult& result)
    : m_result(result)
{
}

void ExportQueryResultCommand::execute()
{
    try
    {
        std::string filename = "exports/query_result_" + std::to_string(std::time(nullptr)) + ".csv";
        std::filesystem::create_directories("exports");
        QueryExporter::exportToCSV(m_result, filename);
        publishEvent(EventType::ExportCompleted, ErrorData{"Saved to " + filename, false});
    }
    catch (const std::exception& e)
    {
        publishEvent(EventType::ExportFailed, ErrorData{"Failed to save CSV: " + std::string(e.what()), true});
    }
}

DisconnectCommand::DisconnectCommand(std::unique_ptr<DatabaseManager>& dbManager, std::unique_ptr<TableManager>& tableManager,
                                     std::unique_ptr<Query>& query, GuiManager& guiManager)
    : m_dbManager(dbManager)
    , m_tableManager(tableManager)
    , m_query(query)
    , m_guiManager(guiManager)
{
}

void DisconnectCommand::execute()
{
    std::cout << "Executing DisconnectCommand..." << std::endl;

    auto& latestTableResult = m_guiManager.getLatestTableResult();
    auto& latestQueryResult = m_guiManager.getLatestQueryResult();

    latestTableResult.tableNames.clear();
    latestTableResult.isVisible = false;

    latestQueryResult.data = QueryResult();

    m_guiManager.getQueryPanel()->setObjectsVisibility(false);
    m_guiManager.getQueryPanel()->setERDiagramVisibility(false); 
    m_guiManager.getERDiagram()->setVisible(false);              
    m_guiManager.getQueryPanel()->clearQuery();

    m_dbManager.reset();
    m_tableManager.reset();
    m_query.reset();

    m_guiManager.setConnected(false);

    publishEvent(EventType::ServerDisconnected, std::any());
    std::cout << "Disconnect complete, all states cleared" << std::endl;
}

ClearStateCommand::ClearStateCommand(LatestTableResult& tableResult, LatestQueryResult& queryResult, QueryPanel& queryPanel)
    : m_tableResult(tableResult)
    , m_queryResult(queryResult)
    , m_queryPanel(queryPanel)
{
}

void ClearStateCommand::execute()
{
    std::cout << "Executing ClearStateCommand..." << std::endl;

    m_tableResult.tableNames.clear();
    m_tableResult.isVisible = false;
    m_queryPanel.setObjectsVisibility(false);
    m_queryResult.data = QueryResult();
    m_queryPanel.clearQuery();

    std::cout << "State cleared" << std::endl;
}
