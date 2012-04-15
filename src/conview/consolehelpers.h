#ifndef CONHELPERS_H
#define CONHELPERS_H

#include <windows.h>

inline SHORT width(const SMALL_RECT &rect)
{
    return rect.Right - rect.Left;
}

inline SHORT height(const SMALL_RECT &rect)
{
    return rect.Bottom - rect.Top;
}

#endif // CONHELPERS_H
