#include "ResultsPanel.h"
#include "QueryPanel.h"

#include <iostream>

#include "../include/raygui.h"
#include <raylib.h>

ResultsPanel::ResultsPanel(float startX, float startY, float width, float height, float screenHeight)
    : screenHeight(screenHeight)
{
    tableBox = {startX, startY, width, height};
    initializeTableList(startX, startY);
    setupSubscriptions();
}

void ResultsPanel::initializeTableList(float startX, float startY)
{
    float listStartY = startY + 90;
    float titleY = listStartY - 65;

    tableList = std::make_unique<ScrollableList>("Database Objects", startX - 290, listStartY, 230, screenHeight - 450, 32, titleY);
    tableList->setCallbacks(
        [this](const std::string& tableName) {
            if (onTableClick)
                onTableClick(tableName);
        },
        [this](const std::string& tableName) {
            if (onTableStructure)
                onTableStructure(tableName);
        });
}

void ResultsPanel::setupSubscriptions()
{
    auto& bus = EventBus::getInstance();

    // Subscribe to QueryExecuted events
    subscriptionIds.push_back({EventType::QueryExecuted, bus.subscribe(EventType::QueryExecuted, [this](const auto& data) {
                                   auto* queryData = std::any_cast<QueryExecutedData>(&data);
                                   if (queryData && queryData->success)
                                   {
                                       currentResult = queryData->result;
                                       showTables = false;
                                   }
                               })});

    // Subscribe to TablesLoaded events
    subscriptionIds.push_back({EventType::TablesLoaded, bus.subscribe(EventType::TablesLoaded, [this](const auto& data) {
                                   auto* tablesData = std::any_cast<TablesLoadedData>(&data);
                                   if (tablesData)
                                   {
                                       currentTables = tablesData->tables;
                                       currentViews = tablesData->views;
                                       currentProcedures = tablesData->procedures;
                                       currentFunctions = tablesData->functions;
                                       showTables = tablesData->success;
                                       if (showTables)
                                       {
                                           currentResult = QueryResult();
                                           updateListContent();
                                       }
                                   }
                               })});

    // Subscribe to DatabaseConnected events
    subscriptionIds.push_back({EventType::DatabaseConnected, bus.subscribe(EventType::DatabaseConnected, [this](const auto&) {
                                   currentResult = QueryResult();
                                   currentTables.clear();
                                   currentViews.clear();
                                   currentProcedures.clear();
                                   currentFunctions.clear();
                                   showTables = false;
                                   tableListScroll = 0.0f;
                               })});

    // Subscribe to reset events
    subscriptionIds.push_back({EventType::DatabaseDisconnected, bus.subscribe(EventType::DatabaseDisconnected, [this](const auto&) {
                                   currentResult = QueryResult();
                                   currentTables.clear();
                                   currentViews.clear();
                                   currentProcedures.clear();
                                   currentFunctions.clear();
                                   showTables = false;
                                   tableListScroll = 0.0f;
                               })});
}

void ResultsPanel::render(bool hasData)
{
    if (queryPanel && queryPanel->isExportDialogActive())
        return; // Just return without clearing the data

    if (showTables && tableList)
    {
        currentResult = QueryResult(); 
        renderObjectTabs();
        renderTableList(currentTables);
    }
    else if (!currentResult.columns.empty() && !queryPanel->isExportDialogActive())
    {
        renderQueryResults(currentResult);
    }
}

