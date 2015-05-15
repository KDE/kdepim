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

#ifndef GRAVATARCACHE_H
#define GRAVATARCACHE_H

#include <QPixmap>
#include <QCache>
namespace PimCommon {
class GravatarCache
{
public:
    static GravatarCache *self();


    GravatarCache();
    ~GravatarCache();

    void saveGravatarPixmap(const QString &hashStr, const QPixmap &pixmap);

    QPixmap loadGravatarPixmap(const QString &hashStr, bool &gravatarStored);

    int maximumSize() const;
    void setMaximumSize(int maximumSize);

private:
    QCache<QString, QPixmap> mCachePixmap;
    int mMaximumSize;
};
}

#endif // GRAVATARCACHE_H
