/*
    knhdrviewitem.cpp

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

#include "knhdrviewitem.h"
#include "knmime.h"
#include <stdio.h>


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
  if(col==3) {
    QString tmpString;
    return tmpString.sprintf("%08d",(uint)art->date()->unixTime());
  } else
    return text(col);
}



bool KNHdrViewItem::greyOut()
{
  if(art->type()==KNMimeBase::ATremote)
    return (  !((KNRemoteArticle*)art)->hasUnreadFollowUps() &&
              ((KNRemoteArticle*)art)->isRead() );
  else return false;  
}



bool KNHdrViewItem::firstColBold()
{
	if(art->type()==KNMimeBase::ATremote)
		return ( static_cast<KNRemoteArticle*>(art)->isNew() );
	else
		return false;
}

