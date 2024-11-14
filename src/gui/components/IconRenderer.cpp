#include "IconRenderer.h"

void IconRenderer::drawStructureIcon(const Rectangle& btnRect, Color color) {
    float padding = DEFAULT_PADDING;
    float iconSize = btnRect.height - (padding * 2);
    float startX = btnRect.x + padding;
    float startY = btnRect.y + padding;
    
    // Draw three horizontal lines to represent table structure
    float lineSpacing = iconSize / 4;
    float lineWidth = iconSize - padding;
    
    // Draw main line (thicker)
    DrawLineEx(
        Vector2{startX, startY + lineSpacing},
        Vector2{startX + lineWidth, startY + lineSpacing},
        DEFAULT_LINE_THICKNESS, color
    );
    
    // Draw two thinner lines below
    for(int i = 1; i < 3; i++) {
        DrawLineEx(
            Vector2{startX, startY + lineSpacing * (i + 1)},
            Vector2{startX + lineWidth * 0.8f, startY + lineSpacing * (i + 1)},
            1.0f, color
        );
    }
}

void IconRenderer::drawExecuteIcon(const Rectangle& btnRect, Color color) {
    float padding = DEFAULT_PADDING;
    float iconSize = btnRect.height - (padding * 2);  // Made icon bigger
    
    // Calculate center of button
    float centerX = btnRect.x + (btnRect.width / 2.0f);
    float centerY = btnRect.y + (btnRect.height / 2.0f);
    
    // Calculate triangle points for a more pleasing shape
    Vector2 points[3] = {
        {centerX - iconSize/3, centerY - iconSize/2.2f},    // Left point
        {centerX + iconSize/2.2f, centerY},                 // Right point
        {centerX - iconSize/3, centerY + iconSize/2.2f}     // Bottom point
    };
    
    // Draw filled triangle with slightly darker color for better visibility
    Color fillColor = color;
    fillColor.r = (color.r > 30) ? color.r - 30 : 0;
    fillColor.g = (color.g > 30) ? color.g - 30 : 0;
    fillColor.b = (color.b > 30) ? color.b - 30 : 0;
    DrawTriangle(points[0], points[1], points[2], fillColor);
    
    // Draw outline for better definition
    DrawTriangleLines(points[0], points[1], points[2], color);
    
    // Add subtle highlight for 3D effect
    Color highlightColor = ColorAlpha(WHITE, 0.4f);  // Made highlight more visible
    DrawLineEx(points[0], points[1], 2.0f, highlightColor);
    DrawLineEx(points[1], points[2], 2.0f, highlightColor);
} 