void ResultsPanel::renderQueryResults(const QueryResult& result)
{
    if (result.columns.empty() || (queryPanel && queryPanel->isExportDialogActive()))
        return;

    DrawRectangleRec(tableBox, RAYWHITE);

    int padding = 5;
    float startX = tableBox.x + padding;
    float startY = tableBox.y + padding + 20;

    std::vector<int> colWidths = calculateColumnWidths(result, padding);

    float totalWidth = 0;
    for (const auto& width : colWidths)
        totalWidth += width;

    float rowHeight = 20;
    float totalHeight = rowHeight;
    for (const auto& row : result.rows)
    {
        float currentRowHeight = 20;
        for (const auto& cell : row)
        {
            size_t newlines = std::count(cell.begin(), cell.end(), '\n');
            float cellHeight = (newlines + 1) * 20.0f;
            currentRowHeight = std::max(currentRowHeight, cellHeight);
        }
        totalHeight += currentRowHeight;
    }

    // Handle horizontal scrolling
    static float scrollX = 0;
    if (totalWidth > tableBox.width)
    {
        float maxScrollX = totalWidth - tableBox.width + padding * 2;
        scrollX = std::min(scrollX, maxScrollX);

        if (CheckCollisionPointRec(GetMousePosition(), tableBox))
        {
            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                scrollX -= GetMouseWheelMove() * 20;
                scrollX = std::max(0.0f, std::min(scrollX, maxScrollX));
            }
        }

        float scrollBarWidth = (tableBox.width / totalWidth) * tableBox.width;
        float scrollBarX = tableBox.x + (scrollX / totalWidth) * (tableBox.width - scrollBarWidth);
        DrawRectangle(tableBox.x, tableBox.y + tableBox.height - 8, tableBox.width, 8, Color{235, 235, 235, 255});
        DrawRectangle(scrollBarX, tableBox.y + tableBox.height - 8, scrollBarWidth, 8, Color{180, 180, 180, 255});
    }

    // Handle vertical scrolling
    static float scrollY = 0;
    if (totalHeight > tableBox.height)
    {
        float maxScrollY = totalHeight - tableBox.height + padding * 2;
        scrollY = std::min(scrollY, maxScrollY);

        if (CheckCollisionPointRec(GetMousePosition(), tableBox))
        {
            if (!IsKeyDown(KEY_LEFT_CONTROL))
            {
                scrollY -= GetMouseWheelMove() * 20;
                scrollY = std::max(0.0f, std::min(scrollY, maxScrollY));
            }
        }

        float scrollBarHeight = (tableBox.height / totalHeight) * tableBox.height;
        float scrollBarY = tableBox.y + (scrollY / totalHeight) * (tableBox.height - scrollBarHeight);
        DrawRectangle(tableBox.x + tableBox.width - 8, tableBox.y, 8, tableBox.height, Color{235, 235, 235, 255});
        DrawRectangle(tableBox.x + tableBox.width - 8, scrollBarY, 8, scrollBarHeight, Color{180, 180, 180, 255});
    }

    BeginScissorMode(tableBox.x, tableBox.y, tableBox.width - 8, tableBox.height - 8);

    // Draw header
    float currentX = startX - scrollX;
    for (size_t i = 0; i < result.columns.size(); ++i)
    {
        drawTableCell(currentX, tableBox.y + padding - scrollY, colWidths[i], GRAY, result.columns[i].c_str(), padding);
        currentX += colWidths[i];
    }

    // Draw rows
    float currentY = startY - scrollY;
    for (const auto& row : result.rows)
    {
        float rowHeight = 20;

        for (const auto& cell : row)
        {
            size_t newlines = std::count(cell.begin(), cell.end(), '\n');
            float cellHeight = (newlines + 1) * 20.0f;
            rowHeight = std::max(rowHeight, cellHeight);
        }

        currentX = startX - scrollX;
        for (size_t i = 0; i < row.size(); ++i)
        {
            if (i >= colWidths.size())
                break;

            drawTableCell(currentX, currentY, colWidths[i], RAYWHITE, row[i].c_str(), padding, rowHeight);
            currentX += colWidths[i];
        }
        currentY += rowHeight;
    }

    EndScissorMode();
}

void ResultsPanel::renderTableList(const std::vector<std::string>& tables)
{
    if (queryPanel && queryPanel->isExportDialogActive())
        return;

    if (tableList)
    {
        renderObjectTabs();
        updateListContent();
    }
}

bool ResultsPanel::isMouseOverTable(float y, float height) const
{
    Vector2 mousePos = GetMousePosition();
    return mousePos.x >= tableNamesBox.x && mousePos.x <= tableNamesBox.x + TABLE_LIST_WIDTH && mousePos.y >= y &&
           mousePos.y <= y + height;
}

