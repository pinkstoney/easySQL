#include "DiagramInteractionHandler.h"

#include <algorithm>
#include <iostream>

DiagramInteractionHandler::DiagramInteractionHandler(Rectangle bounds)
    : bounds(bounds)
{
}

void DiagramInteractionHandler::handleZooming(float& zoom)
{
    if (CheckCollisionPointRec(GetMousePosition(), bounds))
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            zoom = std::clamp(zoom + wheel * ZOOM_STEP, MIN_ZOOM, MAX_ZOOM);
            std::cout << "Zoom adjusted to: " << zoom << std::endl;
        }
    }
}

void DiagramInteractionHandler::handlePanning(Vector2& pan, bool& isPanning)
{
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON))
    {
        isPanning = true;
        std::cout << "Started panning" << std::endl;
    }
    else if (IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON))
    {
        isPanning = false;
        std::cout << "Stopped panning" << std::endl;
    }

    if (isPanning)
    {
        Vector2 delta = GetMouseDelta();
        pan.x += delta.x;
        pan.y += delta.y;
        std::cout << "Panning: " << pan.x << ", " << pan.y << std::endl;
    }
}

void DiagramInteractionHandler::handleTableDragging(std::unordered_map<std::string, Diagram::TableNode>& tables, const Vector2& pan,
                                                    float zoom)
{

    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (auto& [name, table] : tables)
        {
            Rectangle tableRect = {bounds.x + table.position.x * zoom + pan.x, bounds.y + table.position.y * zoom + pan.y,
                                   table.size.x * zoom, table.size.y * zoom};

            if (CheckCollisionPointRec(mousePos, tableRect))
            {
                table.isSelected = true;
                table.isDragging = true;
                std::cout << "Started dragging table: " << name << std::endl;
                break;
            }
        }
    }
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        for (auto& [name, table] : tables)
        {
            if (table.isDragging)
                std::cout << "Stopped dragging table: " << name << std::endl;

            table.isDragging = false;
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        Vector2 delta = GetMouseDelta();
        for (auto& [name, table] : tables)
        {
            if (table.isDragging)
            {
                table.position.x += delta.x / zoom;
                table.position.y += delta.y / zoom;
                std::cout << "Dragging table " << name << " to: " << table.position.x << ", " << table.position.y << std::endl;
            }
        }
    }
}
