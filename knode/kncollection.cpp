/***************************************************************************
                          kncollection.cpp  -  description
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


#include "kncollection.h"
#include "kncollectionviewitem.h"

KNCollection::KNCollection(KNCollection *p)
{
  p_arent=p;
  l_istItem=0;
  c_ount=0;
}



KNCollection::~KNCollection()
{
  delete l_istItem;
}



void KNCollection::setListItem(KNCollectionViewItem *i)
{
  l_istItem=i;
  if(i) {
    i->coll=this;
    i->setText(0, name());
  }
}



void KNCollection::updateListItem()
{
  if(l_istItem) l_istItem->setText(0, n_ame);
}
