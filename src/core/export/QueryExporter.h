#pragma once

#include "models/QueryResult.h"

#include <string>

class QueryExporter
{
public:
    static void exportToCSV(const QueryResult& result, const std::string& filename);
};
