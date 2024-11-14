#include "TableStructureManager.h"

#include <iostream>

TableStructureManager::TableStructureManager(mysqlx::Session& session)
    : session(session)
{
}

std::vector<std::pair<std::string, std::string>> TableStructureManager::getTableStructure(const std::string& tableName)
{
    std::vector<std::pair<std::string, std::string>> structure;
    try
    {
        auto result = session.sql("DESCRIBE " + tableName).execute();
        for (auto row : result)
        {
            std::string columnName = row[0].get<std::string>();
            std::string columnType = row[1].get<std::string>();
            structure.emplace_back(columnName, columnType);
        }
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting table structure: " << e.what() << std::endl;
        throw;
    }
    return structure;
}

std::string TableStructureManager::getCreateStatement(const std::string& tableName) const
{
    try
    {
        auto result = session.sql("SHOW CREATE TABLE " + tableName).execute();
        auto row = result.fetchOne();
        return row[1].get<std::string>();
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting CREATE statement: " << e.what() << std::endl;
        throw;
    }
}

std::vector<std::string> TableStructureManager::getOrderedTableNames() const
{
    std::vector<std::string> orderedTables;
    auto graph = buildDependencyGraph();
    std::set<std::string> visited;
    std::set<std::string> processed;

    for (const auto& [table, _] : graph)
    {
        if (processed.find(table) == processed.end())
            performTopologicalSort(table, graph, visited, processed, orderedTables);
    }

    return orderedTables;
}

std::map<std::string, std::vector<std::string>> TableStructureManager::buildDependencyGraph() const
{
    std::map<std::string, std::vector<std::string>> graph;

    try
    {
        auto tablesResult = session.sql("SHOW FULL TABLES WHERE Table_type = 'BASE TABLE'").execute();
        for (const auto& row : tablesResult.fetchAll())
        {
            std::string tableName = row[0].get<std::string>();

            auto depsResult = session
                                  .sql("SELECT REFERENCED_TABLE_NAME FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                                       "WHERE TABLE_SCHEMA = DATABASE() "
                                       "AND TABLE_NAME = '" +
                                       tableName +
                                       "' "
                                       "AND REFERENCED_TABLE_NAME IS NOT NULL")
                                  .execute();

            std::vector<std::string> deps;

            for (const auto& depRow : depsResult.fetchAll())
                deps.push_back(depRow[0].get<std::string>());

            graph[tableName] = deps;
        }
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error building dependency graph: " << e.what() << std::endl;
        throw;
    }

    return graph;
}

void TableStructureManager::performTopologicalSort(const std::string& table, std::map<std::string, std::vector<std::string>>& graph,
                                                   std::set<std::string>& visited, std::set<std::string>& processed,
                                                   std::vector<std::string>& result) const
{

    if (processed.find(table) != processed.end())
        return;

    visited.insert(table);

    for (const auto& dep : graph[table])
    {
        if (visited.find(dep) != visited.end() && processed.find(dep) == processed.end())
            continue;

        performTopologicalSort(dep, graph, visited, processed, result);
    }

    visited.erase(table);
    processed.insert(table);
    result.push_back(table);
}

bool TableStructureManager::hasTableDependency(const std::string& table1, const std::string& table2) const
{
    try
    {
        auto result = session
                          .sql("SELECT COUNT(*) > 0 FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                               "WHERE TABLE_SCHEMA = DATABASE() "
                               "AND TABLE_NAME = '" +
                               table1 +
                               "' "
                               "AND REFERENCED_TABLE_NAME = '" +
                               table2 + "'")
                          .execute();

        auto row = result.fetchOne();
        return row && row[0].get<bool>();
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error checking table dependency: " << e.what() << std::endl;
        return false;
    }
}
