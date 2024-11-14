#pragma once

#include "../../models/DatabaseConnectionInfo.h"

#include "../components/ScrollableList.h"
#include "../components/dialogs/ImportDialog.h"

#include "Panel.h"

#include <memory>
#include <string>
#include <vector>

#include <raylib.h>

class QueryPanel;
class GuiManager;

class ConnectionPanel : public Panel
{
public:
    enum class ConnectionState
    {
        DISCONNECTED,
        SERVER_CONNECTED,
        DATABASE_CONNECTED
    };

public:
    ConnectionPanel(float startX, float startY, float screenHeight, GuiManager& mgr);
    ~ConnectionPanel() = default;

public:
    void render(bool hasData = false) override;
    bool handleConnect();
    bool handleExit() const;

public:
    DatabaseConnectionInfo getConnectionInfo() const;
    float getWidth() const { return connectBtn.x + connectBtn.width; }
    float getStartX() const { return hostBox.x - 85; }

    void setAvailableDatabases(const std::vector<std::string>& dbs);
    bool isDatabaseSelected() const;

    std::string getSelectedDatabase() const;
    ConnectionState getState() const { return state; }
    void setState(ConnectionState newState);

    bool shouldImportDatabase();
    bool isExportDialogActive() const;
    bool isImportDialogVisible() const;

    void setQueryPanel(std::shared_ptr<QueryPanel> panel) { queryPanel = panel; }

private:
    void setupSubscriptions();
    void renderConnectionForm();
    void renderDatabaseSelection();
    void renderConnectionStatus();
    void resetConnection();

private:
    Rectangle drawStatusContainer();
    void drawConnectionIndicator(Rectangle statusRect);
    void drawConnectionText(Rectangle statusRect);
    void drawTruncatedText(Rectangle statusRect, const std::string& fullText, float maxTextWidth);
    void drawStatusTooltip(Rectangle tooltipRect, const std::string& text);
    void drawFullText(Rectangle statusRect, const std::string& text);

    std::string getTruncatedText(const std::string& text, float maxWidth);
    void handleStatusTooltip(Rectangle statusRect, const std::string& fullText);
    Rectangle calculateTooltipRect(Rectangle statusRect, const std::string& text);

private:
    Rectangle hostBox;
    Rectangle portBox;
    Rectangle userBox;
    Rectangle passwordBox;
    Rectangle connectBtn;
    Rectangle exitBtn;
    Rectangle importDatabaseBtn;

private:
    DatabaseConnectionInfo connInfo;
    std::vector<std::string> databases;
    int selectedDbIndex = -1;
    ConnectionState state = ConnectionState::DISCONNECTED;
    float screenHeight;

    std::unique_ptr<ScrollableList> databaseList;
    float databaseListScroll = 0.0f;

private:
    struct FormField
    {
        const char* label;
        Rectangle* box;
        char* value;
        int maxLength;
        bool* active;
        float labelOffset;
    };

private:
    void initializePanelLayout(float startX, float startY);
    void initializeInputFields(float startX, float startY, float labelWidth, float inputWidth, float spacingY);
    void initializeButtons(float startX, float panelWidth, float sideMargin);

private:
    void drawPanelBackground();
    void drawPanelTitle();
    void renderCurrentState();
    void drawButtons();
    void drawConnectButton();
    void drawExitButton();
    void drawCenteredText(const char* text, Rectangle rect, int fontSize, Color color);

private:
    void drawFormTitle();
    void drawFormFields();
    void drawFormField(const FormField& field, float labelX, float yPos);

private:
    void initializeDatabaseList(float startX, float startY);
    void updateDatabaseList();

private:
    std::shared_ptr<QueryPanel> queryPanel;
    ImportDialog importDialog;
    GuiManager& manager;
};
