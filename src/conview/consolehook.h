#ifndef CONSOLEHOOK_H
#define CONSOLEHOOK_H

#include <QHash>

class ConsoleObserver;

class ConsoleHook
{
    ConsoleHook();
    ~ConsoleHook();

public:
    static ConsoleHook *instance();
    static void destroy();

    bool install();
    void uninstall();

    void registerConsoleWindow(HWND hwndConsole, ConsoleObserver *observer);
    void unregisterConsoleWindow(HWND hwndConsole);

private:
    static void CALLBACK winEventProc(HWINEVENTHOOK hWinEventHook,
                                      DWORD ev,
                                      HWND hwnd,
                                      LONG idObject,
                                      LONG idChild,
                                      DWORD dwEventThread,
                                      DWORD dwmsEventTime);

private:
    HWINEVENTHOOK m_hEventHook;
    QHash<HWND, ConsoleObserver *> m_consoleWindows;
    ConsoleObserver *m_observer;
};

#endif
