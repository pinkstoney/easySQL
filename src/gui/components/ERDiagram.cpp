#include "ERDiagram.h"

#include "diagram/RelationshipRenderer.h"
#include "diagram/TableRenderer.h"

#include <iostream>

ERDiagram::ERDiagram(float x, float y, float width, float height)
    : bounds({x, y, width, height})
    , interactionHandler(std::make_unique<DiagramInteractionHandler>(bounds))
{
}

void ERDiagram::render()
{
    if (!isVisible)
    {
        std::cout << "ERDiagram::render called but diagram is not visible" << std::endl;
        return;
    }

    std::cout << "Rendering ER Diagram with " << tables.size() << " tables and " << relationships.size() << " relationships"
              << std::endl;

    DrawRectangleRec(bounds, RAYWHITE);
    DrawRectangleLinesEx(bounds, 1, Color{200, 200, 200, 255});

    BeginScissorMode(bounds.x, bounds.y, bounds.width, bounds.height);

    renderRelationships();

    renderTables();

    EndScissorMode();

    char zoomText[64];
    snprintf(zoomText, sizeof(zoomText), "Zoom: %.0f%% (Mouse Wheel to zoom)", zoom * 100);
    DrawText(zoomText, bounds.x + 10, bounds.y + 10, 16, DARKGRAY);
    DrawText("Middle Mouse Button to pan", bounds.x + 10, bounds.y + 30, 16, DARKGRAY);
    DrawText("Left Mouse Button to drag tables", bounds.x + 10, bounds.y + 50, 16, DARKGRAY);
}

void ERDiagram::update()
{
    if (!isVisible)
        return;

    interactionHandler->handleZooming(zoom);
    interactionHandler->handlePanning(pan, isPanning);
    interactionHandler->handleTableDragging(tables, pan, zoom);
}

void ERDiagram::renderTables()
{
    for (const auto& [name, table] : tables)
        TableRenderer::render(table, bounds, pan, zoom);
}

void ERDiagram::renderRelationships()
{
    for (const auto& rel : relationships)
    {
        const auto& fromTable = tables[rel.fromTable];
        const auto& toTable = tables[rel.toTable];
        RelationshipRenderer::render(fromTable, toTable, rel.fromColumn, rel.toColumn, bounds, pan, zoom);
    }
}

void ERDiagram::addTable(const std::string& tableName, const std::vector<TableColumn>& columns)
{
    TableNode node;
    node.name = tableName;
    node.columns = columns;

    float width = MIN_TABLE_WIDTH * 1.2f; 
    float height = Diagram::TABLE_HEADER_HEIGHT + columns.size() * Diagram::ROW_HEIGHT;

    node.size = {width, height};

    node.position = {
        static_cast<float>(tables.size()) * (width + 70), 
        static_cast<float>(tables.size()) * 70           
    };

    tables[tableName] = node;

    for (const auto& column : columns)
    {
        if (column.isForeignKey && !column.referencedTable.empty())
            relationships.push_back({tableName, column.name, column.referencedTable, column.referencedColumn});
    }

    autoLayoutTables();
}

void ERDiagram::autoLayoutTables()
{
    const float SPACING_X = MIN_TABLE_WIDTH * 1.5f; 
    const float SPACING_Y = 250;                    
    const int TABLES_PER_ROW = 3;

    int i = 0;
    for (auto& [name, table] : tables)
    {
        int row = i / TABLES_PER_ROW;
        int col = i % TABLES_PER_ROW;

        table.position = {col * SPACING_X + 50, row * SPACING_Y + 50};

        i++;
    }
}

void ERDiagram::clear()
{
    tables.clear();
    relationships.clear();
    zoom = 1.0f;
    pan = {0, 0};
}
