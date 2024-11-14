#include "ScrollableList.h"

#include "IconRenderer.h"

ScrollableList::ScrollableList(const char* title, float x, float y, float width, float height, float itemHeight, float titleY)
    : title(title)
    , bounds({x, y, width, height})
    , itemHeight(itemHeight)
    , titleY(titleY == -1 ? y - 35 : titleY)
{
}

void ScrollableList::render(const std::vector<ListItem>& items)
{
    drawHeader();
    drawListBackground();

    float contentHeight = items.size() * itemHeight;
    handleScrolling(contentHeight);

    BeginScissorMode(bounds.x, bounds.y, bounds.width - SCROLLBAR_WIDTH, bounds.height);
    drawItems(items);
    EndScissorMode();

    DrawRectangleLinesEx(bounds, 1, Color{200, 200, 200, 255});
    drawScrollbar(contentHeight);
}

void ScrollableList::drawHeader()
{
    DrawText(title.c_str(), bounds.x, titleY, 18, Color{70, 70, 70, 255});
}

void ScrollableList::drawListBackground()
{
    DrawRectangleRec(Rectangle{bounds.x + 2, bounds.y + 2, bounds.width, bounds.height}, Color{200, 200, 200, 100});

    DrawRectangleRec(bounds, RAYWHITE);
}

void ScrollableList::drawItems(const std::vector<ListItem>& items)
{
    for (size_t i = 0; i < items.size(); i++)
    {
        float itemY = bounds.y + i * itemHeight - listScroll;

        if (itemY + itemHeight <= bounds.y || itemY >= bounds.y + bounds.height)
            continue;

        float itemWidth =
            items[i].hasActionButton ? bounds.width - ACTION_BUTTON_WIDTH - SCROLLBAR_WIDTH : bounds.width - SCROLLBAR_WIDTH;

        Rectangle itemRect = {bounds.x + 1, itemY, itemWidth, itemHeight};

        bool isHovered = CheckCollisionPointRec(GetMousePosition(), itemRect);
        drawItem(items[i], itemRect, isHovered);

        if (items[i].hasActionButton)
        {
            EndScissorMode();

            Rectangle actionBtn = {itemRect.x + itemRect.width, itemY, ACTION_BUTTON_WIDTH, itemHeight};

            bool isActionHovered = CheckCollisionPointRec(GetMousePosition(), actionBtn);
            Color btnBgColor = isActionHovered ? LIGHTGRAY : RAYWHITE;

            DrawRectangleRec(actionBtn, btnBgColor);
            DrawRectangleLinesEx(actionBtn, 1, isActionHovered ? DARKGRAY : GRAY);
            IconRenderer::drawStructureIcon(actionBtn, isActionHovered ? DARKBLUE : DARKGRAY);

            if (isActionHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && onActionButton)
                onActionButton(items[i].text);

            BeginScissorMode(bounds.x, bounds.y, bounds.width - SCROLLBAR_WIDTH, bounds.height);
        }

        if (i < items.size() - 1)
            DrawLine(bounds.x + 5, itemY + itemHeight, bounds.x + bounds.width - 5, itemY + itemHeight, Color{230, 230, 230, 255});
    }
}

void ScrollableList::drawItem(const ListItem& item, Rectangle itemRect, bool isHovered)
{
    Color bgColor = item.isSelected ? Color{230, 240, 255, 255} : isHovered ? Color{245, 245, 245, 255} : RAYWHITE;
    DrawRectangleRec(itemRect, bgColor);

    if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && onItemClick)
        onItemClick(item.text);

    int textWidth = MeasureText(item.text.c_str(), 16);
    float maxTextWidth = itemRect.width - 20;

    if (textWidth > maxTextWidth)
    {
        std::string truncated = item.text;

        while (MeasureText(truncated.c_str(), 16) > maxTextWidth - MeasureText("...", 16))
            truncated = truncated.substr(0, truncated.length() - 1);

        truncated += "...";

        DrawText(truncated.c_str(), itemRect.x + 10, itemRect.y + (itemHeight - 16) / 2, 16,
                 isHovered || item.isSelected ? DARKBLUE : BLACK);

        if (isHovered)
        {
            EndScissorMode();

            float tooltipWidth = MeasureText(item.text.c_str(), 14) + 20;
            float tooltipX = itemRect.x + itemRect.width + 10;
            float tooltipY = GetMousePosition().y - 15;

            if (tooltipX + tooltipWidth > GetScreenWidth())
                tooltipX = itemRect.x - tooltipWidth - 10;

            Rectangle tooltipRect = {tooltipX, tooltipY, tooltipWidth, 25};

            DrawRectangleRounded(tooltipRect, 0.2f, 4, Color{50, 50, 50, 230});
            DrawText(item.text.c_str(), tooltipRect.x + 10, tooltipRect.y + 5, 14, WHITE);

            BeginScissorMode(bounds.x, bounds.y, bounds.width - SCROLLBAR_WIDTH, bounds.height);
        }
    }
    else
    {
        DrawText(item.text.c_str(), itemRect.x + 10, itemRect.y + (itemHeight - 16) / 2, 16,
                 isHovered || item.isSelected ? DARKBLUE : BLACK);
    }
}

void ScrollableList::handleScrolling(float contentHeight)
{
    float maxScroll = std::max(0.0f, contentHeight - bounds.height);

    if (CheckCollisionPointRec(GetMousePosition(), bounds))
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            listScroll -= wheel * 20;
            listScroll = std::max(0.0f, std::min(listScroll, maxScroll));
        }
    }
}

void ScrollableList::drawScrollbar(float contentHeight)
{
    float maxScroll = std::max(0.0f, contentHeight - bounds.height);
    if (maxScroll > 0)
    {
        Rectangle scrollBarBg = {bounds.x + bounds.width - SCROLLBAR_WIDTH, bounds.y, SCROLLBAR_WIDTH, bounds.height};
        DrawRectangleRec(scrollBarBg, Color{245, 245, 245, 255});

        float visiblePortion = bounds.height / contentHeight;
        float scrollBarHeight = bounds.height * visiblePortion;
        float scrollBarY = bounds.y + (listScroll / maxScroll) * (bounds.height - scrollBarHeight);

        Rectangle scrollBarRect = {scrollBarBg.x, scrollBarY, SCROLLBAR_WIDTH, scrollBarHeight};

        bool isScrollBarHovered = CheckCollisionPointRec(GetMousePosition(), scrollBarRect);
        DrawRectangleRec(scrollBarRect, isScrollBarHovered ? Color{130, 130, 130, 255} : Color{180, 180, 180, 255});
    }
}

void ScrollableList::setCallbacks(ItemClickCallback onClick, ActionButtonCallback onAction)
{
    onItemClick = std::move(onClick);
    onActionButton = std::move(onAction);
}
