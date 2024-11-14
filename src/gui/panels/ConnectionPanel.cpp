#include "ConnectionPanel.h"
#include "../../core/export/DatabaseExporter.h"
#include "../GuiManager.h"
#include "../include/raygui.h"
#include "QueryPanel.h"
#include <cstring>
#include <fstream>
#include <iostream>

ConnectionPanel::ConnectionPanel(float startX, float startY, float screenHeight, GuiManager& mgr)
    : screenHeight(screenHeight)
    , manager(mgr)
{
    initializePanelLayout(startX, startY);
    setupSubscriptions();

    importDialog.setImportCallback([this](const std::string& path) {
        std::cout << "Attempting to import from path: " << path << std::endl;

        if (!std::filesystem::exists(path))
        {
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"File not found: " + path, true});
            return;
        }

        auto dbManager = manager.getDatabaseManager().get();
        if (!dbManager)
        {
            std::cout << "Error: Database manager is null" << std::endl;
            manager.publishEvent(EventType::ErrorOccurred,
                                 ErrorData{"Database connection not initialized. Please reconnect to the server.", true});
            return;
        }

        try
        {
            std::ifstream file(path);
            if (!file.is_open())
            {
                manager.publishEvent(EventType::ErrorOccurred,
                                     ErrorData{"Cannot open file: " + path + "\nPlease check file permissions.", true});
                return;
            }

            if (file.peek() == std::ifstream::traits_type::eof())
            {
                manager.publishEvent(EventType::ErrorOccurred, ErrorData{"The SQL file is empty: " + path, true});
                return;
            }

            file.close();

            if (DatabaseExporter::importFromSQL(dbManager, path))
                manager.publishEvent(EventType::ImportCompleted, ErrorData{"Database successfully imported from " + path, false});
            else
                manager.publishEvent(EventType::ErrorOccurred,
                                     ErrorData{"Failed to import database. Please check if the SQL file is valid.", true});
        }
        catch (const mysqlx::Error& e)
        {
            std::cout << "MySQL error during import: " << e.what() << std::endl;
            std::string errorMsg = "MySQL Error: ";

            if (strstr(e.what(), "Access denied"))
                errorMsg += "Access denied. Please check your permissions.";
            else if (strstr(e.what(), "already exists"))
                errorMsg += "Database or table already exists.";
            else
                errorMsg += e.what();

            manager.publishEvent(EventType::ErrorOccurred, ErrorData{errorMsg, true});
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Runtime error during import: " << e.what() << std::endl;
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Import error: " + std::string(e.what()), true});
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception during import: " << e.what() << std::endl;
            manager.publishEvent(EventType::ErrorOccurred,
                                 ErrorData{"Unexpected error during import: " + std::string(e.what()), true});
        }
        catch (...)
        {
            std::cout << "Unknown error during import" << std::endl;
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"An unknown error occurred during import", true});
        }
    });

    importDatabaseBtn = {startX + 20, startY + 400, 120, 30};
}

void ConnectionPanel::initializePanelLayout(float startX, float startY)
{
    float panelWidth = 300;
    float labelWidth = 70;
    float inputWidth = 180;
    float spacingY = 45;
    float sideMargin = 35;

    float formStartX = startX + sideMargin;
    initializeInputFields(formStartX, startY, labelWidth, inputWidth, spacingY);
    initializeButtons(formStartX, panelWidth, sideMargin);
    initializeDatabaseList(formStartX, startY);

    importDatabaseBtn = {formStartX, connectBtn.y - 40, panelWidth - (sideMargin * 2), 32};
}

void ConnectionPanel::initializeInputFields(float startX, float startY, float labelWidth, float inputWidth, float spacingY)
{
    hostBox = {startX + labelWidth, startY + 80, inputWidth, 28};
    portBox = {startX + labelWidth, hostBox.y + spacingY, inputWidth, 28};
    userBox = {startX + labelWidth, portBox.y + spacingY, inputWidth, 28};
    passwordBox = {startX + labelWidth, userBox.y + spacingY, inputWidth, 28};
}

