#include "StdAfx.h"
#include "filemapview.h"

FileMapView::FileMapView()
    : m_ptr(0)
    , m_count(0)
{
}

FileMapView::~FileMapView()
{
    if (m_ptr)
        UnmapViewOfFile(m_ptr);
}

DWORD FileMapView::map(HANDLE h, DWORD dwDesiredAccess,
                       DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
                       size_t numberOfBytesToMap)
{
    m_count = numberOfBytesToMap;
    m_ptr = MapViewOfFile(h, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, numberOfBytesToMap);
    return m_ptr ? ERROR_SUCCESS : GetLastError();
}

void FileMapView::close()
{
    if (m_ptr) {
        UnmapViewOfFile(m_ptr);
        m_ptr = 0;
    }
}
