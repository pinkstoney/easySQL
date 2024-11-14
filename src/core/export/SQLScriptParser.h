#pragma once
#include <string>
#include <vector>

class SQLScriptParser
{
public:
    struct SQLStatement
    {
        enum class Type
        {
            SET,
            CREATE_DATABASE,
            USE,
            CREATE_TABLE,
            INSERT,
            OTHER
        };

        Type type;
        std::string content;
        std::string tableName; // For CREATE_TABLE and INSERT statements
    };

    static std::vector<SQLStatement> parseScript(const std::string& sqlContent);

private:
    static SQLStatement::Type determineStatementType(const std::string& stmt);
    static std::string extractTableName(const std::string& stmt);
    static std::vector<std::string> splitStatements(const std::string& sql);
};