void ConnectionPanel::initializeButtons(float startX, float panelWidth, float sideMargin)
{
    float bottomY = screenHeight - 90;
    float buttonWidth = panelWidth - (sideMargin * 2);
    connectBtn = {startX, bottomY, buttonWidth, 32};
    exitBtn = {startX, bottomY + 40, buttonWidth, 32};

    importDatabaseBtn = {startX, bottomY - 40, buttonWidth, 32};
}

void ConnectionPanel::render(bool hasData)
{
    drawPanelBackground();
    drawPanelTitle();
    renderCurrentState();

    std::cout << "\nRender conditions for import button:" << std::endl;
    std::cout << "State is SERVER_CONNECTED: " << (state == ConnectionState::SERVER_CONNECTED) << std::endl;
    std::cout << "Export dialog not active: " << (!queryPanel->isExportDialogActive()) << std::endl;
    std::cout << "Import dialog not visible: " << (!importDialog.isVisible()) << std::endl;

    if (state == ConnectionState::SERVER_CONNECTED && !queryPanel->isExportDialogActive() && !importDialog.isVisible())
    {

        Vector2 mousePos = GetMousePosition();
        bool isImportBtnHovered = CheckCollisionPointRec(mousePos, importDatabaseBtn);

        std::cout << "Import button bounds: " << importDatabaseBtn.x << ", " << importDatabaseBtn.y << ", " << importDatabaseBtn.width
                  << ", " << importDatabaseBtn.height << std::endl;
        std::cout << "Mouse position: " << mousePos.x << ", " << mousePos.y << std::endl;
        std::cout << "Button hovered: " << isImportBtnHovered << std::endl;

        Color importBtnColor = isImportBtnHovered ? Color{0, 100, 180, 255} : Color{0, 120, 210, 255};

        DrawRectangleRounded(importDatabaseBtn, 0.2f, 8, importBtnColor);

        const char* importBtnText = "Import SQL";
        int importTextWidth = MeasureText(importBtnText, 18);
        DrawText(importBtnText, importDatabaseBtn.x + (importDatabaseBtn.width - importTextWidth) / 2, importDatabaseBtn.y + 7, 18,
                 WHITE);
    }

    if (!importDialog.isVisible() && !queryPanel->isExportDialogActive())
        drawButtons();

    importDialog.render();
}

void ConnectionPanel::drawPanelBackground()
{
    float panelWidth = 300;

    DrawRectangleGradientV(20, 10, panelWidth, screenHeight - 20, RAYWHITE, Color{245, 245, 245, 255});

    DrawRectangleLinesEx(Rectangle{20, 10, panelWidth, screenHeight - 20}, 1, Color{200, 200, 200, 255});
}

void ConnectionPanel::drawPanelTitle()
{
    DrawText("Database Connection", 35, 25, 20, Color{70, 70, 70, 255});
}

void ConnectionPanel::renderCurrentState()
{
    switch (state)
    {
    case ConnectionState::DISCONNECTED:
        renderConnectionForm();
        break;
    case ConnectionState::SERVER_CONNECTED:
        renderDatabaseSelection();
        break;
    case ConnectionState::DATABASE_CONNECTED:
        renderConnectionStatus();
        break;
    }
}

void ConnectionPanel::drawButtons()
{
    if (!isExportDialogActive())
    {
        drawConnectButton();
        drawExitButton();
    }
}

void ConnectionPanel::drawConnectButton()
{
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), connectBtn);
    Color buttonColor = state == ConnectionState::DISCONNECTED ? Color{0, 120, 210, 255} : Color{210, 60, 60, 255};

    Color currentColor = isHovered ? ColorBrightness(buttonColor, 0.2f) : buttonColor;

    DrawRectangleRounded(connectBtn, 0.2f, 8, currentColor);

    const char* buttonText = state == ConnectionState::DISCONNECTED ? "Connect to Server" : "Disconnect";
    drawCenteredText(buttonText, connectBtn, 18, WHITE);
}

