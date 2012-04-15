#include "stdafx.h"
#include "application.h"
#include "mainwindow.h"
#include "consolehook.h"

Application::Application(int argc, char *argv[])
    : QApplication(argc, argv)
{
    setOrganizationName("consolator.org");
    setOrganizationDomain("consolator.org");
    setApplicationName("Consolator");
}

Application::~Application()
{
    delete m_mainWindow;
    ConsoleHook::destroy();
}

int Application::exec()
{
    if (!ConsoleHook::instance()->install()) {
        QMessageBox::critical(0, tr("Error"), tr("Console hook cannot be installed."));
        ConsoleHook::destroy();
        return 1;
    }

    m_mainWindow = new MainWindow();
    m_mainWindow->show();
    m_mainWindow->move(0,0); // ### remove
    m_mainWindow->resize(640,800); // ### remove

    return QApplication::exec();
}
