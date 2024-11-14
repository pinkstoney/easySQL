#pragma once

#include <string>
#include <vector>

#include <mysqlx/xdevapi.h>

class DatabaseManager
{
public:
    DatabaseManager(const std::string& host, int port, const std::string& user, const std::string& password);

public:
    void connectToDatabase(const std::string& dbName);
    std::vector<std::string> getDatabases();
    mysqlx::Session& getSession() { return session; }
    const mysqlx::Session& getSession() const { return session; }
    std::vector<std::string> getViews();
    std::vector<std::string> getStoredProcedures();
    std::string getCurrentDatabase();
    std::vector<std::string> getTableDependencies(const std::string& tableName);

    bool hasCircularDependencies();

private:
    mysqlx::Session session;
};
