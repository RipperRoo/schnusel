#ifndef CONSOLEVIEW_H
#define CONSOLEVIEW_H

#include <QWidget>
#include "consoleobserver.h"

class ConsoleProcess;

class ConsoleView : public QWidget, protected ConsoleObserver
{
    Q_OBJECT
public:
    explicit ConsoleView(QWidget *parent = 0);
    ~ConsoleView();

    QString title() const { return "conview"; }
signals:

public slots:
    void startProcess();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    // ConsoleObserver implementation
    void onConsoleApplicationStarted(qint64 pid);
    void onConsoleApplicationEnded(qint64 pid);
    void onConsoleCaretMoved(bool selection, bool visible, const COORD &cursorPos);
    void onConsoleLayoutChanged();
    void onConsoleRegionUpdate(const COORD &regionStart, const COORD &regionEnd);
    void onConsoleScrolled(long horizontalDistance, long verticalDistance);
    void onConsoleSimpleUpdate(const COORD &pos, unsigned short utf16Char, unsigned short characterAttributes);

private:
    COORD translateBufferToWindow(const COORD &pos);
    QPoint translateWindowToWidget(const COORD &pos);
    QPoint translateBufferToWidget(const COORD &pos);
    QColor translateForegroundColor(unsigned short attributes);
    QColor translateBackgroundColor(unsigned short attributes);
    void scrollConsoleWindowHorizontally(SHORT distance);
    void scrollConsoleWindowVertically(SHORT distance);

private:
    ConsoleProcess *m_process;
    QPixmap m_pixmap;
    CONSOLE_SCREEN_BUFFER_INFO m_conScreenBufferInfo;
    QSize m_charCellSize;
    QFontMetrics m_fontMetrics;
};

#endif // CONSOLEVIEW_H
