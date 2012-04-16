#ifndef CONSOLEOBSERVER_H
#define CONSOLEOBSERVER_H

class ConsoleObserver
{
public:
    virtual void onConsoleApplicationStarted(unsigned long pid) = 0;
    virtual void onConsoleApplicationEnded(unsigned long pid) = 0;
    virtual void onConsoleCaretMoved(bool selection, bool visible, const COORD &cursorPos) = 0;
    virtual void onConsoleLayoutChanged() = 0;
    virtual void onConsoleRegionUpdate(const COORD &regionStart, const COORD &regionEnd) = 0;
    virtual void onConsoleScrolled(long horizontalDistance, long verticalDistance) = 0;
    virtual void onConsoleSimpleUpdate(const COORD &pos, unsigned short utf16Char, unsigned short characterAttributes) = 0;
};

#endif // CONSOLEOBSERVER_H
