#pragma once

#include "DiagramTypes.h"

#include <raylib.h>

class RelationshipRenderer
{
public:
    static void render(const Diagram::TableNode& from, const Diagram::TableNode& to, const std::string& fromCol,
                       const std::string& toCol, const Rectangle& bounds, const Vector2& pan, float zoom);

private:
    static void drawArrow(const Vector2& start, const Vector2& end);
    static Vector2 calculateBezierPoint(const Vector2& start, const Vector2& end, float t);
    static float calculateDistance(const Vector2& p1, const Vector2& p2);
    static Vector2 findColumnPosition(const Diagram::TableNode& table, const std::string& columnName, float& yOffset);

private:
    static constexpr float ARROW_SIZE = 8.0f;
    static constexpr int BEZIER_SEGMENTS = 20;
    static constexpr float TABLE_HEADER_HEIGHT = Diagram::TABLE_HEADER_HEIGHT;
    static constexpr float ROW_HEIGHT = Diagram::ROW_HEIGHT;
};
