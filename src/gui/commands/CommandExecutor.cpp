#include "CommandExecutor.h"

#include <iostream>

void CommandExecutor::executeCommands(std::vector<std::unique_ptr<DatabaseCommand>>& commands)
{
    for (auto& command : commands)
    {
        try
        {
            if (command)
                command->execute();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Command execution failed: " << e.what() << std::endl;
            throw;
        }
    }
    commands.clear();
}
