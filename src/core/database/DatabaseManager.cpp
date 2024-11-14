#include "DatabaseManager.h"

#include <iostream>

DatabaseManager::DatabaseManager(const std::string& host, int port, const std::string& user, const std::string& password)
    : session(mysqlx::SessionSettings(host, port, user, password))
{
}

void DatabaseManager::connectToDatabase(const std::string& dbName)
{
    session.sql("USE " + dbName).execute();
}

std::vector<std::string> DatabaseManager::getDatabases()
{
    std::vector<std::string> databases;
    auto result = session.sql("SHOW DATABASES").execute();

    for (const auto& row : result.fetchAll())
    {
        std::string dbName = row[0].get<std::string>();

        if (dbName != "information_schema" && dbName != "mysql" && dbName != "performance_schema" && dbName != "sys")
            databases.push_back(dbName);
    }

    return databases;
}

std::vector<std::string> DatabaseManager::getViews()
{
    std::vector<std::string> views;
    try
    {
        auto result = session.sql("SHOW FULL TABLES WHERE Table_type = 'VIEW'").execute();

        for (const auto& row : result.fetchAll())
            views.push_back(row[0].get<std::string>());
    }
    catch (const mysqlx::Error& err)
    {
        std::cerr << "Error getting views: " << err.what() << std::endl;
    }
    return views;
}

std::vector<std::string> DatabaseManager::getStoredProcedures()
{
    std::vector<std::string> procedures;
    try
    {
        auto result = session.sql("SHOW PROCEDURE STATUS WHERE Db = DATABASE()").execute();

        for (const auto& row : result.fetchAll())
            procedures.push_back(row[1].get<std::string>());
    }
    catch (const mysqlx::Error& err)
    {
        std::cerr << "Error getting stored procedures: " << err.what() << std::endl;
    }
    return procedures;
}

std::string DatabaseManager::getCurrentDatabase()
{
    auto result = session.sql("SELECT DATABASE()").execute();
    auto row = result.fetchOne();
    return row ? row[0].get<std::string>() : "";
}

std::vector<std::string> DatabaseManager::getTableDependencies(const std::string& tableName)
{
    std::vector<std::string> dependencies;
    try
    {
        auto result = session
                          .sql("SELECT REFERENCED_TABLE_NAME FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                               "WHERE TABLE_SCHEMA = DATABASE() "
                               "AND TABLE_NAME = '" +
                               tableName +
                               "' "
                               "AND REFERENCED_TABLE_NAME IS NOT NULL")
                          .execute();

        for (const auto& row : result.fetchAll())
        {
            dependencies.push_back(row[0].get<std::string>());
        }
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error getting table dependencies: " << e.what() << std::endl;
    }
    return dependencies;
}

bool DatabaseManager::hasCircularDependencies()
{
    try
    {
        auto result = session
                          .sql("WITH RECURSIVE dependency_chain AS ("
                               "  SELECT TABLE_NAME, REFERENCED_TABLE_NAME, 1 as level "
                               "  FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                               "  WHERE TABLE_SCHEMA = DATABASE() "
                               "    AND REFERENCED_TABLE_NAME IS NOT NULL "
                               "  UNION ALL "
                               "  SELECT d.TABLE_NAME, k.REFERENCED_TABLE_NAME, d.level + 1 "
                               "  FROM dependency_chain d "
                               "  JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE k "
                               "    ON k.TABLE_NAME = d.REFERENCED_TABLE_NAME "
                               "  WHERE k.TABLE_SCHEMA = DATABASE() "
                               "    AND k.REFERENCED_TABLE_NAME IS NOT NULL "
                               "    AND k.REFERENCED_TABLE_NAME = d.TABLE_NAME "
                               ") "
                               "SELECT COUNT(*) > 0 as has_circular "
                               "FROM dependency_chain "
                               "WHERE level > (SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES "
                               "              WHERE TABLE_SCHEMA = DATABASE())")
                          .execute();

        auto row = result.fetchOne();
        return row && row[0].get<bool>();
    }
    catch (const mysqlx::Error& e)
    {
        std::cerr << "Error checking circular dependencies: " << e.what() << std::endl;
        return false;
    }
}
