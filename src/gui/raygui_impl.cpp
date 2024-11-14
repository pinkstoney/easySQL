#include <raylib.h>
#include <iostream>

// Add this before including raygui.h
static float TextToFloat(const char* text)
{
    return static_cast<float>(std::atof(text));
}

#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
