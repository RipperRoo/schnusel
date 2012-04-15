#include "StdAfx.h"
#include "handle.h"

Handle::Handle(HANDLE h)
    : m_handle(h)
{
}

Handle::Handle(const Handle &rhs)
    : m_handle(rhs.m_handle)
{
}

Handle::~Handle()
{
    if (m_handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_handle);
}

void Handle::close()
{
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

Handle &Handle::operator = (const Handle &rhs)
{
    if (m_handle != rhs.m_handle) {
        if (m_handle != INVALID_HANDLE_VALUE)
            CloseHandle(m_handle);
        m_handle = rhs.m_handle;
    }
    return *this;
}

Handle &Handle::operator = (const HANDLE &h)
{
    if (m_handle != h) {
        if (m_handle != INVALID_HANDLE_VALUE)
            CloseHandle(m_handle);
        m_handle = h;
    }
    return *this;
}
