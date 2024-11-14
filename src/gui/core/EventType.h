#pragma once

enum class EventType
{
    ServerConnected,
    ServerDisconnected,
    DatabaseConnected,
    DatabaseDisconnected,
    QueryExecuted,
    QueryFailed,
    TablesLoaded,
    ExportCompleted,
    ExportFailed,
    ErrorOccurred,
    StateReset,
    TableStructureLoaded,
    ImportCompleted
};
