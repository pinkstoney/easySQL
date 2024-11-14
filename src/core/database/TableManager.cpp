#include "TableManager.h"

#include "ValueFormatter.h"

TableManager::TableManager(mysqlx::Session& session, const std::string& dbName)
    : session(session)
    , dbName(dbName)
    , schema(session.getSchema(dbName))
    , structureManager(session)
    , dataManager(session)
{
}

std::vector<std::pair<std::string, std::string>> TableManager::getTableStructure(const std::string& tableName)
{
    return structureManager.getTableStructure(tableName);
}

std::string TableManager::getTableCreateStatement(const std::string& tableName) const
{
    return structureManager.getCreateStatement(tableName);
}

std::vector<std::string> TableManager::getOrderedTableNames() const
{
    return structureManager.getOrderedTableNames();
}

bool TableManager::hasTableDependency(const std::string& table1, const std::string& table2) const
{
    return structureManager.hasTableDependency(table1, table2);
}

std::vector<std::string> TableManager::getTableNames() const
{
    return dataManager.getTableNames();
}

std::vector<std::string> TableManager::getViewNames() const
{
    return dataManager.getViewNames();
}

std::vector<std::string> TableManager::getProcedureNames() const
{
    return dataManager.getProcedureNames();
}

std::vector<std::string> TableManager::getFunctionNames() const
{
    return dataManager.getFunctionNames();
}

std::vector<std::string> TableManager::getTableInsertStatements(const std::string& tableName) const
{
    return dataManager.getTableInsertStatements(tableName);
}

std::string TableManager::valueToString(const mysqlx::Value& value) const
{
    return ValueFormatter::format(value);
}
