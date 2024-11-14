#pragma once

#include "models/QueryResult.h"

#include <string>
#include <vector>

#include <mysqlx/xdevapi.h>

class TableManager;

class Query
{
public:
    Query(mysqlx::Session& sess, TableManager& tableMgr);
    QueryResult execute(const std::string& query);

private:
    mysqlx::Session& session;
    TableManager& tableManager;
};
