#pragma once

#include "conipc.h"
#include <winapitools/filemapview.h>
#include <winapitools/handle.h>
#include <winapitools/sharedmemorystream.h>

class IpcServer
{
public:
    IpcServer(HANDLE hClientEvent, HANDLE hServerEvent, HANDLE hSharedMemory, DWORD dwSharedMemorySize);
    ~IpcServer();

    void setAppHwnd(HWND hwnd) { m_appHwnd = hwnd; }
    void handleCommand();

private:
    bool startShell(const std::wstring &cmdLine);
    void handleStartShell(SharedMemoryStream &s);
    void handleShutDown(SharedMemoryStream &s);
    void handleGetConsoleWindow(SharedMemoryStream &s);
    void handleGetConsoleScreenBufferInfo(SharedMemoryStream &s);
    void handleGetCurrentConsoleFont(SharedMemoryStream &s);
    void handleReadConsoleOutput(SharedMemoryStream &s);
    void handleGetConsoleProcessId(SharedMemoryStream &s);
    void handleWriteConsoleInput(SharedMemoryStream &s);

private:
    HANDLE m_hClientEvent;
    HANDLE m_hServerEvent;
    HANDLE m_hSharedMemory;
    FileMapView m_pSharedMemory;
    HANDLE m_hStdout;
    HANDLE m_hStdin;
    HWND m_appHwnd;
    DWORD m_dwShellPid;

    typedef void (IpcServer::*CommandHandler)(SharedMemoryStream &s);
    CommandHandler m_handleFunctions[IPC_COMMAND_COUNT];
};
