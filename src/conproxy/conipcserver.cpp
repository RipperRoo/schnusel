#include "StdAfx.h"
#include "conipcserver.h"
#include "conipc.h"

#include <cassert>

IpcServer::IpcServer(HANDLE hClientEvent, HANDLE hServerEvent, HANDLE hSharedMemory, DWORD dwSharedMemorySize)
    : m_hClientEvent(hClientEvent)
    , m_hServerEvent(hServerEvent)
    , m_hSharedMemory(hSharedMemory)
    , m_hStdout(INVALID_HANDLE_VALUE)
    , m_hStdin(INVALID_HANDLE_VALUE)
    , m_appHwnd(NULL)
    , m_dwShellPid(0)
{
    DWORD dw;
    dw = m_pSharedMemory.map(hSharedMemory, FILE_MAP_WRITE, 0, 0, dwSharedMemorySize);
    if (dw != ERROR_SUCCESS) {
        WCHAR msg[256];
        wsprintf(msg, L"MapViewOfFile failed with error code %d.\n", dw);
        MessageBox(NULL, msg, L"conproxy error", MB_OK | MB_ICONERROR);
    }

    ZeroMemory(&m_handleFunctions, IPC_COMMAND_COUNT * sizeof(CommandHandler));
    m_handleFunctions[IPC_STARTSHELL] = &IpcServer::handleStartShell;
    m_handleFunctions[IPC_SHUTDOWN] = &IpcServer::handleShutDown;
    m_handleFunctions[IPC_GETCONSOLEWINDOW] = &IpcServer::handleGetConsoleWindow;
    m_handleFunctions[IPC_GETCONSOLESCREENBUFFERINFO] = &IpcServer::handleGetConsoleScreenBufferInfo;
    m_handleFunctions[IPC_GETCURRENTCONSOLEFONT] = &IpcServer::handleGetCurrentConsoleFont;
    m_handleFunctions[IPC_READCONSOLEOUTPUT] = &IpcServer::handleReadConsoleOutput;
    m_handleFunctions[IPC_GETCONSOLEPID] = &IpcServer::handleGetConsoleProcessId;
    m_handleFunctions[IPC_WRITECONSOLEINPUT] = &IpcServer::handleWriteConsoleInput;
}

IpcServer::~IpcServer()
{
}

void IpcServer::handleCommand()
{
    ResetEvent(m_hServerEvent);

    SharedMemoryStream s(&m_pSharedMemory);
    IpcCommandId id;
    s >> id;

    if (id < 0 || id >= IPC_COMMAND_COUNT) {
        WCHAR s[256];
        wsprintf(s, L"CONPROXY doesn't know how to handle command %d.\n", id);
        OutputDebugString(s);
        MessageBox(NULL, s, L"conproxy error", MB_OK | MB_ICONERROR);
        return;
    }

    CommandHandler func = m_handleFunctions[id];
    if (!func)
        return;

    (this->*func)(s);
    SetEvent(m_hClientEvent);
}

bool IpcServer::startShell(const wstring &commandLine)
{
    bool success = true;
    STARTUPINFO si = {0};
    si.cb = sizeof(si);

    WCHAR *processCmdLine = new WCHAR[commandLine.size() + 1];
    wmemcpy(processCmdLine, commandLine.data(), commandLine.size() + 1);

    DWORD dwFlags = CREATE_NEW_CONSOLE;
    PROCESS_INFORMATION pi;
    success = CreateProcess(NULL, processCmdLine, NULL, NULL, TRUE, dwFlags, NULL, NULL, &si, &pi) != 0;
    delete[] processCmdLine;
    processCmdLine = 0;
    if (!success) {
        WCHAR s[256];
        wsprintf(s, L"CreateProcess failed with error code %d.\n", GetLastError());
        OutputDebugString(s);
        return false;
    }

    unsigned char count = 50;
    while (!AttachConsole(pi.dwProcessId) && --count >= 0) {
        WCHAR s[256];
        wsprintf(s, L"AttachConsole failed with error code %d.\n", GetLastError());
        OutputDebugString(s);
        Sleep(250);
    }

    m_hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    m_hStdin = GetStdHandle(STD_INPUT_HANDLE);
    m_dwShellPid = pi.dwProcessId;
    return success;
}