void ResultsPanel::drawTableCell(float x, float y, float width, Color bgColor, const char* text, int padding, float height)
{
    DrawRectangle(x, y, width, height, bgColor);
    DrawRectangleLines(x, y, width, height, BLACK);

    std::string str(text);
    float lineHeight = 20;
    float currentY = y + 2;

    size_t pos = 0;
    std::string line;
    while ((pos = str.find('\n')) != std::string::npos)
    {
        line = str.substr(0, pos);
        DrawText(line.c_str(), x + padding, currentY, 18, DARKGRAY);
        currentY += lineHeight;
        str = str.substr(pos + 1);
    }

    if (!str.empty())
        DrawText(str.c_str(), x + padding, currentY, 18, DARKGRAY);
}

std::vector<int> ResultsPanel::calculateColumnWidths(const QueryResult& result, int padding)
{
    std::vector<int> colWidths;
    for (const auto& col : result.columns)
    {
        int width = MeasureText(col.c_str(), 18) + padding * 2;
        for (const auto& row : result.rows)
        {
            if (colWidths.size() >= row.size())
                break;

            int cellWidth = MeasureText(row[colWidths.size()].c_str(), 18) + padding * 2;
            width = std::max(width, cellWidth);
        }
        colWidths.push_back(width);
    }
    return colWidths;
}

void ResultsPanel::renderObjectTabs()
{
    float tabWidth = 55;
    float tabHeight = 22;
    float startX = tableList->getBounds().x;
    float startY = tableList->getBounds().y - tabHeight - 5;
    float totalWidth = tabWidth * 4;

    startX = startX + (tableList->getBounds().width - totalWidth) / 2;

    const char* tabs[] = {"Tables", "Views", "Procs", "Funcs"};
    int numTabs = 4;

    DrawRectangleRec(Rectangle{tableList->getBounds().x, startY, tableList->getBounds().width, tabHeight}, Color{235, 235, 235, 255});

    for (int i = 0; i < numTabs; i++)
    {
        Rectangle tabRect = {startX + (tabWidth * i), startY, tabWidth, tabHeight};

        bool isSelected = currentTabIndex == i;
        bool isHovered = CheckCollisionPointRec(GetMousePosition(), tabRect);

        Color bgColor = isSelected ? RAYWHITE : isHovered ? Color{245, 245, 245, 255} : Color{235, 235, 235, 255};
        DrawRectangleRec(tabRect, bgColor);

        if (isSelected)
        {
            DrawRectangleLinesEx(tabRect, 1, Color{200, 200, 200, 255});
            DrawLine(tabRect.x, tabRect.y + tabRect.height, tabRect.x + tabRect.width, tabRect.y + tabRect.height, RAYWHITE);
        }
        else
        {
            DrawRectangleLinesEx(tabRect, 1, Color{200, 200, 200, 255});
        }

        int textWidth = MeasureText(tabs[i], 14);
        DrawText(tabs[i], tabRect.x + (tabRect.width - textWidth) / 2, tabRect.y + 4, 14, Color{70, 70, 70, 255});

        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            currentTabIndex = i;
            updateListContent();
        }
    }
}

void ResultsPanel::updateListContent()
{
    if (!tableList)
        return;

    std::vector<ScrollableList::ListItem> items;

    switch (currentTabIndex)
    {
    case 0: // Tables
        for (const auto& table : currentTables)
        {
            items.push_back({table, false, true, ScrollableList::DatabaseObjectType::Table});
        }
        break;

    case 1: // Views
        for (const auto& view : currentViews)
        {
            items.push_back({view, false, true, ScrollableList::DatabaseObjectType::View});
        }
        break;

    case 2: // Procedures
        for (const auto& proc : currentProcedures)
        {
            items.push_back({proc, false, false, ScrollableList::DatabaseObjectType::StoredProcedure});
        }
        break;

    case 3: // Functions
        for (const auto& func : currentFunctions)
        {
            items.push_back({func, false, false, ScrollableList::DatabaseObjectType::Function});
        }
        break;
    }

    std::cout << "Updating list content with: " << items.size() << " items for tab " << currentTabIndex << std::endl;
    std::cout << "Tables: " << currentTables.size() << ", Views: " << currentViews.size()
              << ", Procedures: " << currentProcedures.size() << ", Functions: " << currentFunctions.size() << std::endl;

    tableList->render(items);
}
