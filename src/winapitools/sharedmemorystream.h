#pragma once

#include <string>

class FileMapView;

class SharedMemoryStream
{
public:
    enum Direction
    {
        Read,
        Write            
    };

    SharedMemoryStream(FileMapView *fmv, Direction direction = Write);
    ~SharedMemoryStream();

    char *ptr() const;
    size_t bytesLeft() const;
    void advancePtr(size_t n) { m_offset += n; }
    void reset() { m_offset = 0; }
    void setError(errno_t e);
    errno_t error() const;

private:
    FileMapView *m_fmv;
    Direction m_direction;
    size_t m_offset;
    errno_t m_error;
};

template <typename T>
SharedMemoryStream &operator << (SharedMemoryStream &s, const T &t)
{
    const errno_t err = memcpy_s(s.ptr(), s.bytesLeft(), &t, sizeof(T));
    if (err != 0) {
        s.setError(err);
        return s;
    }
    s.advancePtr(sizeof(T));
    return s;
}

template <typename T>
SharedMemoryStream &operator >> (SharedMemoryStream &s, T &t)
{
    const errno_t err = memcpy_s(&t, sizeof(T), s.ptr(), sizeof(T));
    if (err != 0) {
        s.setError(err);
        return s;
    }
    s.advancePtr(sizeof(T));
    return s;
}

template <>
SharedMemoryStream &operator << (SharedMemoryStream &s, const std::wstring &wstr);

template <>
SharedMemoryStream &operator >> (SharedMemoryStream &s, std::wstring &wstr);
