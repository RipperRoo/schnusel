#pragma once

#include <winapitools/filemapview.h>
#include <winapitools/handle.h>

#include <string>
#include <QString>

class ConsoleProcess
{
    Q_DECLARE_TR_FUNCTIONS(ConsoleProcess);
public:
    ConsoleProcess();
    ~ConsoleProcess();

    bool start();
    QString errorString() const;

    bool ipcStartShell(const std::wstring &cmdLine);
    void ipcShutDown();
    HWND ipcGetConsoleWindow();
    bool ipcGetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO *sbi);
    bool ipcGetCurrentConsoleFont(bool maximumWindow, CONSOLE_FONT_INFO *cfi);
    bool ipcReadConsoleOutput(CHAR_INFO *buffer, const COORD &bufferSize, SMALL_RECT *readRegion);

private:
    bool ipcWait(const DWORD dwTimeout = INFINITE);

private:
    Handle m_hClientEvent;
    Handle m_hServerEvent;
    Handle m_hSharedMemory;
    Handle m_hProcess;
    FileMapView m_pSharedMemory;
    QString m_errorString;
};
