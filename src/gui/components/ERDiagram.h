#pragma once

#include "diagram/DiagramInteractionHandler.h"
#include "diagram/DiagramTypes.h"
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

class ERDiagram
{
public:
    using TableColumn = Diagram::TableColumn;
    using TableNode = Diagram::TableNode;
    using Relationship = Diagram::Relationship;

public:
    ERDiagram(float x, float y, float width, float height);

public:
    void render();
    void update();
    void addTable(const std::string& tableName, const std::vector<TableColumn>& columns);
    void clear();

public:
    void setVisible(bool visible) { isVisible = visible; }
    bool getVisible() const { return isVisible; }

private:
    void renderTables();
    void renderRelationships();
    void autoLayoutTables();

private:
    Rectangle bounds;
    std::unordered_map<std::string, TableNode> tables;
    std::vector<Relationship> relationships;
    float zoom = 1.0f;
    Vector2 pan = {0, 0};
    bool isPanning = false;
    bool isVisible = false;

private:
    std::unique_ptr<DiagramInteractionHandler> interactionHandler;
    static constexpr float MIN_TABLE_WIDTH = Diagram::MIN_TABLE_WIDTH;
};
