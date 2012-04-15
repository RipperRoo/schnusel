#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class MainWindow;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int argc, char *argv[]);
    ~Application();

    int exec();

private:
    MainWindow *m_mainWindow;
};

#endif // APPLICATION_H
