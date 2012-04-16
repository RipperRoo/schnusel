#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QStackedLayout>
#include <QTabWidget>

class ConsoleView;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    ConsoleView *createConsoleView(QWidget *parent = 0);
    void closeTab(int idx);

private slots:
    void on_actionNewWindow_triggered();
    void on_actionNewTab_triggered();
    void on_actionCloseWindow_triggered();
    void on_actionCloseTab_triggered();
    void on_actionExit_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();
    void on_actionEditProfiles_triggered();
    void on_actionEditKeyboardShortcuts_triggered();
    void on_actionEditProfilePreferences_triggered();
    void on_actionShowMenubar_toggled(bool);
    void on_actionFullScreen_toggled(bool);
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionNormalSize_triggered();
    void on_actionAbout_triggered();
    void consoleViewClosed();

private:
    Ui::MainWindow *ui;
    QStackedLayout *m_stackedLayout;
    QTabWidget *m_tabWidget;
    bool m_wasMaximized;
};

#endif // MAINWINDOW_H
