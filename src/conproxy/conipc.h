#pragma once

#include <string>

enum IpcCommandId
{
    IPC_STARTSHELL,
    IPC_SHUTDOWN,
    IPC_GETCONSOLEWINDOW,
    IPC_GETCONSOLESCREENBUFFERINFO,
    IPC_GETCURRENTCONSOLEFONT,
    IPC_READCONSOLEOUTPUT,
    IPC_COMMAND_COUNT
};

//void ipcInit();


struct IpcBasicCommand
{

};
