#pragma once

#include "../../models/QueryResult.h"
#include "../components/IconRenderer.h"
#include "../components/ScrollableList.h"
#include "../core/EventData.h"
#include "../core/EventType.h"
#include "Panel.h"

#include <functional>
#include <memory>
#include <raylib.h>
#include <vector>

class QueryPanel;

class ResultsPanel : public Panel
{
public:
    using TableClickCallback = std::function<void(const std::string&)>;
    using TableStructureCallback = std::function<void(const std::string&)>;

public:
    ResultsPanel(float startX, float startY, float width, float height, float screenHeight);
    ~ResultsPanel() = default;

public:
    void render(bool hasData = false) override;
    void renderQueryResults(const QueryResult& result);
    void renderTableList(const std::vector<std::string>& tables);

public:
    void setTableClickCallback(TableClickCallback callback) { onTableClick = callback; }
    void setTableStructureCallback(TableStructureCallback callback) { onTableStructure = callback; }
    float getStartX() const { return tableBox.x; }

public:
    ScrollableList::DatabaseObjectType getCurrentObjectType() const
    {
        switch (currentTabIndex)
        {
        case 0:
            return ScrollableList::DatabaseObjectType::Table;
        case 1:
            return ScrollableList::DatabaseObjectType::View;
        case 2:
            return ScrollableList::DatabaseObjectType::StoredProcedure;
        case 3:
            return ScrollableList::DatabaseObjectType::Function;
        default:
            return ScrollableList::DatabaseObjectType::Table;
        }
    }

public:
    void setQueryPanel(std::shared_ptr<QueryPanel> panel) { queryPanel = panel; }

private:
    void setupSubscriptions();
    void handleQueryExecuted(const QueryResult* result);
    void handleTablesLoaded(const std::vector<std::string>* tables);
    void handleDatabaseDisconnected();

    void drawTableCell(float x, float y, float width, Color bgColor, const char* text, int padding, float height = 20);
    std::vector<int> calculateColumnWidths(const QueryResult& result, int padding);
    bool isMouseOverTable(float y, float height) const;

    void initializeTableList(float startX, float startY);
    void updateTableList();

    void renderObjectTabs();
    void updateListContent();

private:
    Rectangle tableBox;
    Rectangle tableNamesBox;
    float tableListScroll = 0.0f;
    TableClickCallback onTableClick;
    TableStructureCallback onTableStructure;

private:
    QueryResult currentResult;
    std::vector<std::string> currentTables;
    std::vector<std::string> currentViews;
    std::vector<std::string> currentProcedures;
    std::vector<std::string> currentFunctions;
    int currentTabIndex = 0;

private:
    static constexpr float TABLE_LIST_WIDTH = 200.0f;
    static constexpr float TABLE_ITEM_HEIGHT = 25.0f;
    static constexpr float TABLE_LIST_START_Y = 120.0f;
    static constexpr float TABLE_LIST_HEIGHT = 300.0f;
    static constexpr float STRUCTURE_BUTTON_WIDTH = 35.0f;

private:
    float screenHeight;
    std::unique_ptr<ScrollableList> tableList;
    bool showTables = false;

    float scrollX = 0;
    float scrollY = 0;

private:
    std::shared_ptr<QueryPanel> queryPanel;
};
