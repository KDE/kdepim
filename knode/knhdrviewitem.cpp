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


QString KNHdrViewItem::key(int col, bool) const
{
  if ((col==2)||(col==3)) {   // score, lines
    QString tmpString;
    return tmpString.sprintf("%08d",text(col).toInt());
  }
  if(col==4) {
    time_t date = art->date()->unixTime();

    if(static_cast<KNListView*>(listView())->sortByThreadChangeDate() &&
       (static_cast<KNRemoteArticle*>(art)->subThreadChangeDate() > date)) {
      date = static_cast<KNRemoteArticle*>(art)->subThreadChangeDate();
    }

    return QString::number((uint)date).rightJustify(15, '0');
  }
  return text(col);
}

int KNHdrViewItem::compare(QListViewItem *i, int col, bool ascending) const
{
  return key(col, ascending).localeAwareCompare(i->key(col, ascending));
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

