#pragma once
#include "QueryResult.h"

struct LatestTableResult
{
    std::vector<std::string> tableNames;
    bool isVisible = false;
};

struct LatestQueryResult
{
    QueryResult data;
};
