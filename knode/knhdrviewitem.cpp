/*
    knhdrviewitem.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qdragobject.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knhdrviewitem.h"
#include "knarticle.h"


KNHdrViewItem::KNHdrViewItem(KNListView *ref, KNArticle *a) :
  KNLVItemBase(ref), art(a)
{
}


KNHdrViewItem::KNHdrViewItem(KNLVItemBase *ref, KNArticle *a) :
  KNLVItemBase(ref), art(a)
{
}


KNHdrViewItem::~KNHdrViewItem()
{
  if(art) art->setListItem(0);
}


int KNHdrViewItem::compare(QListViewItem *i, int col, bool) const
{
  KNArticle *otherArticle = static_cast<KNHdrViewItem*>(i)->art;
  int diff = 0;
  time_t date1=0, date2=0;

  switch (col) {
    case 0:
    case 1:
       return text(col).localeAwareCompare(i->text(col));

    case 2:
       if(art->type()==KMime::Base::ATremote) {
         diff = static_cast<KNRemoteArticle*>(art)->score() - static_cast<KNRemoteArticle*>(otherArticle)->score();
         return (diff < 0 ? -1 : diff > 0 ? 1 : 0);
       } else
         return 0;

    case 3:
       diff = art->lines()->numberOfLines() - otherArticle->lines()->numberOfLines();
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    case 4:
       date1 = art->date()->unixTime();
       date2 = otherArticle->date()->unixTime();
       if(art->type()==KMime::Base::ATremote && static_cast<KNListView*>(listView())->sortByThreadChangeDate()) {
         if(static_cast<KNRemoteArticle*>(art)->subThreadChangeDate() > date1)
           date1 = static_cast<KNRemoteArticle*>(art)->subThreadChangeDate();
         if(static_cast<KNRemoteArticle*>(otherArticle)->subThreadChangeDate() > date2)
           date2 = static_cast<KNRemoteArticle*>(otherArticle)->subThreadChangeDate();
       }
       diff = date1 - date2;
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    default:
       return 0;
  }
}


QDragObject* KNHdrViewItem::dragObject()
{
  QDragObject *d=new QStoredDrag( "x-knode-drag/article" , listView()->viewport());
  d->setPixmap(knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::posting));
  return d;
}


int KNHdrViewItem::countUnreadInThread()
{
  int count = 0;
  if(knGlobals.cfgManager->readNewsGeneral()->showUnread()) {
    if(art->type()==KMime::Base::ATremote) {
      count = static_cast<KNRemoteArticle*>(art)->unreadFollowUps();
    }
  }
  return count;
}


bool KNHdrViewItem::greyOut()
{
  if(art->type()==KMime::Base::ATremote)
    return (  !((KNRemoteArticle*)art)->hasUnreadFollowUps() &&
              ((KNRemoteArticle*)art)->isRead() );
  else return false;
}


bool KNHdrViewItem::firstColBold()
{
  if(art->type()==KMime::Base::ATremote)
    return ( static_cast<KNRemoteArticle*>(art)->isNew() );
  else
    return false;
}


QColor KNHdrViewItem::normalColor()
{
  if (art->type()==KMime::Base::ATremote) {
    KNRemoteArticle *rart = static_cast<KNRemoteArticle*>(art);
    return rart->color();
  }
  else
    return knGlobals.cfgManager->appearance()->unreadThreadColor();
}


QColor KNHdrViewItem::greyColor()
{
  return knGlobals.cfgManager->appearance()->readThreadColor();
}

