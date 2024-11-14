#include "ApplicationState.h"

#include "../GuiManager.h"
#include "../commands/CommandFactory.h"
#include "../core/export/DatabaseExporter.h"

#include <iostream>

void DisconnectedState::render(GuiManager& manager)
{
    manager.getConnectionPanel()->render();
}

void DisconnectedState::handleInput(GuiManager& manager)
{
    auto panel = manager.getConnectionPanel();
    if (panel->handleConnect())
    {
        if (panel->getState() == ConnectionPanel::ConnectionState::DISCONNECTED)
        {
            auto command = std::make_unique<ConnectToServerCommand>(manager.getDatabaseManager(), panel->getConnectionInfo(), *panel);
            manager.addCommand(std::move(command));
        }
    }
}

void ConnectedState::render(GuiManager& manager)
{
    manager.getConnectionPanel()->render();
}

void ConnectedState::handleInput(GuiManager& manager)
{
    auto panel = manager.getConnectionPanel();

    if (panel->handleConnect())
    {
        auto command =
            std::make_unique<DisconnectCommand>(manager.getDatabaseManager(), manager.getTableManager(), manager.getQuery(), manager);
        manager.addCommand(std::move(command));
        return;
    }

    panel->shouldImportDatabase();

    if (panel->getState() == ConnectionPanel::ConnectionState::SERVER_CONNECTED && panel->isDatabaseSelected())
    {

        std::string selectedDb = panel->getSelectedDatabase();
        std::cout << "Selected database: " << selectedDb << std::endl;

        if (!selectedDb.empty())
        {
            std::cout << "Initiating database connection..." << std::endl;

            auto command = std::make_unique<ConnectToDatabaseCommand>(manager.getDatabaseManager(), manager.getTableManager(),
                                                                      manager.getQuery(), selectedDb);

            manager.addCommand(std::move(command));
            manager.executePendingCommands();

            if (manager.verifyDatabaseConnection())
            {
                std::cout << "Database connection successful, all objects initialized" << std::endl;
                manager.setConnected(true);
            }
            else
            {
                std::cout << "Failed to initialize database connection objects" << std::endl;
                manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Failed to initialize database connection", true});

                manager.resetDatabaseManager();
                manager.resetTableManager();
                manager.resetQuery();
                manager.setConnected(false);
            }
        }
    }
}

void QueryState::render(GuiManager& manager)
{
    manager.getConnectionPanel()->render();
    manager.getQueryPanel()->render();
    manager.getResultsPanel()->render();
}

