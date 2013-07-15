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
    : mArchiveTopLevelCollectionId(-1)
{
}

FolderArchiveAccountInfo::FolderArchiveAccountInfo(const KConfigGroup &config)
    : mArchiveTopLevelCollectionId(-1)
{
    readConfig(config);
}

FolderArchiveAccountInfo::~FolderArchiveAccountInfo()
{
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

void FolderArchiveAccountInfo::readConfig(const KConfigGroup &config)
{
    mInstanceName = config.readEntry(QLatin1String("instanceName"));
    mArchiveTopLevelCollectionId = config.readEntry(QLatin1String("toplevelid"), -1);
    //TODO
}

void FolderArchiveAccountInfo::writeConfig(KConfigGroup &config )
{
    config.writeEntry(QLatin1String("instanceName"), mInstanceName);
    config.writeEntry(QLatin1String("toplevelid"), mArchiveTopLevelCollectionId);
    //TODO
}
