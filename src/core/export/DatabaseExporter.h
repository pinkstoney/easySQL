#pragma once

#include "DatabaseStructureHandler.h"
#include "SQLScriptParser.h"

#include <string>

class DatabaseExporter
{
public:
    static bool exportToSQL(DatabaseManager* dbManager, TableManager* tableManager, const std::string& filename);
    static bool importFromSQL(DatabaseManager* dbManager, const std::string& filename);

private:
    static bool writeToFile(const std::string& filename, const std::string& dbName, DatabaseStructureHandler& structureHandler);

    static bool executeStatements(DatabaseManager* dbManager, const std::vector<SQLScriptParser::SQLStatement>& statements);

    static std::string generateHeader(const std::string& dbName);
};
