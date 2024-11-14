#pragma once

#include "DiagramTypes.h"

#include <raylib.h>

class TableRenderer
{
public:
    static void render(const Diagram::TableNode& table, const Rectangle& bounds, const Vector2& pan, float zoom);

private:
    static void drawHeader(const std::string& name, const Rectangle& headerRect);
    static void drawColumns(const std::vector<Diagram::TableColumn>& columns, const Rectangle& contentRect, float rowHeight);
    static void drawColumn(const Diagram::TableColumn& column, const Rectangle& rowRect, float fontSize, float padding);

private:
    static constexpr float TABLE_PADDING = Diagram::TABLE_PADDING;
    static constexpr float TABLE_HEADER_HEIGHT = Diagram::TABLE_HEADER_HEIGHT;
    static constexpr float ROW_HEIGHT = Diagram::ROW_HEIGHT;
};
