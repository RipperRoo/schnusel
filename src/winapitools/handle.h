#pragma once

#include <Windows.h>

class Handle
{
public:
    explicit Handle(HANDLE h = INVALID_HANDLE_VALUE);
    explicit Handle(const Handle &rhs);
    ~Handle();

    bool isValid() const;
    operator bool() const;
    void close();

    Handle &operator = (const Handle &h);
    Handle &operator = (const HANDLE &h);
    operator HANDLE() const;
    HANDLE handle() const { return m_handle; }

private:
    HANDLE m_handle;
};

inline bool Handle::isValid() const
{
    return m_handle != INVALID_HANDLE_VALUE;
}

inline Handle::operator bool() const
{
    return m_handle != INVALID_HANDLE_VALUE;
}

inline Handle::operator HANDLE() const
{
    return m_handle;
}
