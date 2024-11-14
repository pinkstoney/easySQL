#pragma once

#include <string>
#include <vector>

#include <mysqlx/xdevapi.h>

class TableDataManager
{
public:
    explicit TableDataManager(mysqlx::Session& session);

public:
    std::vector<std::string> getTableInsertStatements(const std::string& tableName) const;
    std::vector<std::string> getTableNames() const;
    std::vector<std::string> getViewNames() const;
    std::vector<std::string> getProcedureNames() const;
    std::vector<std::string> getFunctionNames() const;

private:
    mysqlx::Session& session;
};
