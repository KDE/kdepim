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

#include <kglobal.h>
#include <kcharsets.h>

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
  if (col==4) {               // date
    QString tmpString;
    return tmpString.sprintf("%08d",(uint)art->date()->unixTime());
  }
  return text(col);
}


QDragObject* KNHdrViewItem::dragObject() const
{
  QDragObject *d=new QStoredDrag( "x-knode-drag/article" , listView()->viewport());
  d->setPixmap(knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::posting));
  return d;
}


QCache<QFont> KNHdrViewItem::f_ontCache;

void KNHdrViewItem::clearFontCache()
{
  f_ontCache.setAutoDelete(true);
  f_ontCache.clear();
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


const QFont& KNHdrViewItem::fontForColumn(int col, const QFont &font)
{
  if (col>1 || knGlobals.cfgManager->appearance()->useFontsForAllCS())
    return font;

  QFont *f=0;
  QCString cs;
  if(col==0) {
    cs = art->subject()->rfc2047Charset();
  }
  else {
    if(art->type()==KMime::Base::ATremote)
      cs = art->from()->rfc2047Charset();
    else
      cs = art->to()->rfc2047Charset();
  }

  // check if we already have a suitable font in the cache
  f=f_ontCache.find(cs);
  if (f) return (*f);

  // new charset...
  f = new QFont(knGlobals.cfgManager->appearance()->articleListFont());
  KGlobal::charsets()->setQFont(*f, KGlobal::charsets()->charsetForEncoding(cs));
  f_ontCache.setAutoDelete(true);
  f_ontCache.insert(cs, f);

  return (*f);
}
