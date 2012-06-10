#include "stdafx.h"
#include "consoleview.h"
#include "consolehelpers.h"
#include "consolehook.h"
#include "consoleprocess.h"
#include "debughelpers.h"

ConsoleView::ConsoleView(QWidget *parent)
    : QWidget(parent)
    , m_process(0)
    , m_fontMetrics(font())
{
    ZeroMemory(&m_conScreenBufferInfo, sizeof(m_conScreenBufferInfo));

    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0, 0, 0));
    pal.setColor(QPalette::Foreground, Qt::lightGray);
    setPalette(pal);
    setAutoFillBackground(true);

    QFont f = font();
    f.setFamily("Lucida Console");
    f.setPointSize(10);
    setFont(f);
    m_fontMetrics = QFontMetrics(font());
    m_charCellSize = QSize(m_fontMetrics.maxWidth(),
                           m_fontMetrics.ascent() + m_fontMetrics.descent() + m_fontMetrics.leading());

    QTimer::singleShot(0, this, SLOT(startProcess()));
    qDebug("ConsoleView::ConsoleView thread: %d", GetCurrentThreadId());
}

ConsoleView::~ConsoleView()
{
    delete m_process;
}

void ConsoleView::startProcess()
{
    if (!m_process)
        m_process = new ConsoleProcess;

    if (!m_process->start()) {
        QMessageBox::critical(this, tr("Error"), m_process->errorString());
        return;
    }

    HWND hwndConsoleWindow = m_process->ipcGetConsoleWindow();
    ConsoleHook::instance()->registerConsoleWindow(hwndConsoleWindow, this);
}

void ConsoleView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawPixmap(e->rect().topLeft(), m_pixmap, e->rect());
}

void ConsoleView::resizeEvent(QResizeEvent *e)
{
    m_pixmap = QPixmap(e->size());
    QPainter p(&m_pixmap);
    p.fillRect(m_pixmap.rect(), palette().background());
    p.setPen(palette().foreground().color());
    p.setFont(font());
    p.drawText(m_pixmap.rect(), Qt::AlignCenter, "CONSOLE VIEW lalala");
}

void ConsoleView::onConsoleApplicationStarted(unsigned long pid)
{
    qDebug() << "onConsoleApplicationStarted" << pid << "shell process:" << m_process->pid();
}

void ConsoleView::onConsoleApplicationEnded(unsigned long pid)
{
    qDebug() << "onConsoleApplicationEnded" << pid;
    if (pid == m_process->pid()) {
        m_process->ipcShutDown();
        QTimer::singleShot(0, this, SLOT(close()));
        QTimer::singleShot(0, this, SIGNAL(closed()));
        return;
    }
}

void ConsoleView::onConsoleCaretMoved(bool selection, bool visible, const COORD &cursorPos)
{
    //qDebug() << "onConsoleCaretMoved" << selection << visible << cursorPos.X << cursorPos.Y;
}

void ConsoleView::onConsoleLayoutChanged()
{
    qDebug() << "onConsoleLayoutChanged";
    SMALL_RECT oldWindow = m_conScreenBufferInfo.srWindow;
    if (!m_process->ipcGetConsoleScreenBufferInfo(&m_conScreenBufferInfo)) {
        qDebug("ipcGetConsoleScreenBufferInfo failed");
        return;
    }
    qDebug() << "    buffer:" << m_conScreenBufferInfo.dwSize;
    qDebug() << "    window:" << m_conScreenBufferInfo.srWindow;

    SMALL_RECT &newWindow = m_conScreenBufferInfo.srWindow;
    if (::height(oldWindow) != ::height(newWindow)
        || ::width(oldWindow) != ::width(newWindow))
    {
        // window size changed
        qDebug() << "    window size changed";
    } else {
        if (oldWindow.Top != newWindow.Top)
            scrollConsoleWindowVertically(oldWindow.Top - newWindow.Top);
        if (oldWindow.Left != newWindow.Left)
            scrollConsoleWindowHorizontally(oldWindow.Left - newWindow.Left);
    }

    CONSOLE_FONT_INFO cfi = {0};
    if (!m_process->ipcGetCurrentConsoleFont(false, &cfi)) {
        qDebug("ipcGetCurrentConsoleFont failed");
        return;
    }

    qDebug() << "~~~current font" << cfi.nFont << cfi.dwFontSize.X << cfi.dwFontSize.Y;
//    QFont f = font();
//    f.setPointSize(cfi.dwFontSize.Y);
//    setFont(f);

//    m_currentFontSize.setWidth(cfi.dwFontSize.X);
//    m_currentFontSize.setHeight(cfi.dwFontSize.Y);
//    qDebug() << "@@@@@m_currentFontSize" << m_currentFontSize;
}

