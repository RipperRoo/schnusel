#pragma once

class Handle;

class FileMapView
{
public:
    FileMapView();
    ~FileMapView();

    DWORD map(HANDLE h, DWORD dwDesiredAccess,
              DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
              size_t numberOfBytesToMap);
    void close();

    void *data() const { return m_ptr; }
    char *c_ptr() const { return static_cast<char*>(m_ptr); }
    size_t count() const { return m_count; }

private:
    void *m_ptr;
    size_t m_count;
};
