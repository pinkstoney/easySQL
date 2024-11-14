#include "DatabaseStructureHandler.h"

#include <iostream>

DatabaseStructureHandler::DatabaseStructureHandler(DatabaseManager* dbManager, TableManager* tableManager)
    : m_dbManager(dbManager)
    , m_tableManager(tableManager)
{
}

std::string DatabaseStructureHandler::getCreateStatement(const std::string& tableName)
{
    return m_tableManager->getTableCreateStatement(tableName);
}

std::vector<std::string> DatabaseStructureHandler::getInsertStatements(const std::string& tableName)
{
    return m_tableManager->getTableInsertStatements(tableName);
}

std::vector<std::string> DatabaseStructureHandler::getOrderedTableNames()
{
    if (m_dbManager->hasCircularDependencies())
    {
        std::cerr << "Warning: Circular dependencies detected" << std::endl;
        return m_tableManager->getTableNames();
    }
    return m_tableManager->getOrderedTableNames();
}

bool DatabaseStructureHandler::hasCircularDependencies(const std::vector<std::string>& tables)
{
    std::set<std::string> visited;
    std::set<std::string> recursionStack;

    for (const auto& table : tables)
    {
        if (detectCircularDependencies(table, visited, recursionStack))
            return true;
    }

    return false;
}

std::map<std::string, std::vector<std::string>> DatabaseStructureHandler::buildDependencyGraph()
{
    std::map<std::string, std::vector<std::string>> graph;
    auto tables = m_tableManager->getTableNames();

    for (const auto& table : tables)
    {
        auto result = m_dbManager->getSession()
                          .sql("SELECT REFERENCED_TABLE_NAME FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                               "WHERE TABLE_SCHEMA = DATABASE() "
                               "AND TABLE_NAME = '" +
                               table +
                               "' "
                               "AND REFERENCED_TABLE_NAME IS NOT NULL")
                          .execute();

        std::vector<std::string> dependencies;

        for (const auto& row : result.fetchAll())
            dependencies.push_back(row[0].get<std::string>());

        graph[table] = dependencies;
    }

    return graph;
}

bool DatabaseStructureHandler::detectCircularDependencies(const std::string& table, std::set<std::string>& visited,
                                                          std::set<std::string>& recursionStack)
{

    if (recursionStack.find(table) != recursionStack.end())
        return true; 

    if (visited.find(table) != visited.end())
        return false; 

    visited.insert(table);
    recursionStack.insert(table);

    auto graph = buildDependencyGraph();
    for (const auto& dependency : graph[table])
    {
        if (detectCircularDependencies(dependency, visited, recursionStack))
            return true;
    }

    recursionStack.erase(table);
    return false;
}

std::vector<std::string> DatabaseStructureHandler::topologicalSort()
{
    auto graph = buildDependencyGraph();
    std::vector<std::string> result;
    std::set<std::string> visited;

    std::function<void(const std::string&)> visit = [&](const std::string& table) {
        if (visited.find(table) != visited.end())
            return;

        visited.insert(table);

        for (const auto& dependency : graph[table])
            visit(dependency);

        result.push_back(table);
    };

    for (const auto& [table, _] : graph)
        visit(table);

    std::reverse(result.begin(), result.end());
    return result;
}