static COORD clippedPoint(const COORD &p, const SMALL_RECT &window)
{
    COORD q;
    q.X = qMin(qMax(p.X, window.Left), window.Right);
    q.Y = qMin(qMax(p.Y, window.Top), window.Bottom);
    return q;
}

void ConsoleView::onConsoleRegionUpdate(const COORD &regionStart, const COORD &regionEnd)
{
    qDebug() << "onConsoleRegionUpdate" << regionStart << regionEnd;
    if (regionStart.X > regionEnd.X || regionStart.Y > regionEnd.Y)
        qWarning("onConsoleRegionUpdate: non-rectangular update region!");

    COORD clippedStart = clippedPoint(regionStart, m_conScreenBufferInfo.srWindow);
    COORD clippedEnd = clippedPoint(regionEnd, m_conScreenBufferInfo.srWindow);
    SMALL_RECT clippedRect = {clippedStart.X, clippedStart.Y, clippedEnd.X, clippedEnd.Y };
    qDebug() << "   clippedrect" << clippedRect;

    clippedEnd.X++;
    clippedEnd.Y++;
    COORD bufferSize = { clippedEnd.X - clippedStart.X, clippedEnd.Y - clippedStart.Y};
    if (bufferSize.X <= 0 || bufferSize.Y <= 0)
        qDebug("ARRRRGH");

    CHAR_INFO *buf = new CHAR_INFO[bufferSize.X * bufferSize.Y];
    if (!m_process->ipcReadConsoleOutput(buf, bufferSize, &clippedRect))
        qWarning("ipcReadConsoleOutput failed");

    QPainter p(&m_pixmap);
    p.setFont(font());

    // ### need to do an update for all regions with different char attributes
    QRect r(translateBufferToWidget(clippedStart), translateBufferToWidget(clippedEnd));
    p.fillRect(r, Qt::black);

    // ### this is the wrong color
    p.setPen(Qt::lightGray);

    QPoint textPos = translateBufferToWidget(clippedStart);
    textPos.ry() += m_fontMetrics.ascent();
    QString text;
    for (SHORT row = 0; row < bufferSize.Y; ++row) {
        text.clear();
        const size_t rowOffset = row * bufferSize.X;
        for (SHORT column = 0; column < bufferSize.X; ++column) {
            WCHAR *const wcharPtr = &buf[column + rowOffset].Char.UnicodeChar;
            text.append(QString::fromWCharArray(wcharPtr, 1));
        }
        p.drawText(textPos, text);
        textPos.ry() += m_charCellSize.height();
    }

    delete[] buf;

    repaint();  // ### rect
}

void ConsoleView::onConsoleScrolled(long /*horizontalDistance*/, long /*verticalDistance*/)
{
    // don't do anything; handled in onConsoleLayoutChanged
}

void ConsoleView::onConsoleSimpleUpdate(const COORD &pos, unsigned short utf16Char, unsigned short characterAttributes)
{
    QPoint widgetPos = translateBufferToWidget(pos);
//    qDebug() << "onConsoleSimpleUpdate" << pos.X << pos.Y << "widget:" << widgetPos.x() << widgetPos.y();
    QPainter p(&m_pixmap);
    p.setFont(font());
    p.setPen(translateForegroundColor(characterAttributes));
    QString text = QString::fromUtf16(&utf16Char, 1);
    QRect charRect(widgetPos.x(), widgetPos.y(), m_charCellSize.width() + 1, m_charCellSize.height() + 1);
    p.fillRect(charRect, translateBackgroundColor(characterAttributes));

#if 0
    QPen oldPen = p.pen();
    p.setPen(Qt::yellow);
#if 1
    p.drawRect(charRect);
#else
    p.drawPoint(widgetPos);
    p.drawPoint(widgetPos + QPoint(0, m_charCellSize.height()));
    p.drawPoint(widgetPos + QPoint(m_charCellSize.width(), 0));
    p.drawPoint(widgetPos + QPoint(m_charCellSize.width(), m_charCellSize.height()));
#endif
    p.setPen(oldPen);
#endif

    widgetPos.ry() += m_fontMetrics.ascent();
    p.drawText(widgetPos, text);
    repaint(charRect.x(), charRect.y(), charRect.width() + 1, charRect.height() + 1);
}

inline COORD ConsoleView::translateBufferToWindow(const COORD &pos)
{
    COORD result = { pos.X - m_conScreenBufferInfo.srWindow.Left,
                     pos.Y - m_conScreenBufferInfo.srWindow.Top };
    return result;
}

