#ifndef PROFILESMANAGER_H
#define PROFILESMANAGER_H

#include <QSettings>
#include <QSharedPointer>
#include <QStringList>

class ProfilesManager
{
public:
    static ProfilesManager *instance();

    ProfilesManager();
    QStringList profileNames() const;
    QString defaultProfileName() const;
    void setDefaultProfileName(const QString &name);
    void add(const QString &name);
    void remove(const QString &name);
    bool rename(const QString &name, const QString &newName);
    bool clone(const QString &name, const QString &newName);
    QSharedPointer<QSettings> profile(const QString &name);

private:
    void initProfile(QSettings &s);
};

#endif // PROFILESMANAGER_H
