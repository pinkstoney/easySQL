#include "ImportDialog.h"

#include <cstring>
#include <iostream>

#include "../include/raygui.h"

ImportDialog::ImportDialog()
    : Dialog("Import Database", 450, 200) 
{
    resetState();
}

void ImportDialog::render()
{
    if (!visible)
    {
        std::cout << "ImportDialog not visible, skipping render" << std::endl;
        return;
    }

    std::cout << "Rendering ImportDialog" << std::endl;
    Dialog::render();

    Rectangle contentBounds = calculateDialogBounds();
    renderInputFields();
    renderButtons();
}

void ImportDialog::renderInputFields()
{
    if (!visible)
        return;

    Rectangle contentBounds = calculateDialogBounds();
    float inputWidth = contentBounds.width - PADDING * 2;
    float inputHeight = 30;
    float labelHeight = 15;

    float y = contentBounds.y + 10;
    DrawText("SQL File Path:", contentBounds.x, y, 14, DARKGRAY);
    y += labelHeight;

    DrawRectangleRec(Rectangle{contentBounds.x, y, inputWidth, inputHeight}, WHITE);

    if (GuiTextBox(Rectangle{contentBounds.x, y, inputWidth, inputHeight}, path, 512, pathActive))
        pathActive = !pathActive;
}

void ImportDialog::renderButtons()
{
    if (!visible)
        return;

    float buttonWidth = 80;
    float buttonHeight = 30;
    float spacing = 10;
    float bottomPadding = 15;

    Rectangle importBtnRect = {bounds.x + bounds.width - buttonWidth * 2 - spacing - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    Rectangle cancelBtnRect = {bounds.x + bounds.width - buttonWidth - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    bool importHovered = CheckCollisionPointRec(GetMousePosition(), importBtnRect);
    bool cancelHovered = CheckCollisionPointRec(GetMousePosition(), cancelBtnRect);

    DrawRectangleRounded(importBtnRect, 0.2f, 8, importHovered ? Color{0, 100, 180, 255} : Color{0, 120, 210, 255});
    DrawRectangleRounded(cancelBtnRect, 0.2f, 8, cancelHovered ? Color{180, 180, 180, 255} : Color{200, 200, 200, 255});

    DrawText("Import", importBtnRect.x + (importBtnRect.width - MeasureText("Import", 18)) / 2, importBtnRect.y + 7, 18, WHITE);
    DrawText("Cancel", cancelBtnRect.x + (cancelBtnRect.width - MeasureText("Cancel", 18)) / 2, cancelBtnRect.y + 7, 18, DARKGRAY);

    handleInput();
}

void ImportDialog::handleInput()
{
    if (!visible)
        return;

    if (IsKeyPressed(KEY_ESCAPE))
    {
        hide();
        return;
    }

    Vector2 mousePos = GetMousePosition();
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        return;

    float buttonWidth = 80;
    float buttonHeight = 30;
    float spacing = 10;
    float bottomPadding = 15;

    Rectangle importBtnRect = {bounds.x + bounds.width - buttonWidth * 2 - spacing - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    Rectangle cancelBtnRect = {bounds.x + bounds.width - buttonWidth - PADDING,
                               bounds.y + bounds.height - buttonHeight - bottomPadding, buttonWidth, buttonHeight};

    if (CheckCollisionPointRec(mousePos, importBtnRect))
    {
        std::cout << "Import button clicked in dialog" << std::endl;

        if (onImport)
            onImport(path);

        hide();
    }
    else if (CheckCollisionPointRec(mousePos, cancelBtnRect))
    {
        std::cout << "Cancel button clicked in dialog" << std::endl;
        hide();
    }
}

void ImportDialog::show()
{
    std::cout << "ImportDialog::show called" << std::endl;
    Dialog::show(); 
    visible = true;
    resetState();
    std::cout << "Dialog visibility after show: " << visible << std::endl;
}

void ImportDialog::hide()
{
    std::cout << "ImportDialog::hide called" << std::endl;
    visible = false;
    resetState();
}

void ImportDialog::resetState()
{
    strcpy(path, "imports/");
    pathActive = false;
}
