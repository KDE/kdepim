/*
    kncollectionviewitem.cpp

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdragobject.h>

#include "kncollectionviewitem.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knglobals.h"
#include "knconfigmanager.h"


KNCollectionViewItem::KNCollectionViewItem(KNListView *vi) :
  KNLVItemBase(vi), coll(0)
{
  num[0]=num[1]=num[2]=-1;
}


KNCollectionViewItem::KNCollectionViewItem(KNLVItemBase *it) :
  KNLVItemBase(it), coll(0)
{
  num[0]=num[1]=num[2]=-1;
}


KNCollectionViewItem::~KNCollectionViewItem()
{
  if(coll) coll->setListItem(0);
}


void KNCollectionViewItem::setNumber(int column, int number)
{
  if ((column >= 1)&&(column <=2)) {
    setText(column, QString::number(number));
    num[column]=number;
  }
}


QString KNCollectionViewItem::key(int c, bool ascending) const
{
  if (!coll)
    return text(c);

  QString prefix;

  if (coll->type()==KNCollection::CTfolder) {   // folders should be always on the bottom
    if ((static_cast<KNFolder*>(coll))->isStandardFolder())  // put the standard folders above the custom folders
      prefix = (ascending)? QString("ba"):QString("ab");
    else
      prefix = (ascending)? QString("bb"):QString("aa");
  } else
    prefix = (ascending)? QString("aa"):QString("bb");

  if ((c >= 1)&&(c <= 2)&&(num[c] != -1)) {
    QString tmpString;
    return prefix+tmpString.sprintf("%07d", num[c]);
  } else
    return prefix+text(0);

}


QDragObject* KNCollectionViewItem::dragObject() const
{
  if (coll && coll->type()==KNCollection::CTfolder) {
    if ((static_cast<KNFolder*>(coll))->isStandardFolder())
      return 0;

    QDragObject *d=new QStoredDrag("x-knode-drag/folder", listView()->viewport());
    d->setPixmap(knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::customFolder));
    return d;
  }
  return 0;
}


bool KNCollectionViewItem::acceptDrag(QDropEvent* event) const
{
  if (event && coll && coll->type()==KNCollection::CTfolder) {
    if (event->provides("x-knode-drag/article"))
      return !(static_cast<KNFolder*>(coll)->isRootFolder());   // don't drop articles on the root folder
    else if (event->provides("x-knode-drag/folder"))
      return !isSelected();             // don't drop on itself
  }

  return false;
}


bool KNCollectionViewItem::firstColBold()
{
  if (coll && coll->type()==KNCollection::CTgroup)
    return ( (static_cast<KNGroup*>(coll))->newCount()>0 );
  else return false;
}


QString KNCollectionViewItem::shortString(QString text, int col, int width, QFontMetrics fm)
{
  if (coll && coll->type()==KNCollection::CTgroup) {
    QString t(text);
    int curPos=0,nextPos=0;
    QString temp;
    while ((fm.width(t) > width)&&(nextPos!=-1)) {
      nextPos = t.find('.',curPos);
      if (nextPos!=-1) {
        temp = t[curPos];
        t.replace(curPos,nextPos-curPos,temp);
        curPos+=2;
      }
    }
    return t;
  } else
    return KNLVItemBase::shortString(text,col,width,fm);
}
