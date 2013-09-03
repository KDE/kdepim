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

#include "folderarchivecache.h"
#include "folderarchiveaccountinfo.h"

FolderArchiveCache::FolderArchiveCache(QObject *parent)
    : QObject(parent)
{
}

FolderArchiveCache::~FolderArchiveCache()
{

}

void FolderArchiveCache::clearCache()
{
    mCache.clear();
}

void FolderArchiveCache::clearCacheWithContainsCollection(Akonadi::Collection::Id id)
{
    QHash<QString, ArchiveCache>::iterator i = mCache.begin();
    while (i!=mCache.end()) {
        if (i.value().colId == id) {
            i = mCache.erase(i);
        } else {
            ++i;
        }
    }
}

Akonadi::Collection::Id FolderArchiveCache::collectionId(FolderArchiveAccountInfo *info)
{
    if (mCache.contains(info->instanceName())) {
        switch(info->folderArchiveType()) {
        case FolderArchiveAccountInfo::UniqueFolder:
            return mCache.value(info->instanceName()).colId;
        case FolderArchiveAccountInfo::FolderByMonths:
            if (mCache.value(info->instanceName()).date.month() != QDate::currentDate().month()) {
                mCache.remove(info->instanceName());
                return -1;
            } else {
                return mCache.value(info->instanceName()).colId;
            }
        case FolderArchiveAccountInfo::FolderByYears:
            if (mCache.value(info->instanceName()).date.year() != QDate::currentDate().year()) {
                mCache.remove(info->instanceName());
                return -1;
            } else {
                return mCache.value(info->instanceName()).colId;
            }
            break;
        }
        return mCache.value(info->instanceName()).colId;
    }
    return -1;
}

void FolderArchiveCache::addToCache(const QString &resourceName, Akonadi::Collection::Id id)
{
    if (mCache.contains(resourceName)) {
        ArchiveCache cache = mCache.value(resourceName);
        cache.colId = id;
        mCache.insert(resourceName, cache);
    } else {
        ArchiveCache cache;
        cache.colId = id;
        mCache.insert(resourceName, cache);
    }
}

#include "folderarchivecache.moc"
