#include "StdAfx.h"
#include "sharedmemorystream.h"
#include "filemapview.h"

SharedMemoryStream::SharedMemoryStream(FileMapView *fmv, Direction direction)
    : m_fmv(fmv)
    , m_direction(direction)
    , m_offset(0)
    , m_error(0)
{
}


SharedMemoryStream::~SharedMemoryStream()
{
}

char *SharedMemoryStream::ptr() const
{
    return m_fmv->c_ptr() + m_offset;
}

size_t SharedMemoryStream::bytesLeft() const
{
    return m_fmv->count() - m_offset;
}

void SharedMemoryStream::setError(errno_t e)
{
    m_error = e;
}

errno_t SharedMemoryStream::error() const
{
    return m_error;
}

template <>
SharedMemoryStream &operator << (SharedMemoryStream &s, const std::wstring &wstr)
{
    s << wstr.size();
    const size_t wstrBufSize = sizeof(wchar_t) * wstr.size();
    const errno_t err = memcpy_s(s.ptr(), s.bytesLeft(), wstr.c_str(), wstrBufSize);
    if (err != 0) {
        s.setError(err);
        return s;
    }
    s.advancePtr(wstrBufSize);
    return s;
}

template <>
SharedMemoryStream &operator >> (SharedMemoryStream &s, std::wstring &wstr)
{
    size_t wstrSize;
    s >> wstrSize;
    const size_t wstrBufSize = sizeof(wchar_t) * wstrSize;
    wstr.resize(wstrSize);
    char *wstrBuf = reinterpret_cast<char *>(const_cast<wchar_t *>(wstr.data()));
    const errno_t err = memcpy_s(wstrBuf, wstrBufSize, s.ptr(), wstrBufSize);
    if (err != 0) {
        s.setError(err);
        return s;
    }
    s.advancePtr(wstrBufSize);
    return s;
}
