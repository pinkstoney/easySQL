#include "RelationshipRenderer.h"

#include <cmath>

void RelationshipRenderer::render(const Diagram::TableNode& from, const Diagram::TableNode& to, const std::string& fromCol,
                                  const std::string& toCol, const Rectangle& bounds, const Vector2& pan, float zoom)
{
    float fromY = 0, toY = 0;

    Vector2 fromPos = findColumnPosition(from, fromCol, fromY);
    Vector2 toPos = findColumnPosition(to, toCol, toY);

    Vector2 start = {bounds.x + (fromPos.x + from.size.x) * zoom + pan.x, bounds.y + fromPos.y * zoom + pan.y};

    Vector2 end = {bounds.x + toPos.x * zoom + pan.x, bounds.y + toPos.y * zoom + pan.y};

    Vector2 previous = start;
    for (int i = 1; i <= BEZIER_SEGMENTS; i++)
    {
        float t = i / static_cast<float>(BEZIER_SEGMENTS);
        Vector2 current = calculateBezierPoint(start, end, t);
        DrawLineEx(previous, current, 2 * zoom, DARKGRAY);
        previous = current;
    }

    drawArrow(previous, end);
}

void RelationshipRenderer::drawArrow(const Vector2& start, const Vector2& end)
{
    Vector2 direction = {end.x - start.x, end.y - start.y};

    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    direction.x /= length;
    direction.y /= length;

    Vector2 perpendicular = {-direction.y, direction.x};

    Vector2 arrowPoint1 = {end.x - direction.x * ARROW_SIZE + perpendicular.x * ARROW_SIZE,
                           end.y - direction.y * ARROW_SIZE + perpendicular.y * ARROW_SIZE};

    Vector2 arrowPoint2 = {end.x - direction.x * ARROW_SIZE - perpendicular.x * ARROW_SIZE,
                           end.y - direction.y * ARROW_SIZE - perpendicular.y * ARROW_SIZE};

    DrawTriangle(arrowPoint1, end, arrowPoint2, DARKGRAY);
}

Vector2 RelationshipRenderer::calculateBezierPoint(const Vector2& start, const Vector2& end, float t)
{
    float distance = calculateDistance(start, end);
    Vector2 control1 = {start.x + distance * 0.25f, start.y};
    Vector2 control2 = {end.x - distance * 0.25f, end.y};

    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    return {uuu * start.x + 3 * uu * t * control1.x + 3 * u * tt * control2.x + ttt * end.x,
            uuu * start.y + 3 * uu * t * control1.y + 3 * u * tt * control2.y + ttt * end.y};
}

float RelationshipRenderer::calculateDistance(const Vector2& p1, const Vector2& p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return sqrt(dx * dx + dy * dy);
}

Vector2 RelationshipRenderer::findColumnPosition(const Diagram::TableNode& table, const std::string& columnName, float& yOffset)
{
    for (size_t i = 0; i < table.columns.size(); i++)
    {
        if (table.columns[i].name == columnName)
        {
            float y = (i + 1) * ROW_HEIGHT + TABLE_HEADER_HEIGHT - (ROW_HEIGHT / 2);
            return {table.position.x, table.position.y + y};
        }
    }
    return {table.position.x, table.position.y + yOffset};
}
