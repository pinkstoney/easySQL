cmake_minimum_required(VERSION 3.20)
project(bodya-sql)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    # Windows paths
    set(MYSQL_CONNECTOR_POSSIBLE_PATHS
        "C:/Program Files/MySQL/Connector C++ 8.0"
        "C:/Program Files (x86)/MySQL/Connector C++ 8.0"
        "$ENV{ProgramFiles}/MySQL/Connector C++ 8.0"
        "$ENV{MYSQLCONNECTOR_ROOT}"
    )
elseif(APPLE)
    # macOS paths
    set(MYSQL_CONNECTOR_POSSIBLE_PATHS
        "/usr/local"
        "/opt/homebrew"
        "/opt/homebrew/Cellar/mysql-connector-c++/*"
        "$ENV{MYSQLCONNECTOR_ROOT}"
    )
else()
    # Linux paths
    set(MYSQL_CONNECTOR_POSSIBLE_PATHS
        "/usr"
        "/usr/local"
        "/opt/mysql"
        "$ENV{MYSQLCONNECTOR_ROOT}"
    )
endif()

# Find MySQL Connector/C++
find_library(MYSQLCPPCONN_LIBRARY
    NAMES 
        mysqlcppconn
        mysqlcppconnx
        libmysqlcppconn
        libmysqlcppconnx
    PATHS ${MYSQL_CONNECTOR_POSSIBLE_PATHS}
    PATH_SUFFIXES 
        lib
        lib64
        lib/x64
)

find_path(MYSQLCPPCONN_INCLUDE_DIR
    NAMES mysqlx/xdevapi.h
    PATHS ${MYSQL_CONNECTOR_POSSIBLE_PATHS}
    PATH_SUFFIXES 
        include
        include/mysql
        include/mysql-cppconn-8
)

if(NOT MYSQLCPPCONN_LIBRARY OR NOT MYSQLCPPCONN_INCLUDE_DIR)
    message(FATAL_ERROR 
        "MySQL Connector/C++ not found. Please:\n"
        "1. Install MySQL Connector/C++ on your system\n"
        "2. Or set MYSQLCONNECTOR_ROOT environment variable to your installation path\n"
        "3. Or manually specify paths using -DMYSQLCPPCONN_LIBRARY=path -DMYSQLCPPCONN_INCLUDE_DIR=path"
    )
endif()

message(STATUS "Found MySQL Connector/C++ library: ${MYSQLCPPCONN_LIBRARY}")
message(STATUS "Found MySQL Connector/C++ include: ${MYSQLCPPCONN_INCLUDE_DIR}")

# Include directories
include_directories(
    ${MYSQLCPPCONN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/_deps/raylib-src/src
    ${CMAKE_BINARY_DIR}/_deps/raylib-src/src/external/glfw/include
)

# Add this to ensure MySQL headers are found
if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
elseif(UNIX)
    include_directories(/usr/include/mysql-cppconn-8)
    link_directories(/usr/lib)
endif()

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/core/database/DatabaseManager.cpp
    src/core/database/TableManager.cpp
    src/core/database/TableStructureManager.cpp
    src/core/database/TableDataManager.cpp
    src/core/database/ValueFormatter.cpp
    src/core/database/Query.cpp
    src/core/export/QueryExporter.cpp
    src/core/export/DatabaseExporter.cpp
    src/core/export/SQLScriptParser.cpp
    src/core/export/DatabaseStructureHandler.cpp
    src/gui/GuiManager.cpp
    src/gui/panels/ConnectionPanel.cpp
    src/gui/panels/QueryPanel.cpp
    src/gui/panels/ResultsPanel.cpp
    src/gui/components/MessageSystem.cpp
    src/gui/components/ScrollableList.cpp
    src/gui/components/ERDiagram.cpp
    src/gui/components/diagram/TableRenderer.cpp
    src/gui/components/diagram/RelationshipRenderer.cpp
    src/gui/components/diagram/DiagramInteractionHandler.cpp
    src/gui/raygui_impl.cpp
    src/gui/commands/DatabaseCommand.cpp
    src/gui/commands/CommandFactory.cpp
    src/gui/core/EventBus.cpp
    src/gui/states/ApplicationState.cpp
    src/gui/commands/CommandExecutor.cpp
    src/gui/components/IconRenderer.cpp
    src/gui/components/dialogs/Dialog.cpp
    src/gui/components/dialogs/ExportDialog.cpp
    src/gui/components/dialogs/ImportDialog.cpp
)

# Add raylib
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 4.5.0
)
FetchContent_MakeAvailable(raylib)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        ${MYSQLCPPCONN_LIBRARY}
        raylib
        winmm
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} 
        ${MYSQLCPPCONN_LIBRARY}
        raylib
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        ${MYSQLCPPCONN_LIBRARY}
        raylib
        GL
        m
        pthread
        dl
        rt
        X11
    )
endif()

# Create directories for exports and imports
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_BINARY_DIR}/imports
        ${CMAKE_CURRENT_BINARY_DIR}/exports
)

# Copy dependencies on Windows
if(WIN32)
    get_filename_component(MYSQL_CONNECTOR_DIR ${MYSQLCPPCONN_LIBRARY} DIRECTORY)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MYSQL_CONNECTOR_DIR}/mysqlcppconn8-2*.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
