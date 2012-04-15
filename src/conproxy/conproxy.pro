TEMPLATE = app
DESTDIR = $$OUT_PWD/../../bin
QT -= core gui

LIBS += user32.lib shell32.lib

PRECOMPILED_HEADER = stdafx.h
HEADERS += conipc.h conipcserver.h conproxy.h Resource.h stdafx.h targetver.h
SOURCES += conipc.cpp conipcserver.cpp conproxy.cpp stdafx.cpp
RC_FILE = conproxy.rc

include(../winapitools/use_winapitools.pri)
