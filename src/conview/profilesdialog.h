#ifndef PROFILESDIALOG_H
#define PROFILESDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ProfilesDialog;
}
QT_END_NAMESPACE

class ProfilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfilesDialog(QWidget *parent = 0);
    ~ProfilesDialog();

private:
    void updateListWidget();

private slots:
    void on_btnNew_clicked();
    void on_btnRename_clicked();
    void on_btnClone_clicked();
    void on_btnDelete_clicked();
    void on_btnClose_clicked();
    void on_listWidget_doubleClicked();

private:
    Ui::ProfilesDialog *ui;
};

#endif // PROFILESDIALOG_H
