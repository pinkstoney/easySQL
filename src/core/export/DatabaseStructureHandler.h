#pragma once

#include <map>
#include <string>
#include <vector>

#include "../database/DatabaseManager.h"
#include "../database/TableManager.h"

class DatabaseStructureHandler
{
public:
    struct TableInfo
    {
        std::string createStatement;
        std::vector<std::string> insertStatements;
        bool hasForeignKeys;
    };

public:
    DatabaseStructureHandler(DatabaseManager* dbManager, TableManager* tableManager);

public:
    std::string getCreateStatement(const std::string& tableName);
    std::vector<std::string> getInsertStatements(const std::string& tableName);
    std::vector<std::string> getOrderedTableNames();
    bool hasCircularDependencies(const std::vector<std::string>& tables);

private:
    std::map<std::string, std::vector<std::string>> buildDependencyGraph();
    bool detectCircularDependencies(const std::string& table, std::set<std::string>& visited, std::set<std::string>& recursionStack);
    std::vector<std::string> topologicalSort();

private:
    DatabaseManager* m_dbManager;
    TableManager* m_tableManager;
};
