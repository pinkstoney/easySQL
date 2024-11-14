#pragma once

#include <string>
#include <vector>

#include <raylib.h>

namespace Diagram
{
struct TableColumn
{
    std::string name;
    std::string type;
    bool isPrimaryKey;
    bool isForeignKey;
    std::string referencedTable;
    std::string referencedColumn;
};

struct TableNode
{
    std::string name;
    std::vector<TableColumn> columns;
    Vector2 position;
    Vector2 size;
    bool isSelected = false;
    bool isDragging = false;
};

struct Relationship
{
    std::string fromTable;
    std::string fromColumn;
    std::string toTable;
    std::string toColumn;
};

constexpr float TABLE_PADDING = 10.0f;
constexpr float TABLE_HEADER_HEIGHT = 30.0f;
constexpr float ROW_HEIGHT = 25.0f;
constexpr float MIN_TABLE_WIDTH = 200.0f;
} // namespace Diagram
