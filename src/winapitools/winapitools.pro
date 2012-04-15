TEMPLATE = lib
CONFIG += static

HEADERS += filemapview.h \
           handle.h \
           memstreambuf.h \
           sharedmemorystream.h \
           stdafx.h \
           targetver.h

SOURCES += filemapview.cpp handle.cpp sharedmemorystream.cpp stdafx.cpp

OTHER_FILES += \
    use_winapitools.pri
