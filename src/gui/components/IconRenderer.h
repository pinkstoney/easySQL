#pragma once

#include <raylib.h>

class IconRenderer
{
public:
    static void drawStructureIcon(const Rectangle& btnRect, Color color);
    static void drawExecuteIcon(const Rectangle& btnRect, Color color);

private:
    static constexpr float DEFAULT_PADDING = 4.0f;
    static constexpr float DEFAULT_LINE_THICKNESS = 2.0f;
};
