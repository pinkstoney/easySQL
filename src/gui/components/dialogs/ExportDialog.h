#pragma once

#include "Dialog.h"

#include <functional>

class ExportDialog : public Dialog
{
public:
    ExportDialog();

public:
    void handleInput() override;
    void render() override;
    void hide() override;
    void show() override;

public:
    void showWithDatabase(const std::string& dbName);

    using ExportCallback = std::function<void(const std::string&, const std::string&)>;
    void setExportCallback(ExportCallback callback) { onExport = callback; }

private:
    void renderInputFields();
    void renderButtons();
    void resetState();

private:
    char filename[256] = "database.sql";
    char path[512] = "exports/";
    bool filenameActive = false;
    bool pathActive = false;

    ExportCallback onExport;
};
