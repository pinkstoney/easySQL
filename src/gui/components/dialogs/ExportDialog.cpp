#include "ExportDialog.h"

#include <cstring>
#include <iostream>

#include "../include/raygui.h"

ExportDialog::ExportDialog()
    : Dialog("Export Database", 450, 250)
{
    resetState();
}

void ExportDialog::render()
{
    if (!visible)
    {
        std::cout << "ExportDialog::render - dialog not visible" << std::endl;
        return;
    }

    std::cout << "ExportDialog::render - rendering dialog" << std::endl;
    Dialog::render(); 

    Rectangle contentBounds = calculateDialogBounds();
    renderInputFields();
    renderButtons();
}

void ExportDialog::show()
{
    std::cout << "ExportDialog::show called" << std::endl;
    visible = true;
    std::cout << "Visibility after show: " << visible << std::endl;
}

void ExportDialog::showWithDatabase(const std::string& dbName)
{
    strcpy(filename, (dbName + ".sql").c_str());
    visible = true;
}

void ExportDialog::hide()
{
    std::cout << "ExportDialog::hide called" << std::endl;
    visible = false;
    resetState();
    std::cout << "Visibility after hide: " << visible << std::endl;
}

void ExportDialog::renderInputFields()
{
    if (!visible)
        return;

    Rectangle contentBounds = calculateDialogBounds();
    float inputWidth = contentBounds.width - PADDING * 2;
    float inputHeight = 30;
    float labelHeight = 15;
    float spacing = 20;

    float y = contentBounds.y + 10;
    DrawText("Filename:", contentBounds.x, y, 14, DARKGRAY);
    y += labelHeight;

    DrawRectangleRec(Rectangle{contentBounds.x, y, inputWidth, inputHeight}, WHITE);
    if (GuiTextBox(Rectangle{contentBounds.x, y, inputWidth, inputHeight}, filename, 256, filenameActive))
    {
        filenameActive = !filenameActive;
        pathActive = false;
    }

    y += inputHeight + spacing;
    DrawText("Path:", contentBounds.x, y, 14, DARKGRAY);
    y += labelHeight;

    DrawRectangleRec(Rectangle{contentBounds.x, y, inputWidth, inputHeight}, WHITE);
    if (GuiTextBox(Rectangle{contentBounds.x, y, inputWidth, inputHeight}, path, 512, pathActive))
    {
        pathActive = !pathActive;
        filenameActive = false;
    }
}

void ExportDialog::renderButtons()
{
    if (!visible)
        return;

    float buttonWidth = 80;
    float buttonHeight = 30;
    float spacing = 10;
    float bottomPadding = 15;

    Rectangle exportBtnRect = {bounds.x + bounds.width - buttonWidth * 2 - spacing - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    Rectangle cancelBtnRect = {bounds.x + bounds.width - buttonWidth - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    bool exportHovered = CheckCollisionPointRec(GetMousePosition(), exportBtnRect);
    bool cancelHovered = CheckCollisionPointRec(GetMousePosition(), cancelBtnRect);

    DrawRectangleRounded(exportBtnRect, 0.2f, 8, exportHovered ? Color{0, 100, 180, 255} : Color{0, 120, 210, 255});
    DrawRectangleRounded(cancelBtnRect, 0.2f, 8, cancelHovered ? Color{180, 180, 180, 255} : Color{200, 200, 200, 255});

    DrawText("Export", exportBtnRect.x + (exportBtnRect.width - MeasureText("Export", 18)) / 2, exportBtnRect.y + 7, 18, WHITE);
    DrawText("Cancel", cancelBtnRect.x + (cancelBtnRect.width - MeasureText("Cancel", 18)) / 2, cancelBtnRect.y + 7, 18, DARKGRAY);

    handleInput();
}

void ExportDialog::handleInput()
{
    if (!visible)
    {
        std::cout << "ExportDialog::handleInput - dialog not visible" << std::endl;
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        std::cout << "ExportDialog::handleInput - ESC pressed" << std::endl;
        hide();
        return;
    }

    Vector2 mousePos = GetMousePosition();
    float buttonWidth = 80;
    float buttonHeight = 30;
    float spacing = 10;
    float bottomPadding = 30;

    Rectangle exportBtnRect = {bounds.x + bounds.width - buttonWidth * 2 - spacing - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    Rectangle cancelBtnRect = {bounds.x + bounds.width - buttonWidth - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    std::cout << "\n=== Export Button Debug ===" << std::endl;
    std::cout << "Mouse position: " << mousePos.x << ", " << mousePos.y << std::endl;
    std::cout << "Export button bounds: " << exportBtnRect.x << ", " << exportBtnRect.y << ", " << exportBtnRect.width << ", "
              << exportBtnRect.height << std::endl;

    bool overExport = CheckCollisionPointRec(mousePos, exportBtnRect);
    bool overCancel = CheckCollisionPointRec(mousePos, cancelBtnRect);
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    std::cout << "Over export button: " << overExport << std::endl;
    std::cout << "Over cancel button: " << overCancel << std::endl;
    std::cout << "Mouse clicked: " << clicked << std::endl;
    std::cout << "Export callback set: " << (onExport ? "yes" : "no") << std::endl;

    if (clicked)
    {
        if (overExport)
        {
            std::cout << "Export button clicked" << std::endl;
            if (onExport)
            {
                std::cout << "Calling export callback with:" << std::endl;
                std::cout << "Path: " << path << std::endl;
                std::cout << "Filename: " << filename << std::endl;
                onExport(path, filename);
            }
            else
            {
                std::cout << "Warning: Export callback is not set!" << std::endl;
            }
            hide();
        }
        else if (overCancel)
        {
            std::cout << "Cancel button clicked" << std::endl;
            hide();
        }
    }
}

void ExportDialog::resetState()
{
    strcpy(filename, "database.sql");
    strcpy(path, "exports/");

    filenameActive = false;
    pathActive = false;
}
