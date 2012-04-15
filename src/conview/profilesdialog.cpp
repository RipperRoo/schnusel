#include "profilesdialog.h"
#include "ui_profilesdialog.h"
#include "profilesmanager.h"
#include <QInputDialog>

ProfilesDialog::ProfilesDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ProfilesDialog)
{
    ui->setupUi(this);
    updateListWidget();
}

ProfilesDialog::~ProfilesDialog()
{
    delete ui;
}

void ProfilesDialog::updateListWidget()
{
    QModelIndex index = ui->listWidget->currentIndex();
    ui->listWidget->clear();
    QString defaultProfileName = ProfilesManager::instance()->defaultProfileName();
    foreach (const QString &name, ProfilesManager::instance()->profileNames()) {
        QListWidgetItem *item = new QListWidgetItem(name, ui->listWidget);
        if (defaultProfileName == name) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
    }
    ui->listWidget->setCurrentIndex(index);
}

void ProfilesDialog::on_btnNew_clicked()
{
    QString name = QInputDialog::getText(this, tr("Add Profile"), tr("New Profile Name:"));
    if (name.isNull())
        return;
    ProfilesManager::instance()->add(name);
    updateListWidget();
}

void ProfilesDialog::on_btnRename_clicked()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item)
        return;
    QString name = item->text();
    QString newName = QInputDialog::getText(this, tr("Rename Profile %1").arg(name),
                                            tr("New Profile Name:"));
    if (newName.isNull())
        return;
    ProfilesManager::instance()->rename(name, newName);
    updateListWidget();
}

void ProfilesDialog::on_btnClone_clicked()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item)
        return;
    QString name = item->text();
    QString newName = QInputDialog::getText(this, tr("Clone Profile %1").arg(name),
                                            tr("New Profile Name:"));
    if (newName.isNull())
        return;
    ProfilesManager::instance()->clone(name, newName);
    updateListWidget();
}

void ProfilesDialog::on_btnDelete_clicked()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item)
        return;
    ProfilesManager::instance()->remove(item->text());
    updateListWidget();
}

void ProfilesDialog::on_btnClose_clicked()
{
    accept();
}

void ProfilesDialog::on_listWidget_doubleClicked()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item)
        return;
    ProfilesManager::instance()->setDefaultProfileName(item->text());
    updateListWidget();
}
