#include "stdafx.h"
#include "consolehook.h"
#include "consoleobserver.h"

static ConsoleHook *g_pHookInstance = 0;

ConsoleHook *ConsoleHook::instance()
{
    if (!g_pHookInstance)
        g_pHookInstance = new ConsoleHook();
    return g_pHookInstance;
}

void ConsoleHook::destroy()
{
    delete g_pHookInstance;
    g_pHookInstance = 0;
}

ConsoleHook::ConsoleHook()
    : m_hEventHook(0)
{
}

ConsoleHook::~ConsoleHook()
{
    uninstall();
}

bool ConsoleHook::install()
{
    m_hEventHook = SetWinEventHook(EVENT_CONSOLE_CARET,               
                                   EVENT_CONSOLE_END_APPLICATION,     
                                   NULL,
                                   ConsoleHook::winEventProc,
                                   0,
                                   0,
                                   WINEVENT_SKIPOWNPROCESS | WINEVENT_OUTOFCONTEXT);
    return m_hEventHook != 0;
}

void ConsoleHook::uninstall()
{
    if (m_hEventHook) {
        UnhookWinEvent(m_hEventHook);
        m_hEventHook = 0;
    }
}

void CALLBACK ConsoleHook::winEventProc(HWINEVENTHOOK /*hWinEventHook*/,
                                        DWORD ev,
                                        HWND hwnd,
                                        LONG idObject,
                                        LONG idChild,
                                        DWORD /*dwEventThread*/,
                                        DWORD /*dwmsEventTime*/)
{
    ConsoleObserver *observer = g_pHookInstance->m_consoleWindows.value(hwnd);
    if (!observer)
        return;

    switch (ev) {
    case EVENT_CONSOLE_CARET:
        observer->onConsoleCaretMoved(idObject & CONSOLE_CARET_SELECTION,
                                      idObject & CONSOLE_CARET_VISIBLE,
                                      *reinterpret_cast<COORD *>(&idChild));
        break;
    case EVENT_CONSOLE_END_APPLICATION:
        observer->onConsoleApplicationEnded(idObject);
        break;
    case EVENT_CONSOLE_LAYOUT:
        observer->onConsoleLayoutChanged();
        break;
    case EVENT_CONSOLE_START_APPLICATION:
        observer->onConsoleApplicationStarted(idObject);
        break;
    case EVENT_CONSOLE_UPDATE_REGION:
        observer->onConsoleRegionUpdate(*reinterpret_cast<COORD *>(&idObject),
                                        *reinterpret_cast<COORD *>(&idChild));
        break;
    case EVENT_CONSOLE_UPDATE_SCROLL:
        observer->onConsoleScrolled(idObject, idChild);
        break;
    case EVENT_CONSOLE_UPDATE_SIMPLE:
        observer->onConsoleSimpleUpdate(*reinterpret_cast<COORD *>(&idObject),
                                        LOWORD(idChild), HIWORD(idChild));
        break;
    }
}

void ConsoleHook::registerConsoleWindow(HWND hwndConsole, ConsoleObserver *observer)
{
    m_consoleWindows.insert(hwndConsole, observer);
}

void ConsoleHook::unregisterConsoleWindow(HWND hwndConsole)
{
    m_consoleWindows.remove(hwndConsole);
}