void ConnectionPanel::drawExitButton()
{
    bool exitHovered = CheckCollisionPointRec(GetMousePosition(), exitBtn);
    Color exitColor = exitHovered ? Color{230, 230, 230, 255} : Color{220, 220, 220, 255};

    DrawRectangleRounded(exitBtn, 0.2f, 8, exitColor);
    drawCenteredText("Exit", exitBtn, 18, DARKGRAY);
}

void ConnectionPanel::drawCenteredText(const char* text, Rectangle rect, int fontSize, Color color)
{
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, rect.x + (rect.width - textWidth) / 2, rect.y + 7, fontSize, color);
}

void ConnectionPanel::renderConnectionForm()
{
    drawFormTitle();
    drawFormFields();
}

void ConnectionPanel::drawFormTitle()
{
    DrawText("Connection Details", 35, hostBox.y - 35, 18, Color{70, 70, 70, 255});
}

void ConnectionPanel::drawFormFields()
{
    static bool hostActive = false, portActive = false, userActive = false, pwdActive = false;

    FormField fields[] = {{"Host:", &hostBox, connInfo.host, sizeof(connInfo.host), &hostActive, 0},
                          {"Port:", &portBox, connInfo.port, sizeof(connInfo.port), &portActive, 0},
                          {"User:", &userBox, connInfo.user, sizeof(connInfo.user), &userActive, 0},
                          {"Password:", &passwordBox, connInfo.password, sizeof(connInfo.password), &pwdActive, 15}};

    float labelX = hostBox.x - 75;
    float labelSpacingY = 45;

    for (int i = 0; i < 4; i++)
    {
        drawFormField(fields[i], labelX, hostBox.y + labelSpacingY * i);
    }
}

void ConnectionPanel::drawFormField(const FormField& field, float labelX, float yPos)
{
    DrawText(field.label, labelX - field.labelOffset, yPos + 7, 16, Color{70, 70, 70, 255});

    Rectangle inputRect = *field.box;
    DrawRectangleRec(inputRect, RAYWHITE);

    bool isHovered = CheckCollisionPointRec(GetMousePosition(), inputRect);
    Color borderCol = *field.active ? Color{0, 120, 210, 255} : (isHovered ? Color{150, 150, 150, 255} : Color{200, 200, 200, 255});

    DrawRectangleRoundedLines(inputRect, 0.2f, 8, 1.0f, borderCol);

    if (GuiTextBox(*field.box, field.value, field.maxLength, *field.active))
    {
        *field.active = !*field.active;
    }
}

void ConnectionPanel::setupSubscriptions()
{
    auto& bus = EventBus::getInstance();

    // Subscribe to ServerConnected events
    subscriptionIds.push_back({EventType::ServerConnected, bus.subscribe(EventType::ServerConnected, [this](const auto& data) {
                                   auto* connData = std::any_cast<DatabaseConnectedData>(&data);
                                   if (connData && connData->success)
                                   {
                                       state = ConnectionState::SERVER_CONNECTED;
                                   }
                               })});

    // Subscribe to DatabaseConnected events
    subscriptionIds.push_back({EventType::DatabaseConnected, bus.subscribe(EventType::DatabaseConnected, [this](const auto& data) {
                                   auto* connData = std::any_cast<DatabaseConnectedData>(&data);
                                   if (connData && connData->success)
                                   {
                                       state = ConnectionState::DATABASE_CONNECTED;
                                       strcpy(connInfo.dbName, connData->databaseName.c_str());
                                   }
                               })});

    // Subscribe to reset events
    subscriptionIds.push_back({EventType::ServerDisconnected, bus.subscribe(EventType::ServerDisconnected, [this](const auto&) {
                                   state = ConnectionState::DISCONNECTED;
                                   resetConnection();
                               })});

    subscriptionIds.push_back({EventType::DatabaseDisconnected, bus.subscribe(EventType::DatabaseDisconnected, [this](const auto&) {
                                   state = ConnectionState::DISCONNECTED;
                                   resetConnection();
                               })});

    subscriptionIds.push_back({EventType::StateReset, bus.subscribe(EventType::StateReset, [this](const auto&) {
                                   state = ConnectionState::DISCONNECTED;
                                   resetConnection();
                               })});
}

