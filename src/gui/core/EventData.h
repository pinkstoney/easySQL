#pragma once

#include "../../models/QueryResult.h"

#include <string>
#include <vector>

struct DatabaseConnectedData
{
    std::string databaseName;
    bool success;

    DatabaseConnectedData(std::string name, bool succ)
        : databaseName(std::move(name))
        , success(succ)
    {
    }
};

struct QueryExecutedData
{
    QueryResult result;
    bool success;
    std::string error;

    QueryExecutedData(QueryResult res, bool succ, std::string err = "")
        : result(std::move(res))
        , success(succ)
        , error(std::move(err))
    {
    }
};

struct TablesLoadedData
{
    std::vector<std::string> tables;
    std::vector<std::string> views;
    std::vector<std::string> procedures;
    std::vector<std::string> functions;
    bool success;

    TablesLoadedData(
        std::vector<std::string> tabs,
        std::vector<std::string> vws,
        std::vector<std::string> procs,
        std::vector<std::string> funcs,
        bool succ)
        : tables(std::move(tabs))
        , views(std::move(vws))
        , procedures(std::move(procs))
        , functions(std::move(funcs))
        , success(succ)
    {
    }
};

struct ErrorData
{
    std::string message;
    bool isError;

    ErrorData(std::string msg, bool error)
        : message(std::move(msg))
        , isError(error)
    {
    }
};

struct TableStructureData
{
    std::string tableName;
    QueryResult structure;
    bool success;

    TableStructureData(std::string name, QueryResult res, bool succ)
        : tableName(std::move(name))
        , structure(std::move(res))
        , success(succ)
    {
    }
};