inline QPoint ConsoleView::translateWindowToWidget(const COORD &pos)
{
    return QPoint(pos.X * m_charCellSize.width(),
                  pos.Y * m_charCellSize.height());
}

inline QPoint ConsoleView::translateBufferToWidget(const COORD &pos)
{
    return translateWindowToWidget(translateBufferToWindow(pos));
}

QColor ConsoleView::translateForegroundColor(unsigned short attributes)
{
    // ### take a color table (profile settings) into account
    const int intensity = (attributes & FOREGROUND_INTENSITY) ? 255 : 192;
    return QColor((attributes & FOREGROUND_RED) ? intensity : 0,
                  (attributes & FOREGROUND_GREEN) ? intensity : 0,
                  (attributes & FOREGROUND_BLUE) ? intensity : 0);
}

QColor ConsoleView::translateBackgroundColor(unsigned short attributes)
{
    // ### take a color table (profile settings) into account
    int intensity = (attributes & BACKGROUND_INTENSITY) ? 255 : 192;
    return QColor((attributes & BACKGROUND_RED) ? intensity : 0,
                  (attributes & BACKGROUND_GREEN) ? intensity : 0,
                  (attributes & BACKGROUND_BLUE) ? intensity : 0);
}

void ConsoleView::scrollConsoleWindowHorizontally(SHORT distance)
{
    qDebug() << "scrollConsoleWindowHorizontally" << distance;
    const SHORT windowWidth = ::width(m_conScreenBufferInfo.srWindow);

    if (distance < 0) {
        // scroll right
        if (-distance < windowWidth) {
            int dx = distance * m_charCellSize.width();
            QRect rect = m_pixmap.rect().adjusted(0, 0, dx, 0);
            m_pixmap.scroll(dx, 0, rect);
            rect.adjust(dx, 0, dx, 0);
            update(rect);
        }

        COORD regionStart = {m_conScreenBufferInfo.srWindow.Left,
                             m_conScreenBufferInfo.srWindow.Top};
        COORD regionEnd = {m_conScreenBufferInfo.srWindow.Right + distance + 1,
                           m_conScreenBufferInfo.srWindow.Bottom};
        onConsoleRegionUpdate(regionStart, regionEnd);
    } else if (distance > 0) {
        // scroll left
        if (distance < windowWidth) {
            int dx = distance * m_charCellSize.width();
            QRect rect = m_pixmap.rect().adjusted(dx, 0, 0, 0);
            m_pixmap.scroll(dx, 0, rect);
            rect.adjust(dx, 0, dx, 0);
            update(rect);
        }

        COORD regionStart = {m_conScreenBufferInfo.srWindow.Left + distance - 1,
                             m_conScreenBufferInfo.srWindow.Top};
        COORD regionEnd = {m_conScreenBufferInfo.srWindow.Right,
                           m_conScreenBufferInfo.srWindow.Bottom};
        onConsoleRegionUpdate(regionStart, regionEnd);
    }
}

void ConsoleView::scrollConsoleWindowVertically(SHORT distance)
{
    qDebug() << "scrollConsoleWindowVertically" << distance;
    const SHORT windowHeight = ::height(m_conScreenBufferInfo.srWindow);
    if (distance < 0) {
        // scroll down
        if (-distance < windowHeight) {
            int dy = distance * m_charCellSize.height();
            QRect rect = m_pixmap.rect().adjusted(0, dy, 0, 0);
            m_pixmap.scroll(0, dy, rect);
            rect.adjust(0, dy, 0, dy);
            update(rect);
        }

        COORD regionStart = {m_conScreenBufferInfo.srWindow.Left,
                             m_conScreenBufferInfo.srWindow.Bottom + distance + 1};
        COORD regionEnd = {m_conScreenBufferInfo.srWindow.Right,
                           m_conScreenBufferInfo.srWindow.Bottom};
        onConsoleRegionUpdate(regionStart, regionEnd);
    } else if (distance > 0) {
        // scroll up
        if (distance < windowHeight) {
            int dy = distance * m_charCellSize.height();
            QRect rect = m_pixmap.rect().adjusted(0, 0, 0, -dy);
            m_pixmap.scroll(0, dy, rect);
            rect.adjust(0, dy, 0, dy);
            update(rect);
        }

        COORD regionStart = {m_conScreenBufferInfo.srWindow.Left,
                             m_conScreenBufferInfo.srWindow.Top};
        COORD regionEnd = {m_conScreenBufferInfo.srWindow.Right,
                           m_conScreenBufferInfo.srWindow.Top + distance - 1};
        onConsoleRegionUpdate(regionStart, regionEnd);
    }
}
