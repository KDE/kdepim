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

#include "gravatarcache.h"
#include <QDir>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
using namespace PimCommon;

Q_GLOBAL_STATIC(GravatarCache, s_gravatarCache)

GravatarCache::GravatarCache()
    : mMaximumSize(20)
{
    mCachePixmap.setMaxCost(mMaximumSize);
    //Make sure that this folder is created. Otherwise we can't store gravatar
    mGravatarPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/gravatar/");
    QDir().mkpath(mGravatarPath);
}

GravatarCache::~GravatarCache()
{

}

GravatarCache *GravatarCache::self()
{
    return s_gravatarCache;
}

void GravatarCache::saveGravatarPixmap(const QString &hashStr, const QPixmap &pixmap)
{
    if (!hashStr.isEmpty() && !pixmap.isNull()) {
        if (!mCachePixmap.contains(hashStr)) {
            const QString path = mGravatarPath + hashStr + QLatin1String(".png");
            //qDebug() << " path " << path;
            if (pixmap.save(path)) {
                //qDebug() <<" saved in cache "<< hashStr << path;
                mCachePixmap.insert(hashStr, new QPixmap(pixmap));
            }
        }
    }
}

QPixmap GravatarCache::loadGravatarPixmap(const QString &hashStr, bool &gravatarStored)
{
    gravatarStored = false;
    //qDebug()<<" hashStr"<<hashStr;
    if (!hashStr.isEmpty()) {
        if (mCachePixmap.contains(hashStr)) {
            //qDebug()<<" contains in cache "<< hashStr;
            gravatarStored = true;
            return *(mCachePixmap.object(hashStr));
        } else {
            const QString path = mGravatarPath + hashStr + QLatin1String(".png");
            QFileInfo fi(path);
            if (fi.exists()) {
                QPixmap pix;
                if (pix.load(path)) {
                    //qDebug() << " add to cache "<<hashStr << path;
                    mCachePixmap.insert(hashStr, new QPixmap(pix));
                    gravatarStored = true;
                    return pix;
                }
            } else {
                return QPixmap();
            }
        }
    }
    return QPixmap();
}

int GravatarCache::maximumSize() const
{
    return mMaximumSize;
}

void GravatarCache::setMaximumSize(int maximumSize)
{
    if (mMaximumSize != maximumSize) {
        mMaximumSize = maximumSize;
        mCachePixmap.setMaxCost(mMaximumSize);
    }
}

void GravatarCache::clear()
{
    mCachePixmap.clear();
}

void GravatarCache::clearAllCache()
{
    const QString path = mGravatarPath;
    if (!path.isEmpty()) {
        QDir dir(path);
        if (dir.exists()) {
            const QFileInfoList list = dir.entryInfoList();  // get list of matching files and delete all
            Q_FOREACH (const QFileInfo &it, list) {
                dir.remove(it.fileName());
            }
        }
    }
    clear();
}

