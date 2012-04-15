isEmpty(CON_SOURCE_ROOT): CON_SOURCE_ROOT=$$PWD/..
isEmpty(CON_BUILD_ROOT): CON_BUILD_ROOT=$$OUT_PWD/..

INCLUDEPATH += $$CON_SOURCE_ROOT
DEPENDPATH += $$CON_SOURCE_ROOT

CONFIG(debug, debug|release) {
    LIBS += $$CON_BUILD_ROOT/winapitools/debug/winapitools.lib
}
CONFIG(release, debug|release) {
    LIBS += $$CON_BUILD_ROOT/winapitools/release/winapitools.lib
}
