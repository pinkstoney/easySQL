#pragma once

#include "DatabaseCommand.h"

#include <memory>

class CommandFactory
{
public:
    static std::unique_ptr<DatabaseCommand> createConnectCommand(std::unique_ptr<DatabaseManager>& dbManager,
                                                                 const DatabaseConnectionInfo& connInfo,
                                                                 ConnectionPanel& connectionPanel);

    static std::unique_ptr<DatabaseCommand> createDatabaseConnectCommand(std::unique_ptr<DatabaseManager>& dbManager,
                                                                         std::unique_ptr<TableManager>& tableManager,
                                                                         std::unique_ptr<Query>& query, const std::string& dbName);

    static std::unique_ptr<DatabaseCommand> createQueryCommand(Query* query, const std::string& queryText, QueryResult& resultOutput);

    static std::unique_ptr<DatabaseCommand> createExportCommand(const QueryResult& result);
};
