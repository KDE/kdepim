/***************************************************************************
                     knhdrviewitem.h - description
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

#ifndef KNHDRVIEWITEM_H
#define KNHDRVIEWITEM_H

#include "knlvitembase.h"
#include "knarticle.h"



class KNHdrViewItem : public KNLVItemBase  {
	
	public:
		KNHdrViewItem(KNListView *ref, KNArticle *a=0);
		KNHdrViewItem(KNLVItemBase *ref, KNArticle *a=0);
		~KNHdrViewItem();
		QString key(int, bool) const;
		KNArticle *art;
		
	protected:
		bool greyOut();
		bool firstColBold();
		
};

#endif




