/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

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
    i->setCollection( this );
    i->setLabelText( name() );
  }
}



void KNCollection::updateListItem()
{
  if ( l_istItem ) {
    l_istItem->setLabelText( name() );
  }
}
