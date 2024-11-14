#pragma once

#include <string>

struct DatabaseConnectionInfo
{
    char host[128] = "";
    char port[16] = "";
    char user[64] = "";
    char password[64] = "";
    char dbName[64] = "";
    bool editMode = false;
};
