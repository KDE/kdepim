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

#include "migrateconfig.h"

#include <QStandardPaths>
#include <Kdelibs4Migration>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace PimCommon;

MigrateConfig::MigrateConfig(const QString &appName)
    : mAppName(appName)
{
}

void MigrateConfig::setConfigFileNameList(const QStringList &configFileNameList)
{
    mConfigFileNameList = configFileNameList;
}

void MigrateConfig::setUiFileNameList(const QStringList &uiFileNameList)
{
    mUiFileNameList = uiFileNameList;
}

void MigrateConfig::start()
{
    // Testing for kdehome
    Kdelibs4Migration migration;
    if (!migration.kdeHomeFound()) {
        return;
    }

    Q_FOREACH( const QString &configFileName, mConfigFileNameList) {
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

    if (mAppName.isEmpty()) {
        qCritical() <<" We can not migrate ui file. AppName is missing";
    } else {
        Q_FOREACH( const QString &uiFileName, mUiFileNameList) {
            const QString newConfigLocation
                    = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                    QStringLiteral("/kxmlgui5/") + mAppName + QLatin1Char('/') + uiFileName;
            if (QFile(newConfigLocation).exists()) {
                continue;
            }
            QFileInfo fileInfo(newConfigLocation);
            QDir().mkpath(fileInfo.absolutePath());

            QString oldConfigFile(migration.locateLocal("data", mAppName + QLatin1Char('/') + uiFileName));
            if (!oldConfigFile.isEmpty()) {
                QFile(oldConfigFile).copy(newConfigLocation);
            }
        }
    }
}
