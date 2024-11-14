#include "TableRenderer.h"

#include <algorithm>

void TableRenderer::render(const Diagram::TableNode& table, const Rectangle& bounds, const Vector2& pan, float zoom)
{
    Vector2 pos = {bounds.x + table.position.x * zoom + pan.x, bounds.y + table.position.y * zoom + pan.y};

    Rectangle tableRect = {pos.x, pos.y, table.size.x * zoom, table.size.y * zoom};

    DrawRectangleRec(tableRect, WHITE);
    DrawRectangleLinesEx(tableRect, 1, table.isSelected ? BLUE : DARKGRAY);

    Rectangle headerRect = {tableRect.x, tableRect.y, tableRect.width, TABLE_HEADER_HEIGHT * zoom};
    drawHeader(table.name, headerRect);

    Rectangle contentRect = {tableRect.x, headerRect.y + headerRect.height, tableRect.width, tableRect.height - headerRect.height};
    drawColumns(table.columns, contentRect, ROW_HEIGHT * zoom);
}

void TableRenderer::drawHeader(const std::string& name, const Rectangle& headerRect)
{
    DrawRectangleRec(headerRect, Color{230, 230, 230, 255});

    float fontSize = std::min(16.0f, headerRect.height * 0.6f);

    Vector2 textSize = MeasureTextEx(GetFontDefault(), name.c_str(), fontSize, 1);
    Vector2 textPos = {headerRect.x + (headerRect.width - textSize.x) / 2, headerRect.y + (headerRect.height - textSize.y) / 2};

    DrawTextEx(GetFontDefault(), name.c_str(), textPos, fontSize, 1, DARKGRAY);
}

void TableRenderer::drawColumns(const std::vector<Diagram::TableColumn>& columns, const Rectangle& contentRect, float rowHeight)
{
    float yOffset = contentRect.y;
    float fontSize = std::min(14.0f, rowHeight * 0.6f);
    float padding = TABLE_PADDING;

    for (const auto& column : columns)
    {
        Rectangle rowRect = {contentRect.x, yOffset, contentRect.width, rowHeight};
        drawColumn(column, rowRect, fontSize, padding);
        yOffset += rowHeight;
    }
}

void TableRenderer::drawColumn(const Diagram::TableColumn& column, const Rectangle& rowRect, float fontSize, float padding)
{
    DrawRectangleRec(rowRect, WHITE);
    DrawRectangleLinesEx(rowRect, 1, Color{230, 230, 230, 255});

    std::string columnText = column.name;
    if (column.isPrimaryKey)
        columnText = "[PK] " + columnText;
    if (column.isForeignKey)
        columnText = "[FK] " + columnText;

    float availableWidth = rowRect.width - (padding * 3);
    float maxNameWidth = availableWidth * 0.7f;
    float maxTypeWidth = availableWidth * 0.3f;

    std::string displayText = columnText;
    float textWidth = MeasureText(displayText.c_str(), fontSize);
    if (textWidth > maxNameWidth)
    {
        while (textWidth > maxNameWidth - MeasureText("...", fontSize))
        {
            displayText = displayText.substr(0, displayText.length() - 1);
            textWidth = MeasureText(displayText.c_str(), fontSize);
        }
        displayText += "...";
    }

    DrawTextEx(GetFontDefault(), displayText.c_str(), {rowRect.x + padding, rowRect.y + (rowRect.height - fontSize) / 2}, fontSize, 1,
               column.isPrimaryKey ? DARKBLUE : DARKGRAY);

    std::string typeText = column.type;
    float typeTextWidth = MeasureText(typeText.c_str(), fontSize * 0.9f);
    if (typeTextWidth > maxTypeWidth)
    {
        while (typeTextWidth > maxTypeWidth - MeasureText("...", fontSize * 0.9f))
        {
            typeText = typeText.substr(0, typeText.length() - 1);
            typeTextWidth = MeasureText(typeText.c_str(), fontSize * 0.9f);
        }
        typeText += "...";
    }

    DrawTextEx(GetFontDefault(), typeText.c_str(),
               {rowRect.x + rowRect.width - typeTextWidth - padding, rowRect.y + (rowRect.height - fontSize) / 2}, fontSize * 0.9f, 1,
               GRAY);
}
