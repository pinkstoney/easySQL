#include "CommandFactory.h"

#include "DatabaseCommand.h"

std::unique_ptr<DatabaseCommand> CommandFactory::createConnectCommand(std::unique_ptr<DatabaseManager>& dbManager,
                                                                      const DatabaseConnectionInfo& connInfo,
                                                                      ConnectionPanel& connectionPanel)
{
    return std::make_unique<ConnectToServerCommand>(dbManager, connInfo, connectionPanel);
}

std::unique_ptr<DatabaseCommand> CommandFactory::createDatabaseConnectCommand(std::unique_ptr<DatabaseManager>& dbManager,
                                                                              std::unique_ptr<TableManager>& tableManager,
                                                                              std::unique_ptr<Query>& query, const std::string& dbName)
{
    return std::make_unique<ConnectToDatabaseCommand>(dbManager, tableManager, query, dbName);
}

std::unique_ptr<DatabaseCommand> CommandFactory::createQueryCommand(Query* query, const std::string& queryText,
                                                                    QueryResult& resultOutput)
{
    return std::make_unique<ExecuteQueryCommand>(query, queryText, resultOutput);
}

std::unique_ptr<DatabaseCommand> CommandFactory::createExportCommand(const QueryResult& result)
{
    return std::make_unique<ExportQueryResultCommand>(result);
}
