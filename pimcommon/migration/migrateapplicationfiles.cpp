/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "migrateapplicationfiles.h"
#include <kdelibs4migration.h>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>

using namespace PimCommon;

MigrateApplicationFiles::MigrateApplicationFiles(QObject *parent)
    : QObject(parent),
      mMigrateApplicationVersion(1),
      mCurrentConfigVersion(0)
{

}

MigrateApplicationFiles::~MigrateApplicationFiles()
{

}

bool MigrateApplicationFiles::start()
{
    if (mMigrateInfoList.isEmpty()) {
        Q_EMIT migrateDone();
        return false;
    }

    if (mConfigFileName.isEmpty()) {
        qDebug() << " config file name not defined.";
        Q_EMIT migrateDone();
        return false;
    }
    // Testing for kdehome
    Kdelibs4Migration migration;
    if (!migration.kdeHomeFound()) {
        Q_EMIT migrateDone();
        return false;
    }
    return migrateConfig();
}

bool MigrateApplicationFiles::migrateConfig()
{
    Q_FOREACH(const MigrateFileInfo &info, mMigrateInfoList) {
        if (info.version() > mCurrentConfigVersion) {
            if (info.folder()) {
                migrateFolder(info);
            } else {
                migrateFile(info);
            }
        }
    }
    Q_EMIT migrateDone();
    return true;
}

int MigrateApplicationFiles::currentConfigVersion() const
{
    return mCurrentConfigVersion;
}

void MigrateApplicationFiles::setCurrentConfigVersion(int currentConfigVersion)
{
    mCurrentConfigVersion = currentConfigVersion;
}


QString MigrateApplicationFiles::configFileName() const
{
    return mConfigFileName;
}

void MigrateApplicationFiles::setConfigFileName(const QString &configFileName)
{
    mConfigFileName = configFileName;
}

void MigrateApplicationFiles::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(mConfigFileName, KConfig::SimpleConfig);
    KConfigGroup grp = config->group(QStringLiteral("Migrate"));
    grp.writeEntry(QStringLiteral("Version"), mMigrateApplicationVersion);
    grp.sync();
}

void MigrateApplicationFiles::migrateFolder(const MigrateFileInfo &info)
{
}

void MigrateApplicationFiles::migrateFile(const MigrateFileInfo &info)
{
}

int MigrateApplicationFiles::version() const
{
    return mMigrateApplicationVersion;
}

void MigrateApplicationFiles::setVersion(int version)
{
    mMigrateApplicationVersion = version;
}

bool MigrateApplicationFiles::checkIfNecessary()
{
    if (mConfigFileName.isEmpty()) {
        qDebug() << " config file name not defined.";
        return false;
    }
    KSharedConfig::Ptr config = KSharedConfig::openConfig(mConfigFileName, KConfig::SimpleConfig);
    if (config->hasGroup(QStringLiteral("Migrate"))) {
        KConfigGroup grp = config->group(QStringLiteral("Migrate"));
        mCurrentConfigVersion = grp.readEntry(QStringLiteral("Version"), 0);
        if (mCurrentConfigVersion < mMigrateApplicationVersion) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void MigrateApplicationFiles::insertMigrateInfo(const MigrateFileInfo &info)
{
    if (info.isValid()) {
        mMigrateInfoList.append(info);
    }
}

