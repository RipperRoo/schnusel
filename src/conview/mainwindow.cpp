#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "consoleview.h"
#include "profilesdialog.h"

#include <QMenuBar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
//    : QMainWindow(parent, Qt::FramelessWindowHint),
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    //    setAttribute(Qt::WA_TranslucentBackground);
//    //    statusBar()->setAutoFillBackground(true);

    // make sure that the short cuts still work if the menu bar is hidden
    addActions(ui->menuBar->actions());

    Q_ASSERT(ui->centralWidget->layout() == 0);
    m_stackedLayout = new QStackedLayout;
    ui->centralWidget->setLayout(m_stackedLayout);
    ConsoleView *view = new ConsoleView;
    m_stackedLayout->addWidget(view);
    m_tabWidget = new QTabWidget;
    m_stackedLayout->addWidget(m_tabWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNewWindow_triggered()
{
    MainWindow *mw = new MainWindow;
    mw->show();
}

void MainWindow::on_actionNewTab_triggered()
{
    ConsoleView *consoleView = 0;
    if (m_tabWidget->count() == 0) {
        QWidget *w = m_stackedLayout->currentWidget();
        m_stackedLayout->removeWidget(w);
        consoleView = qobject_cast<ConsoleView *>(w);
        Q_CHECK_PTR(consoleView);
        m_tabWidget->addTab(w, consoleView->title());
    }
    consoleView = new ConsoleView;
    int idx = m_tabWidget->addTab(consoleView, consoleView->title());
    m_tabWidget->setCurrentIndex(idx);
    ui->actionCloseTab->setEnabled(true);
}

void MainWindow::on_actionCloseWindow_triggered()
{
    close();
}

void MainWindow::on_actionCloseTab_triggered()
{
    int idx = m_tabWidget->currentIndex();
    QWidget *w = m_tabWidget->widget(idx);
    m_tabWidget->removeTab(idx);
    delete w;
    bool singleView = m_tabWidget->count() == 1;
    ui->actionCloseTab->setEnabled(!singleView);
    if (singleView) {
        idx = m_tabWidget->currentIndex();
        w = m_tabWidget->widget(idx);
        m_tabWidget->removeTab(idx);
        idx = m_stackedLayout->addWidget(w);
        m_stackedLayout->setCurrentIndex(idx);
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionCopy_triggered()
{

}

void MainWindow::on_actionPaste_triggered()
{

}

void MainWindow::on_actionSelectAll_triggered()
{

}

void MainWindow::on_actionEditProfiles_triggered()
{
    ProfilesDialog dlg;
    dlg.exec();
}

void MainWindow::on_actionEditKeyboardShortcuts_triggered()
{
}

void MainWindow::on_actionEditProfilePreferences_triggered()
{
}

void MainWindow::on_actionShowMenubar_toggled(bool enable)
{
    QMenuBar *mb = menuBar();
    if (enable)
        mb->show();
    else
        mb->hide();
}

void MainWindow::on_actionFullScreen_toggled(bool enable)
{
    if (enable) {
        m_wasMaximized = isMaximized();
        showFullScreen();
    } else {
        if (m_wasMaximized)
            showMaximized();
        else
            showNormal();
    }
}

void MainWindow::on_actionZoomIn_triggered()
{

}

void MainWindow::on_actionZoomOut_triggered()
{

}

void MainWindow::on_actionNormalSize_triggered()
{

}

void MainWindow::on_actionAbout_triggered()
{

}
