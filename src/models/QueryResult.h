#pragma once
#include <string>
#include <vector>

struct QueryResult
{
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;
};
