#include "profilesmanager.h"

Q_GLOBAL_STATIC(ProfilesManager, theInstance)

ProfilesManager *ProfilesManager::instance()
{
    return theInstance();
}

ProfilesManager::ProfilesManager()
{
}

QStringList ProfilesManager::profileNames() const
{
    QSettings s;
    s.beginGroup("profiles");
    return s.childGroups();
}

QString ProfilesManager::defaultProfileName() const
{
    QSettings s;
    s.beginGroup("profiles");
    return s.value("default", "default").toString();
}

void ProfilesManager::setDefaultProfileName(const QString &name)
{
    QSettings s;
    s.beginGroup("profiles");
    s.setValue("default", name);
    s.endGroup();
}

void ProfilesManager::add(const QString &name)
{
    QSettings s;
    s.beginGroup("profiles");
    s.beginGroup(name);
    initProfile(s);
    s.endGroup();
}

void ProfilesManager::remove(const QString &name)
{
    QSettings s;
    s.beginGroup("profiles");
    s.remove(name);
}

bool ProfilesManager::rename(const QString &name, const QString &newName)
{
    if (!clone(name, newName))
        return false;
    remove(name);
    return true;
}

bool ProfilesManager::clone(const QString &name, const QString &newName)
{
    if (newName.isEmpty() || name == newName)
        return false;
    QSettings s;
    s.beginGroup("profiles");
    s.beginGroup(name);
    QStringList keys = s.allKeys();
    s.endGroup();
    foreach (const QString &key, keys) {
        QVariant v = s.value(name + QLatin1Char('/') + key);
        s.setValue(newName + QLatin1Char('/') + key, v);
    }
    return true;
}

QSharedPointer<QSettings> ProfilesManager::profile(const QString &name)
{
    QSharedPointer<QSettings> s(new QSettings);
    s->beginGroup("profiles");
    s->beginGroup(name);
    return s;
}

void ProfilesManager::initProfile(QSettings &s)
{
    s.setValue("opacity", 100);
}