void ConnectionPanel::setState(ConnectionState newState)
{
    state = newState;
    if (state == ConnectionState::DISCONNECTED)
    {
        resetConnection();
    }
}

void ConnectionPanel::resetConnection()
{
    selectedDbIndex = -1;
    databases.clear();
    databaseListScroll = 0.0f;
    memset(connInfo.dbName, 0, sizeof(connInfo.dbName));
}

bool ConnectionPanel::isDatabaseSelected() const
{
    bool selected = selectedDbIndex >= 0 && selectedDbIndex < databases.size();
    std::cout << "isDatabaseSelected: " << selected << " (index=" << selectedDbIndex << ", dbName=" << connInfo.dbName << ")"
              << std::endl;
    return selected;
}

std::string ConnectionPanel::getSelectedDatabase() const
{
    if (selectedDbIndex >= 0 && selectedDbIndex < databases.size())
    {
        std::cout << "Returning selected database: " << databases[selectedDbIndex] << std::endl;
        return databases[selectedDbIndex];
    }
    return "";
}

DatabaseConnectionInfo ConnectionPanel::getConnectionInfo() const
{
    return connInfo;
}

bool ConnectionPanel::handleConnect()
{
    if (queryPanel && queryPanel->isExportDialogActive())
        return false;

    return GuiButton(connectBtn, state == ConnectionState::DISCONNECTED ? "Connect to Server" : "Disconnect");
}

bool ConnectionPanel::handleExit() const
{
    return GuiButton(exitBtn, "Exit");
}

void ConnectionPanel::renderDatabaseSelection()
{
    if (databaseList && !importDialog.isVisible())
    {
        std::vector<ScrollableList::ListItem> items;

        for (size_t i = 0; i < databases.size(); i++)
            items.push_back({databases[i], static_cast<int>(i) == selectedDbIndex, false});

        databaseList->render(items);
    }
}

void ConnectionPanel::initializeDatabaseList(float startX, float startY)
{
    databaseList = std::make_unique<ScrollableList>("Available Databases",
                                                    35,                 
                                                    120,               
                                                    230,              
                                                    screenHeight - 450,
                                                    32                 
    );

    databaseList->setCallbacks([this](const std::string& dbName) {
        selectedDbIndex = std::find(databases.begin(), databases.end(), dbName) - databases.begin();
        strcpy(connInfo.dbName, dbName.c_str());
        state = ConnectionState::DATABASE_CONNECTED;
    });
}

void ConnectionPanel::setAvailableDatabases(const std::vector<std::string>& dbs)
{
    databases = dbs;
    updateDatabaseList();
}

void ConnectionPanel::updateDatabaseList()
{
    std::vector<ScrollableList::ListItem> items;

    for (const auto& db : databases)
        items.push_back({db, false, false});

    if (databaseList)
        databaseList->render(items);
}

void ConnectionPanel::renderConnectionStatus()
{
    Rectangle statusRect = drawStatusContainer();
    drawConnectionIndicator(statusRect);
    drawConnectionText(statusRect);
}

Rectangle ConnectionPanel::drawStatusContainer()
{
    Rectangle statusRect = {35, 60, 270, 40};

    DrawRectangleRec(statusRect, RAYWHITE);
    DrawRectangleLinesEx(statusRect, 1, Color{200, 200, 200, 255});

    return statusRect;
}

void ConnectionPanel::drawConnectionIndicator(Rectangle statusRect)
{
    DrawCircle(statusRect.x + 15, statusRect.y + statusRect.height / 2, 4, Color{0, 180, 0, 255});
}

