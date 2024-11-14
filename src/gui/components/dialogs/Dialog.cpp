#include "Dialog.h"

Dialog::Dialog(const std::string& title, float width, float height)
    : title(title)
{
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    bounds = {(screenWidth - width) / 2, (screenHeight - height) / 2, width, height};
}

void Dialog::render()
{
    if (!visible)
        return;

    bounds.x = (GetScreenWidth() - bounds.width) / 2;
    bounds.y = (GetScreenHeight() - bounds.height) / 2;

    renderOverlay();
    renderDialog();
    renderTitle();
}

void Dialog::renderOverlay()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 100});
}

void Dialog::renderDialog()
{
    DrawRectangleRounded(bounds, 0.1f, 8, WHITE);
}

void Dialog::renderTitle()
{
    DrawText(title.c_str(), bounds.x + PADDING, bounds.y + PADDING, 20, DARKGRAY);
}

Rectangle Dialog::calculateDialogBounds() const
{
    return Rectangle{bounds.x + PADDING, bounds.y + TITLE_HEIGHT + PADDING, bounds.width - (PADDING * 2),
                     bounds.height - TITLE_HEIGHT - (PADDING * 2)};
}
