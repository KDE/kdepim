/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqmap.h>
#include <tqstylesheet.h>

#include <klocale.h>
#include <kstaticdeleter.h>

#include <libkdepim/progressmanager.h>

#include "feedlist.h"
#include "feed.h"
#include "treenode.h"

#include "progressmanager.h"

//#include <kdebug.h>

namespace Akregator {

class ProgressManager::ProgressManagerPrivate
{
    public:
        FeedList* feedList;
        TQMap<Feed*, ProgressItemHandler*> handlers;
    
};

static KStaticDeleter<ProgressManager> progressmanagersd;
ProgressManager* ProgressManager::m_self = 0;

ProgressManager* ProgressManager::self()
{
    if (!m_self)
        m_self = progressmanagersd.setObject(m_self, new ProgressManager);
    return m_self;
}

ProgressManager::ProgressManager() : d(new ProgressManagerPrivate)
{
    d->feedList = 0;
}

ProgressManager::~ProgressManager()
{
    delete d; 
    d = 0;
}

void ProgressManager::setFeedList(FeedList* feedList)
{
    if (feedList == d->feedList)
        return;

    if (d->feedList != 0)
    {
        for (TQMap<Feed*, ProgressItemHandler*>::ConstIterator it = d->handlers.begin(); it != d->handlers.end(); ++it)
            delete *it;
        d->handlers.clear();
        
        disconnect(d->feedList, TQT_SIGNAL(signalNodeAdded(TreeNode*)), this, TQT_SLOT(slotNodeAdded(TreeNode*)));
        disconnect(d->feedList, TQT_SIGNAL(signalNodeRemoved(TreeNode*)), this, TQT_SLOT(slotNodeRemoved(TreeNode*)));
    }

    d->feedList = feedList;
    
    if (feedList != 0)
    {
        TQValueList<TreeNode*> list = feedList->asFlatList();
    
        for (TQValueList<TreeNode*>::ConstIterator it = list.begin(); it != list.end(); ++it)
            slotNodeAdded(*it);
        connect(feedList, TQT_SIGNAL(signalNodeAdded(TreeNode*)), this, TQT_SLOT(slotNodeAdded(TreeNode*)));
        connect(feedList, TQT_SIGNAL(signalNodeRemoved(TreeNode*)), this, TQT_SLOT(slotNodeRemoved(TreeNode*)));
    }
}
     
void ProgressManager::slotNodeAdded(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
    {
        if (!d->handlers.contains(feed))
        d->handlers[feed] = new ProgressItemHandler(feed);
        connect(feed, TQT_SIGNAL(signalDestroyed(TreeNode*)), this, TQT_SLOT(slotNodeDestroyed(TreeNode*)));
    }
}

void ProgressManager::slotNodeRemoved(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
    {
        disconnect(feed, TQT_SIGNAL(signalDestroyed(TreeNode*)), this, TQT_SLOT(slotNodeDestroyed(TreeNode*)));
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

void ProgressManager::slotNodeDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
    {
        delete d->handlers[feed];
        d->handlers.remove(feed);
    }
}

class ProgressItemHandler::ProgressItemHandlerPrivate
{
    public:

        Feed* feed;
        KPIM::ProgressItem* progressItem;
};

ProgressItemHandler::ProgressItemHandler(Feed* feed) : d(new ProgressItemHandlerPrivate)
{
    d->feed = feed;
    d->progressItem = 0;
    
    connect(feed, TQT_SIGNAL(fetchStarted(Feed*)), this, TQT_SLOT(slotFetchStarted()));
    connect(feed, TQT_SIGNAL(fetched(Feed*)), this, TQT_SLOT(slotFetchCompleted()));
    connect(feed, TQT_SIGNAL(fetchError(Feed*)), this, TQT_SLOT(slotFetchError()));
    connect(feed, TQT_SIGNAL(fetchAborted(Feed*)), this, TQT_SLOT(slotFetchAborted()));
}

ProgressItemHandler::~ProgressItemHandler()
{
    if (d->progressItem)
    {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }

    delete d; 
    d = 0;
}

void ProgressItemHandler::slotFetchStarted()
{
    if (d->progressItem)
    {
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
    
    d->progressItem = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), TQStyleSheet::escape( d->feed->title() ), TQString::null, true);

    connect(d->progressItem, TQT_SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), d->feed, TQT_SLOT(slotAbortFetch()));
}


void ProgressItemHandler::slotFetchCompleted()
{
    if (d->progressItem)
    {
        d->progressItem->setStatus(i18n("Fetch completed"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void ProgressItemHandler::slotFetchError()
{
    if (d->progressItem)
    {
        d->progressItem->setStatus(i18n("Fetch error"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

void ProgressItemHandler::slotFetchAborted()
{
    if (d->progressItem)
    {
        d->progressItem->setStatus(i18n("Fetch aborted"));
        d->progressItem->setComplete();
        d->progressItem = 0;
    }
}

} // namespace Akregator

#include "progressmanager.moc"
