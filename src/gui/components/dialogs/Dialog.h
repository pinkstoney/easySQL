#pragma once

#include <string>

#include <raylib.h>

class Dialog
{
public:
    Dialog(const std::string& title, float width, float height);
    virtual ~Dialog() = default;

public:
    virtual void render();
    virtual void handleInput() = 0;

    bool isVisible() const { return visible; }
    virtual void show() { visible = true; }
    virtual void hide() { visible = false; }

protected:
    void renderOverlay();
    void renderDialog();
    void renderTitle();

    Rectangle calculateDialogBounds() const;

protected:
    std::string title;
    Rectangle bounds;
    bool visible = false;

protected:
    static constexpr float TITLE_HEIGHT = 40.0f;
    static constexpr float PADDING = 20.0f;
};
