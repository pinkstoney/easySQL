#pragma once

#include "../../core/export/DatabaseExporter.h"

#include "../components/IconRenderer.h"

#include "../components/dialogs/ExportDialog.h"
#include "../core/EventData.h"
#include "../core/EventType.h"
#include "Panel.h"

#include <memory>
#include <string>

#include <raylib.h>

class GuiManager;

class QueryPanel : public Panel
{
public:
    QueryPanel(float startX, float startY, float width, GuiManager& manager);
    ~QueryPanel() = default;

public:
    void render(bool hasData = false) override;
    std::string getQueryText() const;
    void clearQuery();

public:
    bool shouldExecuteQuery() const;
    bool shouldShowObjects();
    bool shouldExportCSV() const;
    bool shouldShowERDiagram();

    float getStartX() const { return queryBox.x; }
    float getHeight() const { return 60; }
    char* getQueryInput() { return queryInput; }
    bool getObjectsVisibility() const { return objectsVisible; }
    bool getERDiagramVisibility() const { return erDiagramVisible; }

    void setObjectsVisibility(bool visible);
    void setERDiagramVisibility(bool visible);

public:
    void simulateExecuteQuery()
    {
        executeQueryFlag = true;
        queryFromTableClick = true;
    }
    void resetObjectsVisibility() { objectsVisible = false; }
    bool isQueryFromTableClick() const { return queryFromTableClick; }
    bool isExportDialogActive() const { return exportDialog.isVisible(); }

public:
    bool shouldExportDatabase();
    bool shouldImportDatabase() const;

private:
    void setupSubscriptions();
    void renderQueryInput();
    void renderButtons(bool hasData);
    void resetQueryFromTableClick() { queryFromTableClick = false; }

private:
    Rectangle queryBox;
    Rectangle sendQueryBtn;
    Rectangle showObjectsBtn;
    Rectangle saveToCSVBtn;
    Rectangle showERDiagramBtn;
    Rectangle exportDatabaseBtn;
    Rectangle importDatabaseBtn;

private:
    static const int QUERY_BUFFER_SIZE = 512;
    char queryInput[QUERY_BUFFER_SIZE];
    bool objectsVisible = false;
    bool executeQueryFlag = false;
    bool queryFromTableClick = false;
    bool erDiagramVisible = false;

private:
    static constexpr float EXECUTE_BUTTON_WIDTH = 35.0f;

    ExportDialog exportDialog;
    GuiManager& manager;
};
