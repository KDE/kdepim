/*
    kncollectionviewitem.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <config.h>
#include "kncollectionviewitem.h"
#include "kngroup.h"
#include "knnntpaccount.h"


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
  QString prefix;

  if (coll->type()==KNCollection::CTfolder)    // folders should be always on the bottom
    prefix = (ascending)? QString("b"):QString("a");
  else
    prefix = (ascending)? QString("a"):QString("b");

  if ((c >= 1)&&(c <= 2)&&(num[c] != -1)) {
     QString tmpString;
     return prefix+tmpString.sprintf("%07d", num[c]);
  } else
    return prefix+text(0);

}


bool KNCollectionViewItem::firstColBold()
{
  if(coll->type()==KNCollection::CTgroup)
    return ( ((KNGroup*)coll)->newCount()>0 );
  else return false;
}


QString KNCollectionViewItem::shortString(QString text, int col, int width, QFontMetrics fm)
{
  if ((col!=0) || !(coll->type()==KNCollection::CTgroup))
    return KNLVItemBase::shortString(text,col,width,fm);
  else {
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
  }
}
