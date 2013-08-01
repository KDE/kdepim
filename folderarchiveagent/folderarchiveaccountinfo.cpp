/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "folderarchiveaccountinfo.h"

#include <KConfigGroup>

FolderArchiveAccountInfo::FolderArchiveAccountInfo()
    : mArchiveType(UniqFolder),
      mArchiveTopLevelCollectionId(-1),
      mEnabled(false)
{
}

FolderArchiveAccountInfo::FolderArchiveAccountInfo(const KConfigGroup &config)
    : mArchiveType(UniqFolder),
      mArchiveTopLevelCollectionId(-1),
      mEnabled(false)
{
    readConfig(config);
}

FolderArchiveAccountInfo::~FolderArchiveAccountInfo()
{
}

bool FolderArchiveAccountInfo::isValid() const
{
    return (mArchiveTopLevelCollectionId > -1);
}

void FolderArchiveAccountInfo::setFolderArchiveType(FolderArchiveAccountInfo::FolderArchiveType type)
{
    mArchiveType = type;
}

FolderArchiveAccountInfo::FolderArchiveType FolderArchiveAccountInfo::folderArchiveType() const
{
    return mArchiveType;
}

void FolderArchiveAccountInfo::setArchiveTopLevel(Akonadi::Collection::Id id)
{
    mArchiveTopLevelCollectionId = id;
}

Akonadi::Collection::Id FolderArchiveAccountInfo::archiveTopLevel() const
{
    return mArchiveTopLevelCollectionId;
}

QString FolderArchiveAccountInfo::instanceName() const
{
    return mInstanceName;
}

void FolderArchiveAccountInfo::setInstanceName(const QString &instance)
{
    mInstanceName = instance;
}

void FolderArchiveAccountInfo::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

bool FolderArchiveAccountInfo::enabled() const
{
    return mEnabled;
}

void FolderArchiveAccountInfo::readConfig(const KConfigGroup &config)
{
    mInstanceName = config.readEntry(QLatin1String("instanceName"));
    mArchiveTopLevelCollectionId = config.readEntry(QLatin1String("topLevelCollectionId"), -1);
    mArchiveType = static_cast<FolderArchiveType>(config.readEntry("folderArchiveType", (int)UniqFolder));
    mEnabled = config.readEntry("enabled", false);
}

void FolderArchiveAccountInfo::writeConfig(KConfigGroup &config )
{
    config.writeEntry(QLatin1String("instanceName"), mInstanceName);
    config.writeEntry(QLatin1String("topLevelCollectionId"), mArchiveTopLevelCollectionId);
    config.writeEntry(QLatin1String("folderArchiveType"), (int)mArchiveType);
    config.writeEntry(QLatin1String("enabled"), mEnabled);
}