void QueryState::handleInput(GuiManager& manager)
{
    auto panel = manager.getConnectionPanel();
    if (panel->handleConnect())
    {
        auto command =
            std::make_unique<DisconnectCommand>(manager.getDatabaseManager(), manager.getTableManager(), manager.getQuery(), manager);
        manager.addCommand(std::move(command));
        return;
    }

    auto queryPanel = manager.getQueryPanel();
    if (queryPanel->shouldExecuteQuery())
    {
        if (!manager.verifyDatabaseConnection())
        {
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Database connection not properly initialized", true});
            std::cout << "Database verification failed in QueryState" << std::endl;
            return;
        }

        try
        {
            std::string queryText = queryPanel->getQueryText();
            if (queryText.empty())
            {
                manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Query cannot be empty", false});
                return;
            }

            auto command =
                CommandFactory::createQueryCommand(manager.getQuery().get(), queryText, manager.getLatestQueryResult().data);
            manager.addCommand(std::move(command));
        }
        catch (const std::exception& e)
        {
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{std::string("Query error: ") + e.what(), true});
        }
    }

    if (queryPanel->shouldShowObjects())
    {
        try
        {
            auto& latestTableResult = manager.getLatestTableResult();
            bool newVisibility = !latestTableResult.isVisible;

            latestTableResult.isVisible = newVisibility;
            queryPanel->setObjectsVisibility(newVisibility);

            if (newVisibility)
            {
                auto command = std::make_unique<LoadTablesCommand>(manager.getTableManager(), latestTableResult.tableNames);
                manager.addCommand(std::move(command));
            }
            else
            {
                latestTableResult.tableNames.clear();
                manager.publishEvent(EventType::TablesLoaded,
                                     TablesLoadedData{std::vector<std::string>(), std::vector<std::string>(),
                                                      std::vector<std::string>(), std::vector<std::string>(), false});
            }
        }
        catch (const std::exception& e)
        {
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{e.what(), true});
            auto& latestTableResult = manager.getLatestTableResult();
            latestTableResult.isVisible = false;
            queryPanel->setObjectsVisibility(false);
        }
    }

    if (queryPanel->shouldShowERDiagram())
    {
        try
        {
            std::cout << "Handling ER Diagram toggle in ApplicationState" << std::endl;
            bool newVisibility = !manager.getERDiagram()->getVisible();
            std::cout << "Setting new visibility to: " << newVisibility << std::endl;

            queryPanel->setERDiagramVisibility(newVisibility);
            manager.getERDiagram()->setVisible(newVisibility);

            if (newVisibility)
            {
                std::cout << "Loading database structure..." << std::endl;
                auto tableManager = manager.getTableManager().get();

                if (!tableManager)
                    throw std::runtime_error("Table manager not initialized");

                manager.getERDiagram()->clear();
                auto tables = tableManager->getTableNames();
                std::cout << "Found " << tables.size() << " tables" << std::endl;

                for (const auto& tableName : tables)
                {
                    auto structure = tableManager->getTableStructure(tableName);

                    std::vector<ERDiagram::TableColumn> columns;
                    for (const auto& [colName, colType] : structure)
                    {
                        std::string query = "SELECT REFERENCED_TABLE_NAME, REFERENCED_COLUMN_NAME "
                                            "FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                                            "WHERE TABLE_SCHEMA = DATABASE() "
                                            "AND TABLE_NAME = '" +
                                            tableName +
                                            "' "
                                            "AND COLUMN_NAME = '" +
                                            colName +
                                            "' "
                                            "AND REFERENCED_TABLE_NAME IS NOT NULL";

                        auto result = manager.getQuery()->execute(query);

                        bool isForeignKey = !result.rows.empty();
                        std::string refTable, refColumn;
                        if (isForeignKey)
                        {
                            refTable = result.rows[0][0];
                            refColumn = result.rows[0][1];
                        }

                        query = "SELECT COUNT(*) "
                                "FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                                "WHERE TABLE_SCHEMA = DATABASE() "
                                "AND TABLE_NAME = '" +
                                tableName +
                                "' "
                                "AND COLUMN_NAME = '" +
                                colName +
                                "' "
                                "AND CONSTRAINT_NAME = 'PRIMARY'";

                        result = manager.getQuery()->execute(query);
                        bool isPrimaryKey = !result.rows.empty() && result.rows[0][0] != "0";

                        columns.push_back({colName, colType, isPrimaryKey, isForeignKey, refTable, refColumn});
                    }

                    manager.getERDiagram()->addTable(tableName, columns);
                }
            }
            else
            {
                std::cout << "Clearing ER Diagram" << std::endl;
                manager.getERDiagram()->clear();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "Error in ER Diagram handling: " << e.what() << std::endl;
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{e.what(), true});
            queryPanel->setERDiagramVisibility(false);
            manager.getERDiagram()->setVisible(false);
        }
    }

    if (queryPanel->shouldExportDatabase())
    {
        try
        {
            std::string filename = "exports/database_" + std::to_string(std::time(nullptr)) + ".sql";

            if (DatabaseExporter::exportToSQL(manager.getDatabaseManager().get(), manager.getTableManager().get(), filename))
                manager.publishEvent(EventType::ExportCompleted, ErrorData{"Database exported to " + filename, false});
            else
                throw std::runtime_error("Failed to export database");
        }
        catch (const std::exception& e)
        {
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Export failed: " + std::string(e.what()), true});
        }
    }

    if (queryPanel->shouldImportDatabase())
    {
        try
        {
            std::string filename = "imports/database.sql";

            if (DatabaseExporter::importFromSQL(manager.getDatabaseManager().get(), filename))
                manager.publishEvent(EventType::ExportCompleted, ErrorData{"Database imported from " + filename, false});
            else
                throw std::runtime_error("Failed to import database");
        }
        catch (const std::exception& e)
        {
            manager.publishEvent(EventType::ErrorOccurred, ErrorData{"Import failed: " + std::string(e.what()), true});
        }
    }
}
