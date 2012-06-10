#ifndef DEBUGHELPERS_H
#define DEBUGHELPERS_H

#include <QDebug>
#include <qt_windows.h>

QDebug operator<<(QDebug dbg, const COORD &c);
QDebug operator<<(QDebug dbg, const SMALL_RECT &sr);

#endif // DEBUGHELPERS_H
