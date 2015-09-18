/*
  Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef CALENDARSUPPORT_TAGCACHE_H
#define CALENDARSUPPORT_TAGCACHE_H

#include <AkonadiCore/Tag>

namespace Akonadi
{
class Monitor;
}
class KJob;

namespace CalendarSupport
{

/**
 * A tag cache
 */
class TagCache : public QObject
{
    Q_OBJECT
public:
    TagCache();
    Akonadi::Tag getTagByGid(const QByteArray &gid) const;

private Q_SLOTS:
    void onTagAdded(const Akonadi::Tag &);
    void onTagChanged(const Akonadi::Tag &);
    void onTagRemoved(const Akonadi::Tag &);
    void onTagsFetched(KJob *);

private:
    void retrieveTags();

    QHash<Akonadi::Tag::Id, Akonadi::Tag> mCache;
    QHash<QByteArray, Akonadi::Tag::Id> mGidCache;
    Akonadi::Monitor *mMonitor;
};

}

#endif
