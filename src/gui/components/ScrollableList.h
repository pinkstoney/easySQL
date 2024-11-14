#pragma once

#include <functional>
#include <string>
#include <vector>

#include <raylib.h>

class ScrollableList
{
public:
    enum class DatabaseObjectType
    {
        Table,
        View,
        StoredProcedure,
        Function
    };

    struct ListItem
    {
        std::string text;
        bool isSelected = false;
        bool hasActionButton = false;
        DatabaseObjectType type = DatabaseObjectType::Table;
    };

public:
    using ItemClickCallback = std::function<void(const std::string&)>;
    using ActionButtonCallback = std::function<void(const std::string&)>;

public:
    ScrollableList(const char* title, float x, float y, float width, float height, float itemHeight, float titleY = -1);

public:
    void render(const std::vector<ListItem>& items);

    void setCallbacks(ItemClickCallback onClick, ActionButtonCallback onAction = nullptr);
    void setScroll(float scroll) { listScroll = scroll; }
    float getScroll() const { return listScroll; }
    Rectangle getBounds() const { return bounds; }

private:
    void drawHeader();
    void drawListBackground();
    void drawItems(const std::vector<ListItem>& items);
    void drawItem(const ListItem& item, Rectangle itemRect, bool isHovered);
    void handleScrolling(float contentHeight);
    void drawScrollbar(float contentHeight);
    void drawTooltip(const std::string& text, Rectangle itemRect);

private:
    std::string title;
    Rectangle bounds;
    float itemHeight;
    float listScroll = 0.0f;

    ItemClickCallback onItemClick;
    ActionButtonCallback onActionButton;

    static constexpr float ACTION_BUTTON_WIDTH = 35.0f;
    static constexpr float SCROLLBAR_WIDTH = 8.0f;

    float titleY;
};
