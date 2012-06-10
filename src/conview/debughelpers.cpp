#include "debughelpers.h"

QDebug operator<<(QDebug dbg, const COORD &c)
{
    dbg.nospace() << "(" << c.X << ", " << c.Y << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const SMALL_RECT &sr)
{
    dbg.nospace() << "(" << sr.Left << ", " << sr.Top
                  << ", " << sr.Right << ", " << sr.Bottom << ")";
    return dbg.space();
}

