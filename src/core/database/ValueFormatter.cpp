#include "ValueFormatter.h"

#include <iomanip>
#include <sstream>

std::string ValueFormatter::format(const mysqlx::Value& value)
{
    if (value.isNull())
        return "NULL";

    try
    {
        switch (value.getType())
        {
        case mysqlx::Value::Type::BOOL:
            return value.get<bool>() ? "TRUE" : "FALSE";

        case mysqlx::Value::Type::INT64:
            return std::to_string(value.get<int64_t>());

        case mysqlx::Value::Type::UINT64:
            return std::to_string(value.get<uint64_t>());

        case mysqlx::Value::Type::FLOAT:
        case mysqlx::Value::Type::DOUBLE: {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << value.get<double>();
            return ss.str();
        }

        case mysqlx::Value::Type::STRING:
            return value.get<std::string>();

        case mysqlx::Value::Type::RAW:
            return formatDateTime(value);

        default:
            return "Unsupported Type";
        }
    }
    catch (const mysqlx::Error& err)
    {
        return "Error: " + std::string(err.what());
    }
}

std::string ValueFormatter::formatTime(const unsigned char* data, int length)
{
    char text[9];

    if (length == 4)
    { // TIME with hours, minutes, seconds
        int32_t hour = data[1] % 24;
        int32_t minute = data[2] % 60;
        int32_t second = data[3] % 60;
        snprintf(text, sizeof(text), "%02d:%02d:%02d", hour, minute, second);
    }
    else if (length == 3)
    { // TIME with hours, minutes
        int32_t hour = data[1] % 24;
        int32_t minute = data[2] % 60;
        snprintf(text, sizeof(text), "%02d:%02d:00", hour, minute);
    }
    else if (length == 2)
    { // TIME with hours
        int32_t hour = data[1] % 24;
        snprintf(text, sizeof(text), "%02d:00:00", hour);
    }
    else
    { // Default or invalid
        return "00:00:00";
    }

    return text;
}

std::string ValueFormatter::formatDate(const unsigned char* data)
{
    uint32_t packed_date = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    uint32_t year = packed_date & 0xFFFF;
    uint32_t month = (packed_date >> 16) & 0xFF;
    uint32_t day = (packed_date >> 24) & 0xFF;

    if (year > 2155)
        year = year - 2048;

    char text[11];
    snprintf(text, sizeof(text), "%04d-%02d-%02d", year, month, day);
    return text;
}

std::string ValueFormatter::formatDateTime(const mysqlx::Value& value)
{
    if (value.getType() == mysqlx::Value::Type::RAW)
    {
        mysqlx::bytes data = value.getRawBytes();
        const unsigned char* first = data.begin();
        int lengthOfData = data.length();

        if (lengthOfData == 4 && first[0] != 0)
            return formatDate(first);
        else if (lengthOfData >= 1 && lengthOfData <= 4)
            return formatTime(first, lengthOfData);
    }
    return "Unknown Date/Time Format";
}
