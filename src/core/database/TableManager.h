#pragma once

#include "TableDataManager.h"
#include "TableStructureManager.h"

#include <string>

class TableManager
{
public:
    TableManager(mysqlx::Session& session, const std::string& dbName);

public:
    std::vector<std::pair<std::string, std::string>> getTableStructure(const std::string& tableName);
    std::string getTableCreateStatement(const std::string& tableName) const;
    std::vector<std::string> getOrderedTableNames() const;
    bool hasTableDependency(const std::string& table1, const std::string& table2) const;

public:
    std::vector<std::string> getTableNames() const;
    std::vector<std::string> getViewNames() const;
    std::vector<std::string> getProcedureNames() const;
    std::vector<std::string> getFunctionNames() const;
    std::vector<std::string> getTableInsertStatements(const std::string& tableName) const;

public:
    std::string valueToString(const mysqlx::Value& value) const;

private:
    mysqlx::Session& session;
    std::string dbName;
    mysqlx::Schema schema;

private:
    TableStructureManager structureManager;
    TableDataManager dataManager;
};
