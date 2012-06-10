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
    IPC_GETCONSOLEPID,
    IPC_WRITECONSOLEINPUT,
    IPC_COMMAND_COUNT   // this must be the last entry
};

//void ipcInit();


struct IpcBasicCommand
{

};
