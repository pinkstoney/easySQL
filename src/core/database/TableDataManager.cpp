#include "TableDataManager.h"

#include "ValueFormatter.h"

#include <iostream>
#include <sstream>

TableDataManager::TableDataManager(mysqlx::Session& session)
    : session(session)
{
}

std::vector<std::string> TableDataManager::getTableNames() const
{
    std::vector<std::string> tables;
    try
    {
        auto result = session.sql("SHOW FULL TABLES WHERE Table_type = 'BASE TABLE'").execute();

        for (const auto& row : result.fetchAll())
            tables.push_back(row[0].get<std::string>());
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting table names: " << e.what() << std::endl;
        throw;
    }
    return tables;
}

std::vector<std::string> TableDataManager::getViewNames() const
{
    std::vector<std::string> views;
    try
    {
        auto result = session.sql("SHOW FULL TABLES WHERE Table_type = 'VIEW'").execute();

        for (const auto& row : result.fetchAll())
            views.push_back(row[0].get<std::string>());
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting view names: " << e.what() << std::endl;
        throw;
    }
    return views;
}

std::vector<std::string> TableDataManager::getProcedureNames() const
{
    std::vector<std::string> procedures;
    try
    {
        auto result = session.sql("SHOW PROCEDURE STATUS WHERE Db = DATABASE()").execute();

        for (const auto& row : result.fetchAll())
            procedures.push_back(row[1].get<std::string>());
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting procedure names: " << e.what() << std::endl;
        throw;
    }
    return procedures;
}

std::vector<std::string> TableDataManager::getFunctionNames() const
{
    std::vector<std::string> functions;
    try
    {
        auto result = session.sql("SHOW FUNCTION STATUS WHERE Db = DATABASE()").execute();

        for (const auto& row : result.fetchAll())
            functions.push_back(row[1].get<std::string>());
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting function names: " << e.what() << std::endl;
        throw;
    }
    return functions;
}

std::vector<std::string> TableDataManager::getTableInsertStatements(const std::string& tableName) const
{
    std::vector<std::string> statements;
    try
    {
        auto result = session.sql("SELECT * FROM " + tableName).execute();
        auto rows = result.fetchAll();

        for (const auto& row : rows)
        {
            std::stringstream ss;
            ss << "INSERT INTO " << tableName << " VALUES (";

            for (size_t i = 0; i < row.colCount(); ++i)
            {
                if (i > 0)
                    ss << ", ";

                if (row[i].isNull())
                    ss << "NULL";
                else
                    ss << "'" << ValueFormatter::format(row[i]) << "'";
            }

            ss << ")";
            statements.push_back(ss.str());
        }
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error generating INSERT statements for " << tableName << ": " << e.what() << std::endl;
        throw;
    }
    return statements;
}
