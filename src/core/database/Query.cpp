#include "Query.h"
#include "TableManager.h"

#include <iostream>

Query::Query(mysqlx::Session& sess, TableManager& tableMgr)
    : session(sess)
    , tableManager(tableMgr)
{
}

QueryResult Query::execute(const std::string& query)
{
    QueryResult resultData;
    std::cout << "\nExecuting custom query:" << std::endl;

    try
    {
        mysqlx::RowResult result = session.sql(query).execute();
        const mysqlx::Columns& columns = result.getColumns();

        for (const auto& column : columns)
        {
            std::string columnName = column.getColumnName();
            resultData.columns.push_back(columnName);
        }

        for (const auto& row : result)
        {
            std::vector<std::string> currentRow;
            for (size_t i = 0; i < resultData.columns.size(); ++i)
            {
                std::string valueStr = tableManager.valueToString(row[i]);
                currentRow.push_back(valueStr);
            }
            resultData.rows.push_back(currentRow);
        }
    }
    catch (const mysqlx::Error& err)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Query execution failed: ") + e.what());
    }

    return resultData;
}
