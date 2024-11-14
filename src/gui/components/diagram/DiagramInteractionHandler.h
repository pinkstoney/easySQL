#pragma once

#include "DiagramTypes.h"

#include <unordered_map>

#include <raylib.h>

class DiagramInteractionHandler
{
public:
    DiagramInteractionHandler(Rectangle bounds);

public:
    void handleZooming(float& zoom);
    void handlePanning(Vector2& pan, bool& isPanning);
    void handleTableDragging(std::unordered_map<std::string, Diagram::TableNode>& tables, const Vector2& pan, float zoom);

private:
    Rectangle bounds;
    static constexpr float MIN_ZOOM = 0.5f;
    static constexpr float MAX_ZOOM = 2.0f;
    static constexpr float ZOOM_STEP = 0.1f;
};
