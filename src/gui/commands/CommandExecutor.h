#pragma once

#include "DatabaseCommand.h"

#include <memory>
#include <vector>

class CommandExecutor
{
public:
    static void executeCommands(std::vector<std::unique_ptr<DatabaseCommand>>& commands);
};
