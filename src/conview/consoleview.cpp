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
    setFocusPolicy(Qt::ClickFocus);
    ZeroMemory(&m_conScreenBufferInfo, sizeof(m_conScreenBufferInfo));

    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0, 0, 0));
    pal.setColor(QPalette::Foreground, Qt::lightGray);
    setPalette(pal);
    setAutoFillBackground(true);

    QFont f = font();
    f.setFamily("Lucida Console");  // TODO: don't hard code
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

void ConsoleView::focusInEvent(QFocusEvent *)
{
    qDebug() << Q_FUNC_INFO;
}

void ConsoleView::focusOutEvent(QFocusEvent *)
{
    qDebug() << Q_FUNC_INFO;
}

//static INPUT_RECORD toInputRecord(QKeyEvent *e)
//{

//}

void ConsoleView::keyPressEvent(QKeyEvent *e)
{
    keyPressReleaseEvent(e, TRUE);
}

void ConsoleView::keyReleaseEvent(QKeyEvent *e)
{
    keyPressReleaseEvent(e, FALSE);
}

void ConsoleView::keyPressReleaseEvent(QKeyEvent *e, BOOL keyDown)
{
    size_t nRecords = qMax(e->text().count(), 1);
    INPUT_RECORD *ir = new INPUT_RECORD[nRecords];
    ZeroMemory(ir, sizeof(INPUT_RECORD) * nRecords);
    ir[0].EventType = KEY_EVENT;
    KEY_EVENT_RECORD *ker = &ir[0].Event.KeyEvent;
    if (e->modifiers() & Qt::ShiftModifier)
        ker->dwControlKeyState |= SHIFT_PRESSED;
    if (e->modifiers() & Qt::ControlModifier)
        ker->dwControlKeyState |= LEFT_CTRL_PRESSED;
    if (e->modifiers() & Qt::AltModifier)
        ker->dwControlKeyState |= LEFT_ALT_PRESSED;
    if (e->modifiers() & Qt::MetaModifier)
        qDebug() << "keyPressEvent Qt::MetaModifier";

    for (size_t i = 0; i < nRecords; ++i) {
        if (i != 0)
            ir[i] = ir[i - 1];
        ker = &ir[i].Event.KeyEvent;
        ker->bKeyDown = keyDown;
        ker->wRepeatCount = 1;
        if (static_cast<int>(i) < e->text().length())
            ker->uChar.UnicodeChar = e->text().at(i).unicode();
        ker->wVirtualScanCode = e->nativeScanCode();
        ker->wVirtualKeyCode = e->nativeVirtualKey();
    }

    DWORD dwNumberOfEventsWritten = 0;
    if (!m_process->ipcWriteConsoleInput(ir, nRecords, &dwNumberOfEventsWritten))
        qDebug("ipcWriteConsoleInput failed");

    delete[] ir;
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
    QPoint textPos = translateBufferToWidget(clippedStart);
    textPos.ry() += m_fontMetrics.ascent();
    QString text;
    text.reserve(bufferSize.X);
    size_t rowOffset = 0;
    for (SHORT row = 0; row < bufferSize.Y; ++row) {
        WORD lastCharAttributes = buf[0].Attributes;
        WORD charAttributes = buf[0].Attributes;
        SHORT startIdx = 0;
        const SHORT lastColumn = bufferSize.X - 1;
        const int origTextPosX = textPos.x();

        for (SHORT column = 0; column <= lastColumn; ++column) {
            if (column == lastColumn) {
                ++column;
            } else if (charAttributes != buf[column].Attributes) {
                lastCharAttributes = charAttributes;
                charAttributes = buf[column].Attributes;
            } else {
                continue;
            }

            text.resize(column - startIdx);
            for (SHORT bufIdx = startIdx; bufIdx < column; ++bufIdx)
                text[bufIdx - startIdx] = QChar(buf[bufIdx + rowOffset].Char.UnicodeChar);

            p.fillRect(
                QRect(textPos.x(), textPos.y() - m_fontMetrics.ascent(),
                      m_charCellSize.width() * text.length() + 1, m_charCellSize.height() + 1),
                translateBackgroundColor(lastCharAttributes));
            p.setPen(translateForegroundColor(lastCharAttributes));
            p.drawText(textPos, text);
            startIdx = column;
            textPos.rx() += text.length() * m_charCellSize.width();
        }

        rowOffset += bufferSize.X;
        textPos.rx() = origTextPosX;
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
    Q_UNUSED(utf16Char);
    Q_UNUSED(characterAttributes);
    QPoint widgetPos = translateBufferToWidget(pos);
//    qDebug() << "onConsoleSimpleUpdate" << pos.X << pos.Y << "widget:" << widgetPos.x() << widgetPos.y();

    // For some applications (e.g. vim) we get nonsense values in utf16Char.
    // We must read the console buffer to check what's in there.
    CHAR_INFO charInfo;
    {
        const COORD bufSize = {1, 1};
        SMALL_RECT readRegion = { pos.X, pos.Y, pos.X, pos.Y };
        if (!m_process->ipcReadConsoleOutput(&charInfo, bufSize, &readRegion))
            qWarning("ipcReadConsoleOutput failed");
    }

    QPainter p(&m_pixmap);
    p.setFont(font());
    p.setPen(translateForegroundColor(characterAttributes));
    QString text = QChar(charInfo.Char.UnicodeChar);
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
