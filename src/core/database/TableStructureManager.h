#pragma once

#include <map>
#include <string>
#include <vector>

#include <mysqlx/xdevapi.h>

class TableStructureManager
{
public:
    explicit TableStructureManager(mysqlx::Session& session);

public:
    std::vector<std::pair<std::string, std::string>> getTableStructure(const std::string& tableName);
    std::string getCreateStatement(const std::string& tableName) const;
    std::vector<std::string> getOrderedTableNames() const;
    bool hasTableDependency(const std::string& table1, const std::string& table2) const;

private:
    std::map<std::string, std::vector<std::string>> buildDependencyGraph() const;
    void performTopologicalSort(const std::string& table, std::map<std::string, std::vector<std::string>>& graph,
                                std::set<std::string>& visited, std::set<std::string>& processed,
                                std::vector<std::string>& result) const;

private:
    mysqlx::Session& session;
};
