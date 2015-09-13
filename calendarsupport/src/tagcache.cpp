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

#include "tagcache.h"
#include "calendarsupport_debug.h"

#include <AkonadiCore/TagAttribute>
#include <AkonadiCore/TagFetchJob>
#include <AkonadiCore/TagFetchScope>
#include <AkonadiCore/Monitor>

using namespace CalendarSupport;

TagCache::TagCache()
    : QObject(),
      mMonitor(new Akonadi::Monitor(this))
{
    mMonitor->setTypeMonitored(Akonadi::Monitor::Tags);
    mMonitor->tagFetchScope().fetchAttribute<Akonadi::TagAttribute>();
    connect(mMonitor, &Akonadi::Monitor::tagAdded, this, &TagCache::onTagAdded);
    connect(mMonitor, &Akonadi::Monitor::tagRemoved, this, &TagCache::onTagRemoved);
    connect(mMonitor, &Akonadi::Monitor::tagChanged, this, &TagCache::onTagChanged);
    retrieveTags();
}

Akonadi::Tag TagCache::getTagByGid(const QByteArray &gid) const
{
    return mCache.value(mGidCache.value(gid));
}

void TagCache::onTagAdded(const Akonadi::Tag &tag)
{
    mCache.insert(tag.id(), tag);
    mGidCache.insert(tag.gid(), tag.id());
}

void TagCache::onTagChanged(const Akonadi::Tag &tag)
{
    onTagAdded(tag);
}

void TagCache::onTagRemoved(const Akonadi::Tag &tag)
{
    mCache.remove(tag.id());
    mGidCache.remove(tag.gid());
}

void TagCache::retrieveTags()
{
    Akonadi::TagFetchJob *tagFetchJob = new Akonadi::TagFetchJob(this);
    tagFetchJob->fetchScope().fetchAttribute<Akonadi::TagAttribute>();
    connect(tagFetchJob, &Akonadi::TagFetchJob::result, this, &TagCache::onTagsFetched);
}

void TagCache::onTagsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(CALENDARSUPPORT_LOG) << "Failed to fetch tags: " << job->errorString();
        return;
    }
    Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob *>(job);
    Q_FOREACH (const Akonadi::Tag &tag, fetchJob->tags()) {
        onTagAdded(tag);
    }
}
