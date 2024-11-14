#include "SQLScriptParser.h"

#include <algorithm>

std::vector<SQLScriptParser::SQLStatement> SQLScriptParser::parseScript(const std::string& sqlContent)
{
    std::vector<SQLStatement> statements;
    auto rawStatements = splitStatements(sqlContent);

    for (const auto& stmt : rawStatements)
    {
        if (stmt.empty() || stmt.find_first_not_of(" \t\n\r") == std::string::npos)
            continue;

        SQLStatement statement;
        statement.content = stmt;
        statement.type = determineStatementType(stmt);

        if (statement.type == SQLStatement::Type::CREATE_TABLE || statement.type == SQLStatement::Type::INSERT)
            statement.tableName = extractTableName(stmt);

        statements.push_back(statement);
    }

    return statements;
}

SQLScriptParser::SQLStatement::Type SQLScriptParser::determineStatementType(const std::string& stmt)
{
    std::string upperStmt = stmt;
    std::transform(upperStmt.begin(), upperStmt.end(), upperStmt.begin(), ::toupper);

    if (upperStmt.find("SET ") == 0)
        return SQLStatement::Type::SET;

    if (upperStmt.find("CREATE DATABASE") != std::string::npos)
        return SQLStatement::Type::CREATE_DATABASE;

    if (upperStmt.find("USE ") == 0)
        return SQLStatement::Type::USE;

    if (upperStmt.find("CREATE TABLE") != std::string::npos)
        return SQLStatement::Type::CREATE_TABLE;

    if (upperStmt.find("INSERT INTO") != std::string::npos)
        return SQLStatement::Type::INSERT;

    return SQLStatement::Type::OTHER;
}

std::string SQLScriptParser::extractTableName(const std::string& stmt)
{
    std::string upperStmt = stmt;
    std::transform(upperStmt.begin(), upperStmt.end(), upperStmt.begin(), ::toupper);

    std::string tableName;
    size_t pos;

    if ((pos = upperStmt.find("CREATE TABLE")) != std::string::npos)
        pos += 12;
    else if ((pos = upperStmt.find("INSERT INTO")) != std::string::npos)
        pos += 11;
    else
        return "";

    // Skip whitespace
    while (pos < stmt.length() && std::isspace(stmt[pos]))
        pos++;

    // Handle backtick quotes
    if (stmt[pos] == '`')
    {
        pos++; // Skip opening backtick
        size_t end = stmt.find('`', pos);

        if (end != std::string::npos)
            tableName = stmt.substr(pos, end - pos);
    }
    else
    {
        while (pos < stmt.length() && !std::isspace(stmt[pos]) && stmt[pos] != '(' && stmt[pos] != ';')
        {
            tableName += stmt[pos];
            pos++;
        }
    }

    return tableName;
}

std::vector<std::string> SQLScriptParser::splitStatements(const std::string& sql)
{
    std::vector<std::string> statements;
    std::string currentStmt;
    bool inString = false;
    bool inComment = false;
    bool lineComment = false;

    for (size_t i = 0; i < sql.length(); i++)
    {
        char c = sql[i];
        char next = (i + 1 < sql.length()) ? sql[i + 1] : '\0';

        // Handle comments
        if (!inString)
        {
            if (!inComment && !lineComment && c == '-' && next == '-')
            {
                lineComment = true;
                i++;
                continue;
            }
            if (!inComment && !lineComment && c == '/' && next == '*')
            {
                inComment = true;
                i++;
                continue;
            }
            if (inComment && c == '*' && next == '/')
            {
                inComment = false;
                i++;
                continue;
            }
            if (lineComment && c == '\n')
            {
                lineComment = false;
                continue;
            }
        }

        // Skip if in comment
        if (inComment || lineComment)
            continue;

        // Handle strings
        if (c == '\'' && (i == 0 || sql[i - 1] != '\\'))
            inString = !inString;

        // Handle statement termination
        if (!inString && c == ';')
        {
            currentStmt += c;
            if (!currentStmt.empty())
            {
                statements.push_back(currentStmt);
                currentStmt.clear();
            }
            continue;
        }

        currentStmt += c;
    }

    // Add last statement if it doesn't end with semicolon
    if (!currentStmt.empty())
        statements.push_back(currentStmt);

    return statements;
}
