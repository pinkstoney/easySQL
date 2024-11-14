#include "QueryExporter.h"

#include <fstream>

void QueryExporter::exportToCSV(const QueryResult& result, const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Could not create file: " + filename);

    for (size_t i = 0; i < result.columns.size(); ++i)
    {
        file << result.columns[i];

        if (i < result.columns.size() - 1)
            file << ",";
    }
    file << "\n";

    for (const auto& row : result.rows)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            std::string value = row[i];

            if (value.find(',') != std::string::npos)
                file << "\"" << value << "\"";
            else
                file << value;

            if (i < row.size() - 1)
                file << ",";
        }
        file << "\n";
    }

    file.close();
}
