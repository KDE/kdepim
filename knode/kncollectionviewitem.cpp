/***************************************************************************
                          kncollectionviewitem.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


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
