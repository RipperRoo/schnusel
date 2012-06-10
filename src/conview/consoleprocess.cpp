#include "StdAfx.h"
#include "consoleprocess.h"
#include <conproxy/conipc.h>
#include <winapitools/sharedmemorystream.h>

using namespace std;

ConsoleProcess::ConsoleProcess()
    : m_pid(0)
{
}

ConsoleProcess::~ConsoleProcess()
{
    m_pSharedMemory.close();
}

bool ConsoleProcess::start()
{
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    m_hClientEvent = CreateEvent(&sa, TRUE, FALSE, NULL);
    m_hServerEvent = CreateEvent(&sa, TRUE, FALSE, NULL);

    DWORD sharedMemorySize = 1024 * 1024 * 4;
    m_hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE,
                                        0, sharedMemorySize, NULL);
    if (m_hSharedMemory == NULL) {
        WCHAR msg[256];
        wsprintf(msg, L"CreateFileMapping failed with error code %d.\n", GetLastError());
        OutputDebugString(msg);
        return false;
    }

    DWORD dw;
    dw = m_pSharedMemory.map(m_hSharedMemory, FILE_MAP_WRITE, 0, 0, sharedMemorySize);
    if (dw != ERROR_SUCCESS) {
        WCHAR msg[256];
        wsprintf(msg, L"MapViewOfFile failed with error code %d.\n", dw);
        OutputDebugString(msg);
        return false;
    }

    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    DWORD dwCreationFlags = 0;
    const size_t bufsize = 1024;
    wchar_t *buf = new wchar_t[bufsize];
    wsprintf(buf, L" %d %d %d %d", m_hClientEvent.handle(), m_hServerEvent.handle(), m_hSharedMemory.handle(), sharedMemorySize);
    QString conproxyPath = QCoreApplication::applicationDirPath() + QLatin1String("/conproxy.exe");
    BOOL success = CreateProcess((WCHAR*)conproxyPath.utf16(), buf, NULL, NULL, TRUE, dwCreationFlags, NULL, NULL, &si, &pi);
    DWORD dwError;
    if (!success)
        dwError = GetLastError();
    delete[] buf;
    buf = 0;
    if (!success) {
        m_errorString = tr("CreateProcess failed with error code %1.").arg(GetLastError());
        return false;
    }

    CloseHandle(pi.hThread);
    m_hProcess = pi.hProcess;

    if (!ipcWait(5000)) {
        m_errorString = tr("Console proxy process seems to be dead.");
        return false;
    }

    // ### hard coded path
    if (!ipcStartShell(L"C:\\Windows\\System32\\cmd.exe /K \"prompt P\"")) {
        m_errorString = tr("The console proxy could not start the shell process.");
        return false;
    }

    return true;
}

QString ConsoleProcess::errorString() const
{
    return m_errorString;
}

unsigned long ConsoleProcess::pid()
{
    if (!m_pid) {
        DWORD pid;
        if (ipcGetConsoleProcessId(&pid)) {
            m_pid = pid;
        } else {
            m_errorString = tr("ipcGetConsoleProcessPid failed");
        }
    }
    return m_pid;
}

bool ConsoleProcess::ipcWait(const DWORD dwTimeout)
{
    if (WaitForSingleObject(m_hClientEvent, dwTimeout) == WAIT_OBJECT_0) {
        ResetEvent(m_hClientEvent);
        return true;
    }
    return false;
}

bool ConsoleProcess::ipcStartShell(const std::wstring &cmdLine)
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_STARTSHELL << cmdLine;

    SetEvent(m_hServerEvent);
    s.reset();
    if (!ipcWait(5000))
        return false;

    bool result;
    s >> result;
    return result;
}

void ConsoleProcess::ipcShutDown()
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_SHUTDOWN;
    SetEvent(m_hServerEvent);
    ipcWait();
}

HWND ConsoleProcess::ipcGetConsoleWindow()
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_GETCONSOLEWINDOW;

    SetEvent(m_hServerEvent);
    s.reset();
    if (!ipcWait())
        return 0;

    HWND result;
    s >> result;
    return result;
}

bool ConsoleProcess::ipcGetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO *sbi)
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_GETCONSOLESCREENBUFFERINFO;

    SetEvent(m_hServerEvent);
    s.reset();
    if (!ipcWait())
        return false;

    s >> *sbi;
    return true;
}

bool ConsoleProcess::ipcGetCurrentConsoleFont(bool maximumWindow, CONSOLE_FONT_INFO *cfi)
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_GETCURRENTCONSOLEFONT << maximumWindow;

    SetEvent(m_hServerEvent);
    s.reset();
    if (!ipcWait())
        return false;

    s >> *cfi;
    return true;
}

bool ConsoleProcess::ipcReadConsoleOutput(CHAR_INFO *buf, const COORD &bufferSize, SMALL_RECT *readRegion)
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_READCONSOLEOUTPUT << bufferSize << *readRegion;

    SetEvent(m_hServerEvent);
    s.reset();
    if (!ipcWait())
        return false;

    BOOL success;
    s >> success >> *readRegion;
    if (!success)
        return false;

    const size_t bufSize = sizeof(CHAR_INFO) * bufferSize.X * bufferSize.Y;
    const errno_t err = memcpy_s(buf, bufSize, s.ptr(), bufSize);
    if (err != 0) {
        s.setError(err);
        return false;
    }
    s.advancePtr(bufSize);

    return true;
}

bool ConsoleProcess::ipcGetConsoleProcessId(DWORD *pid)
{
    SharedMemoryStream s(&m_pSharedMemory);
    s << IPC_GETCONSOLEPID;

    SetEvent(m_hServerEvent);
    s.reset();
    if (!ipcWait())
        return false;

    s >> *pid;
    return pid != 0;
}
