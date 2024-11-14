#include "QueryPanel.h"

#include "../../core/export/DatabaseExporter.h"
#include "../GuiManager.h"
#include "../components/IconRenderer.h"
#include "../core/EventData.h"
#include "../core/EventType.h"

#include <cstring>
#include <iostream>
#include <memory>

#include "../include/raygui.h"
#include <raylib.h>

QueryPanel::QueryPanel(float startX, float startY, float width, GuiManager& mgr)
    : manager(mgr)
{
    float buttonWidth = 260;
    float buttonSpacingX = 15;
    float buttonSpacingY = 45;

    queryBox = {startX, startY, width - EXECUTE_BUTTON_WIDTH - 20, 30};
    sendQueryBtn = {startX + width - EXECUTE_BUTTON_WIDTH - 10, startY, EXECUTE_BUTTON_WIDTH, 30};

    showObjectsBtn = {35, 540, buttonWidth / 2 - 5, 30};
    showERDiagramBtn = {showObjectsBtn.x + buttonWidth / 2 + buttonSpacingX, showObjectsBtn.y, buttonWidth / 2 - 5, 30};

    exportDatabaseBtn = {35, showObjectsBtn.y + 40, buttonWidth + 10, 30};

    saveToCSVBtn = {startX + width - buttonWidth / 2, startY + buttonSpacingY * 2, buttonWidth / 2, 30};

    memset(queryInput, 0, QUERY_BUFFER_SIZE);
    setupSubscriptions();

    exportDialog.setExportCallback([this](const std::string& path, const std::string& filename) {
        std::cout << "\n=== Export Callback Debug ===" << std::endl;
        std::cout << "Export callback triggered" << std::endl;
        std::cout << "Path: " << path << std::endl;
        std::cout << "Filename: " << filename << std::endl;

        auto dbManager = manager.getDatabaseManager().get();
        auto tableManager = manager.getTableManager().get();

        std::cout << "Database Manager: " << (dbManager ? "valid" : "null") << std::endl;
        std::cout << "Table Manager: " << (tableManager ? "valid" : "null") << std::endl;

        if (!dbManager || !tableManager)
        {
            std::cout << "Error: Manager(s) are null" << std::endl;
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Database connection not properly initialized", true});
            return;
        }

        std::string fullPath = path + "/" + filename;
        std::cout << "Full export path: " << fullPath << std::endl;

        try
        {
            std::cout << "Starting export..." << std::endl;
            bool result = DatabaseExporter::exportToSQL(dbManager, tableManager, fullPath);
            std::cout << "Export result: " << (result ? "success" : "failure") << std::endl;

            if (result)
                manager.publishEvent(EventType::ExportCompleted, ErrorData{"Database exported to " + fullPath, false});
            else
                manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Failed to export database", true});
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception during export: " << e.what() << std::endl;
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Export error: " + std::string(e.what()), true});
        }
        catch (...)
        {
            std::cout << "Unknown exception during export" << std::endl;
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Unknown export error occurred", true});
        }
    });
}

void QueryPanel::setupSubscriptions()
{
    auto& bus = EventBus::getInstance();

    // Subscribe to DatabaseConnected events
    subscriptionIds.push_back({EventType::DatabaseConnected, bus.subscribe(EventType::DatabaseConnected, [this](const auto&) {
                                   clearQuery();
                                   resetObjectsVisibility();
                                   erDiagramVisible = false;
                                   queryFromTableClick = false;
                               })});

    // Subscribe to DatabaseDisconnected events
    subscriptionIds.push_back({EventType::DatabaseDisconnected, bus.subscribe(EventType::DatabaseDisconnected, [this](const auto&) {
                                   clearQuery();
                                   resetObjectsVisibility();
                                   erDiagramVisible = false;
                                   queryFromTableClick = false;
                               })});

    // Subscribe to StateReset events
    subscriptionIds.push_back({EventType::StateReset, bus.subscribe(EventType::StateReset, [this](const auto&) {
                                   clearQuery();
                                   resetObjectsVisibility();
                                   erDiagramVisible = false;
                                   queryFromTableClick = false;
                               })});

    // Subscribe to QueryExecuted events
    subscriptionIds.push_back({EventType::QueryExecuted, bus.subscribe(EventType::QueryExecuted, [this](const auto& data) {
                                   auto* queryData = std::any_cast<QueryExecutedData>(&data);
                                   if (queryData && queryData->success)
                                   {
                                       if (!queryFromTableClick && !objectsVisible)
                                           resetObjectsVisibility();
                                   }
                               })});
}

