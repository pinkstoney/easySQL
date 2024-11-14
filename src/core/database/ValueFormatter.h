#pragma once

#include <string>

#include <mysqlx/xdevapi.h>

class ValueFormatter
{
public:
    static std::string format(const mysqlx::Value& value);

private:
    static std::string formatTime(const unsigned char* data, int length);
    static std::string formatDate(const unsigned char* data);
    static std::string formatDateTime(const mysqlx::Value& value);
    static std::string formatNumeric(const mysqlx::Value& value);
};