void IpcServer::handleStartShell(SharedMemoryStream &s)
{
    wstring cmdLine;
    s >> cmdLine;
    bool result = startShell(cmdLine);
    s.reset();
    s << result;
}

void IpcServer::handleShutDown(SharedMemoryStream &s)
{
    FreeConsole();
    PostMessage(m_appHwnd, WM_CLOSE, 0, 0);
}

void IpcServer::handleGetConsoleWindow(SharedMemoryStream &s)
{
    s.reset();
    s << GetConsoleWindow();
}

void IpcServer::handleGetConsoleScreenBufferInfo(SharedMemoryStream &s)
{
    s.reset();

    CONSOLE_SCREEN_BUFFER_INFO sbi = {0};
    GetConsoleScreenBufferInfo(m_hStdout, &sbi);

    s << sbi;
}

void IpcServer::handleGetCurrentConsoleFont(SharedMemoryStream &s)
{
    bool maximumWindow;
    s >> maximumWindow;
    s.reset();

    CONSOLE_FONT_INFO cfi = {0};
    GetCurrentConsoleFont(m_hStdout, maximumWindow, &cfi);

    s << cfi;
}

void IpcServer::handleReadConsoleOutput(SharedMemoryStream &s)
{
    COORD bufferSize;
    SMALL_RECT readRegion;
    s >> bufferSize >> readRegion;
    s.reset();

    CHAR_INFO *buf = new CHAR_INFO[bufferSize.X * bufferSize.Y];
    const COORD bufferCoord = {0, 0};
    BOOL success = ReadConsoleOutput(m_hStdout, buf, bufferSize, bufferCoord, &readRegion);
    s << success << readRegion;
    if (!success)
        return;

    const size_t bufSize = sizeof(CHAR_INFO) * bufferSize.X * bufferSize.Y;
    const errno_t err = memcpy_s(s.ptr(), s.bytesLeft(), buf, bufSize);
    if (err != 0) {
        s.setError(err);
        return;
    }
    s.advancePtr(bufSize);

    delete[] buf;
}

void IpcServer::handleGetConsoleProcessId(SharedMemoryStream &s)
{
    s.reset();

    s << m_dwShellPid;
}

void IpcServer::handleWriteConsoleInput(SharedMemoryStream &s)
{
    DWORD nLength;
    s >> nLength;

    INPUT_RECORD *ir = new INPUT_RECORD[nLength];
    for (DWORD i = 0; i < nLength; ++i) {
        s >> ir[i].EventType;
        switch (ir[i].EventType) {
        case FOCUS_EVENT:
            break;
        case KEY_EVENT:
            s >> ir[i].Event.KeyEvent.bKeyDown
              >> ir[i].Event.KeyEvent.wRepeatCount
              >> ir[i].Event.KeyEvent.wVirtualKeyCode
              >> ir[i].Event.KeyEvent.wVirtualScanCode
              >> ir[i].Event.KeyEvent.uChar.UnicodeChar
              >> ir[i].Event.KeyEvent.dwControlKeyState;
            break;
        case MENU_EVENT:
            break;
        case MOUSE_EVENT:
            break;
        case WINDOW_BUFFER_SIZE_EVENT:
            break;
        }
    }

    DWORD dwNumberOfEventsWritten = 0;
    BOOL success = WriteConsoleInput(m_hStdin, ir, nLength, &dwNumberOfEventsWritten);
    delete[] ir;

    s.reset();
    s << success << dwNumberOfEventsWritten;
}