void QueryPanel::render(bool hasData)
{
    if (!exportDialog.isVisible())
    {
        renderQueryInput();

        bool isExecuteHovered = CheckCollisionPointRec(GetMousePosition(), sendQueryBtn);

        Color btnColor = isExecuteHovered ? Color{40, 160, 40, 255} : Color{60, 180, 60, 255};

        DrawRectangleRounded(Rectangle{sendQueryBtn.x + 2, sendQueryBtn.y + 2, sendQueryBtn.width, sendQueryBtn.height}, 0.3f, 8,
                             Color{30, 30, 30, 60});

        DrawRectangleRounded(sendQueryBtn, 0.3f, 8, btnColor);

        float iconOffset = isExecuteHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON) ? 1.0f : 0.0f;
        IconRenderer::drawExecuteIcon(
            Rectangle{sendQueryBtn.x + iconOffset, sendQueryBtn.y + iconOffset, sendQueryBtn.width, sendQueryBtn.height}, WHITE);

        bool isObjectsBtnHovered = CheckCollisionPointRec(GetMousePosition(), showObjectsBtn);
        Color objectsBtnColor = objectsVisible ? (isObjectsBtnHovered ? Color{100, 100, 100, 255} : Color{130, 130, 130, 255})
                                               : (isObjectsBtnHovered ? Color{40, 160, 40, 255} : Color{60, 180, 60, 255});

        DrawRectangleRounded(showObjectsBtn, 0.2f, 8, objectsBtnColor);

        const char* objBtnText = objectsVisible ? "Hide Objects" : "Show Objects";
        int objTextWidth = MeasureText(objBtnText, 18);
        DrawText(objBtnText, showObjectsBtn.x + (showObjectsBtn.width - objTextWidth) / 2, showObjectsBtn.y + 7, 18, WHITE);

        // Draw ER Diagram button
        bool isERDiagramBtnHovered = CheckCollisionPointRec(GetMousePosition(), showERDiagramBtn);
        Color erBtnColor = erDiagramVisible ? (isERDiagramBtnHovered ? Color{100, 100, 100, 255} : Color{130, 130, 130, 255})
                                            : (isERDiagramBtnHovered ? Color{40, 160, 40, 255} : Color{60, 180, 60, 255});

        DrawRectangleRounded(showERDiagramBtn, 0.2f, 8, erBtnColor);

        const char* erBtnText = erDiagramVisible ? "Hide Diagram" : "Show Diagram";
        int erTextWidth = MeasureText(erBtnText, 18);
        DrawText(erBtnText, showERDiagramBtn.x + (showERDiagramBtn.width - erTextWidth) / 2, showERDiagramBtn.y + 7, 18, WHITE);

        bool isExportBtnHovered = CheckCollisionPointRec(GetMousePosition(), exportDatabaseBtn);
        Color exportBtnColor = isExportBtnHovered ? Color{0, 100, 180, 255} : Color{0, 120, 210, 255};

        DrawRectangleRounded(exportDatabaseBtn, 0.2f, 8, exportBtnColor);

        const char* exportBtnText = "Export SQL";
        int exportTextWidth = MeasureText(exportBtnText, 18);
        DrawText(exportBtnText, exportDatabaseBtn.x + (exportDatabaseBtn.width - exportTextWidth) / 2, exportDatabaseBtn.y + 7, 18,
                 WHITE);

        if (hasData)
            GuiButton(saveToCSVBtn, "Export CSV");
    }

    std::cout << "Export dialog visible: " << exportDialog.isVisible() << std::endl;
    exportDialog.render();
}

void QueryPanel::renderQueryInput()
{
    static bool queryActive = false;

    DrawRectangleRec(queryBox, WHITE);
    if (GuiTextBox(queryBox, queryInput, QUERY_BUFFER_SIZE, queryActive))
        queryActive = !queryActive;
}

std::string QueryPanel::getQueryText() const
{
    return std::string(queryInput);
}

void QueryPanel::clearQuery()
{
    memset(queryInput, 0, QUERY_BUFFER_SIZE);
}

bool QueryPanel::shouldExecuteQuery() const
{
    if (exportDialog.isVisible())
        return false; 

    if (executeQueryFlag)
    {
        const_cast<QueryPanel*>(this)->executeQueryFlag = false;
        return true;
    }

    bool clicked = CheckCollisionPointRec(GetMousePosition(), sendQueryBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (clicked)
    {
        const_cast<QueryPanel*>(this)->resetQueryFromTableClick();
    }
    return clicked;
}

bool QueryPanel::shouldShowObjects()
{
    if (exportDialog.isVisible())
        return false; 

    bool clicked = CheckCollisionPointRec(GetMousePosition(), showObjectsBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (clicked)
    {
        resetQueryFromTableClick();
    }
    return clicked;
}

bool QueryPanel::shouldExportCSV() const
{
    if (exportDialog.isVisible())
        return false; 
    return GuiButton(saveToCSVBtn, "Export CSV");
}

void QueryPanel::setObjectsVisibility(bool visible)
{
    objectsVisible = visible;
    if (!visible)
        resetQueryFromTableClick();
}

bool QueryPanel::shouldShowERDiagram()
{
    if (exportDialog.isVisible())
        return false; 

    bool clicked = CheckCollisionPointRec(GetMousePosition(), showERDiagramBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (clicked)
    {
        std::cout << "ER Diagram button clicked. Current visibility: " << erDiagramVisible << std::endl;
    }
    return clicked;
}

void QueryPanel::setERDiagramVisibility(bool visible)
{
    erDiagramVisible = visible;
}

bool QueryPanel::shouldExportDatabase()
{
    if (exportDialog.isVisible())
        return false;

    if (CheckCollisionPointRec(GetMousePosition(), exportDatabaseBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        std::cout << "Export button clicked, showing dialog..." << std::endl;

        auto dbManager = manager.getDatabaseManager().get();
        if (dbManager)
        {
            try
            {
                auto result = dbManager->getSession().sql("SELECT DATABASE()").execute();
                auto row = result.fetchOne();
                std::string dbName = row[0].get<std::string>();
                exportDialog.showWithDatabase(dbName);
            }
            catch (const std::exception& e)
            {
                std::cout << "Error getting database name: " << e.what() << std::endl;
                exportDialog.show(); 
            }
        }
        else
        {
            exportDialog.show(); 
        }

        std::cout << "Dialog visibility after show: " << exportDialog.isVisible() << std::endl;
        return false;
    }
    return false;
}

bool QueryPanel::shouldImportDatabase() const
{
    return CheckCollisionPointRec(GetMousePosition(), importDatabaseBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
