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
  KNLVItemBase(vi)
{
  coll=0;
}



KNCollectionViewItem::KNCollectionViewItem(KNLVItemBase *it) :
  KNLVItemBase(it)
{
  coll=0;
}



KNCollectionViewItem::~KNCollectionViewItem()
{
  if(coll) coll->setListItem(0);
}



QString KNCollectionViewItem::key(int c, bool) const
{
  if(coll->type()==KNCollection::CTfolder && c==0)
    return QString("\xff\xff\xff\xff");
  else return text(c);
}



bool KNCollectionViewItem::firstColBold()
{
  if(coll->type()==KNCollection::CTgroup)
    return ( ((KNGroup*)coll)->newCount()>0 );
  else return false;
}
