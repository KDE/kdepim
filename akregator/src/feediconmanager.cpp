/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "feed.h"
#include "feediconmanager.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#include <tqdict.h>
#include <tqpixmap.h>
#include <tqvaluelist.h>

namespace Akregator {

class FeedIconManager::FeedIconManagerPrivate
{
    public:
    TQValueList<Feed*> registeredFeeds;
    TQDict<Feed> urlDict;
};

FeedIconManager *FeedIconManager::m_instance = 0;

static KStaticDeleter<FeedIconManager> feediconmanagersd;

FeedIconManager* FeedIconManager::self()
{
    if (!m_instance)
        m_instance = feediconmanagersd.setObject(m_instance, new FeedIconManager);
    return m_instance;
}

void FeedIconManager::fetchIcon(Feed* feed)
{
    if (!d->registeredFeeds.contains(feed))
    {
        d->registeredFeeds.append(feed);
        connect(feed, TQT_SIGNAL(signalDestroyed(TreeNode*)), this, TQT_SLOT(slotFeedDestroyed(TreeNode*)));
    }
    TQString iconURL = getIconURL(KURL(feed->xmlUrl()));
    d->urlDict.insert(iconURL, feed);
    loadIcon(iconURL);
}

FeedIconManager::FeedIconManager(TQObject * parent, const char *name)
:  TQObject(parent, name), DCOPObject("FeedIconManager"), d(new FeedIconManagerPrivate)
{
    connectDCOPSignal("kded",
                      "favicons", "iconChanged(bool, TQString, TQString)",
                      "slotIconChanged(bool, TQString, TQString)", false);
}


FeedIconManager::~FeedIconManager()
{
    delete d;
    d = 0;
}

void FeedIconManager::loadIcon(const TQString & url)
{
    KURL u(url);

    TQString iconFile = iconLocation(u);
    
    if (iconFile.isNull())
    {
        TQByteArray data;
        TQDataStream ds(data, IO_WriteOnly);
        ds << u;
        kapp->dcopClient()->send("kded", "favicons", "downloadHostIcon(KURL)",
                                 data);
    }
    else
        slotIconChanged(false, url, iconFile);

}

TQString FeedIconManager::getIconURL(const KURL& url)
{
    return "http://" +url.host() + "/";
}

TQString FeedIconManager::iconLocation(const KURL & url) const
{
    TQByteArray data, reply;
    TQCString replyType;
    TQDataStream ds(data, IO_WriteOnly);

    ds << url;

    kapp->dcopClient()->call("kded", "favicons", "iconForURL(KURL)", data,
                             replyType, reply);

    if (replyType == "TQString") {
        TQDataStream replyStream(reply, IO_ReadOnly);
        TQString result;
        replyStream >> result;
        return result;
    }

    return TQString::null;
}

void FeedIconManager::slotFeedDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
        while (d->registeredFeeds.contains(feed))
            d->registeredFeeds.remove(d->registeredFeeds.find(feed));
}

void FeedIconManager::slotIconChanged(bool /*isHost*/, const TQString& hostOrURL,
                                  const TQString& iconName)
{
    TQString iconFile = KGlobal::dirs()->findResource("cache",
                                 iconName+".png");
    Feed* f;
    TQPixmap p = TQPixmap(iconFile);
    if (!p.isNull()) // we don't set null pixmaps, as feed checks pixmap.isNull() to find out whether the icon was already loaded or not. It would request the icon another time, resulting an infinite loop (until stack overflow that is
    {
        while (( f = d->urlDict.take(hostOrURL) ))
            if (d->registeredFeeds.contains(f))
                f->setFavicon(p);
    }
    emit signalIconChanged(hostOrURL, iconFile);
}

} // namespace Akregator
#include "feediconmanager.moc"