void ConnectionPanel::drawConnectionText(Rectangle statusRect)
{
    float maxTextWidth = statusRect.width - 45; // Account for circle and padding
    std::string fullText = "Connected to: " + std::string(connInfo.dbName);
    int textWidth = MeasureText(fullText.c_str(), 16);

    if (textWidth > maxTextWidth)
    {
        drawTruncatedText(statusRect, fullText, maxTextWidth);
        handleStatusTooltip(statusRect, fullText);
    }
    else
    {
        drawFullText(statusRect, fullText);
    }
}

void ConnectionPanel::drawTruncatedText(Rectangle statusRect, const std::string& fullText, float maxTextWidth)
{
    std::string truncated = getTruncatedText(fullText, maxTextWidth);

    DrawText(truncated.c_str(), statusRect.x + 30, statusRect.y + (statusRect.height - 16) / 2, 16, Color{70, 70, 70, 255});
}

std::string ConnectionPanel::getTruncatedText(const std::string& text, float maxWidth)
{
    std::string truncated = text;
    int textWidth = MeasureText(truncated.c_str(), 16);

    while (textWidth > maxWidth - MeasureText("...", 16))
    {
        truncated = truncated.substr(0, truncated.length() - 1);
        textWidth = MeasureText(truncated.c_str(), 16);
    }

    return truncated + "...";
}

void ConnectionPanel::handleStatusTooltip(Rectangle statusRect, const std::string& fullText)
{
    if (CheckCollisionPointRec(GetMousePosition(), statusRect))
    {
        Rectangle tooltipRect = calculateTooltipRect(statusRect, fullText);
        drawStatusTooltip(tooltipRect, fullText);
    }
}

Rectangle ConnectionPanel::calculateTooltipRect(Rectangle statusRect, const std::string& text)
{
    float tooltipWidth = MeasureText(text.c_str(), 14) + 20;
    float tooltipX = statusRect.x;
    float tooltipY = statusRect.y + statusRect.height + 5;

    if (tooltipX + tooltipWidth > GetScreenWidth())
    {
        tooltipX = GetScreenWidth() - tooltipWidth - 10;
    }

    return Rectangle{tooltipX, tooltipY, tooltipWidth, 25};
}

void ConnectionPanel::drawStatusTooltip(Rectangle tooltipRect, const std::string& text)
{
    DrawRectangleRounded(tooltipRect, 0.2f, 4, Color{50, 50, 50, 230});
    DrawText(text.c_str(), tooltipRect.x + 10, tooltipRect.y + 5, 14, WHITE);
}

void ConnectionPanel::drawFullText(Rectangle statusRect, const std::string& text)
{
    DrawText(text.c_str(), statusRect.x + 30, statusRect.y + (statusRect.height - 16) / 2, 16, Color{70, 70, 70, 255});
}

bool ConnectionPanel::shouldImportDatabase()
{
    Vector2 mousePos = GetMousePosition();
    bool isOverButton = CheckCollisionPointRec(mousePos, importDatabaseBtn);
    bool isClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    std::cout << "\nImport button check:" << std::endl;
    std::cout << "Mouse position: " << mousePos.x << ", " << mousePos.y << std::endl;
    std::cout << "Button bounds: " << importDatabaseBtn.x << ", " << importDatabaseBtn.y << ", " << importDatabaseBtn.width << ", "
              << importDatabaseBtn.height << std::endl;
    std::cout << "Over button: " << isOverButton << std::endl;
    std::cout << "Mouse clicked: " << isClicked << std::endl;
    std::cout << "Current state: " << static_cast<int>(state) << std::endl;
    std::cout << "Export dialog active: " << (queryPanel && queryPanel->isExportDialogActive()) << std::endl;

    if (state == ConnectionState::SERVER_CONNECTED && !queryPanel->isExportDialogActive() && isOverButton && isClicked)
    {
        std::cout << "Import button clicked - showing dialog" << std::endl;
        importDialog.show();
        return false; 
    }
    return false;
}

bool ConnectionPanel::isExportDialogActive() const
{
    return queryPanel && queryPanel->isExportDialogActive();
}

bool ConnectionPanel::isImportDialogVisible() const
{
    return importDialog.isVisible();
}
