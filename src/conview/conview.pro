TEMPLATE = app
QT       += core gui widgets

TARGET = consolator
DESTDIR = $$OUT_PWD/../../bin

LIBS += user32.lib

include(../winapitools/use_winapitools.pri)

PRECOMPILED_HEADER = stdafx.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    application.cpp \
    consoleview.cpp \
    profilesdialog.cpp \
    profilesmanager.cpp \
    consoleprocess.cpp \
    consolehook.cpp \
    debughelpers.cpp

HEADERS += \
    mainwindow.h \
    application.h \
    consoleview.h \
    profilesdialog.h \
    profilesmanager.h \
    consoleprocess.h \
    consolehook.h \
    stdafx.h \
    consoleobserver.h \
    debughelpers.h \
    consolehelpers.h

FORMS += \
    mainwindow.ui \
    profilesdialog.ui
