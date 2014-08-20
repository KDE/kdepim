/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "kdelibs4configmigrater.h"

#include <QStandardPaths>
#include <Kdelibs4Migration>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace PimCommon;

class Kdelibs4ConfigMigrater::Private
{
public:
    Private(const QString &appName)
        : appName(appName)
    {

    }

    QStringList configFiles;
    QStringList uiFiles;
    QString appName;
};

Kdelibs4ConfigMigrater::Kdelibs4ConfigMigrater(const QString &appName)
    : d(new Private(appName))
{
}

Kdelibs4ConfigMigrater::~Kdelibs4ConfigMigrater()
{
    delete d;
}

void Kdelibs4ConfigMigrater::setConfigFiles(const QStringList &configFileNameList)
{
    d->configFiles = configFileNameList;
}

void Kdelibs4ConfigMigrater::setUiFiles(const QStringList &uiFileNameList)
{
    d->uiFiles = uiFileNameList;
}

bool Kdelibs4ConfigMigrater::migrate()
{
    // Testing for kdehome
    Kdelibs4Migration migration;
    if (!migration.kdeHomeFound()) {
        return false;
    }

    Q_FOREACH( const QString &configFileName, d->configFiles) {
        const QString newConfigLocation
                = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                + QLatin1Char('/') + configFileName;

        if (QFile(newConfigLocation).exists()) {
            continue;
        }
        //Be safe
        QFileInfo fileInfo(newConfigLocation);
        QDir().mkpath(fileInfo.absolutePath());

        QString oldConfigFile(migration.locateLocal("config", configFileName));
        if (!oldConfigFile.isEmpty()) {
            QFile(oldConfigFile).copy(newConfigLocation);
        }
    }

    if (d->appName.isEmpty()) {
        qCritical() <<" We can not migrate ui file. AppName is missing";
    } else {
        Q_FOREACH( const QString &uiFileName, d->uiFiles) {
            const QString newConfigLocation
                    = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                    QStringLiteral("/kxmlgui5/") + d->appName + QLatin1Char('/') + uiFileName;
            if (QFile(newConfigLocation).exists()) {
                continue;
            }
            QFileInfo fileInfo(newConfigLocation);
            QDir().mkpath(fileInfo.absolutePath());

            QString oldConfigFile(migration.locateLocal("data", d->appName + QLatin1Char('/') + uiFileName));
            if (!oldConfigFile.isEmpty()) {
                QFile(oldConfigFile).copy(newConfigLocation);
            }
        }
    }
    return true;
}
