/***************************************************************************
                     knhdrviewitem.cpp - description
 copyright            : (C) 1999 by Christian Thurner
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

#include "knhdrviewitem.h"
#include "knfetcharticle.h"
#include <stdio.h>

KNHdrViewItem::KNHdrViewItem(KNListView *ref, KNArticle *a) :
	KNLVItemBase(ref)
{
	art=a;
}



KNHdrViewItem::KNHdrViewItem(KNLVItemBase *ref, KNArticle *a) :
	KNLVItemBase(ref)
{
	art=a;
}



KNHdrViewItem::~KNHdrViewItem()
{
	if(art) art->setListItem(0);
}



QString KNHdrViewItem::key(int col, bool asc) const
{
	if(col==3)
      return QString::number((uint) art->timeT());
	
	else return this->text(col);
	
}



bool KNHdrViewItem::greyOut()
{
	if(art->type()==KNArticleBase::ATfetch)
		return ( 	!((KNFetchArticle*)art)->hasUnreadFollowUps() &&
						 	((KNFetchArticle*)art)->isRead() );
	else return false;	
}



bool KNHdrViewItem::firstColBold()
{
	return art->isNew();
}




