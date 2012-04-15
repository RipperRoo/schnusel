#pragma once

#include <streambuf>

class memstreambuf : public std::streambuf
{
public:
    memstreambuf(char *begin, size_t size)
    {
        setg(begin, begin, begin + size);
    }

    //int_type underflow()
    //{
    //    return gptr() == egptr() ?
    //        traits_type::eof() :
    //        traits_type::to_int_type(*gptr());
    //}
};
