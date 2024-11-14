#pragma once
#include "Dialog.h"
#include <functional>

class ImportDialog : public Dialog {
public:
    ImportDialog();
    void handleInput() override;
    void render() override;
    void hide() override;
    void show() override;

    using ImportCallback = std::function<void(const std::string&)>;
    void setImportCallback(ImportCallback callback) { onImport = callback; }

private:
    void renderInputFields();
    void renderButtons();
    void resetState();

    char path[512] = "imports/";
    bool pathActive = false;

    ImportCallback onImport;
}; 