#include "gui/GuiManager.h"

#include <mysqlx/xdevapi.h>

#include <iostream>

int main()
{
    try
    {
        const int screenWidth = 1280;
        const int screenHeight = 720;

        GuiManager gui(screenWidth, screenHeight);
        gui.initialize();

        bool shouldClose = false;

        LatestQueryResult latestResult;

        LatestTableResult latestTableResult;

        Message errorMsg = {"", true};

        while (!WindowShouldClose() && !shouldClose)
            gui.render(shouldClose, latestResult, latestTableResult, errorMsg);

        CloseWindow();
    }
    catch (const mysqlx::Error& err)
    {
        std::cerr << "MySQL Connector/C++ Error: " << err.what() << std::endl;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "STD Error: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error!" << std::endl;
        return 1;
    }

    return 0;
}
