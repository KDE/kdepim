/***************************************************************************
                     knlvitembase.h - description
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

#ifndef KNLVITEMBASE_H
#define KNLVITEMBASE_H

#include <knlistview.h>


class KNLVItemBase : public QListViewItem  {
	
	public:
		enum pixmapType { 	PTgreyBall=0, PTredBall=1, PTgreyBallChkd=2,
												PTredBallChkd=3, PTnewFups=4, PTeyes=5,
												PTmail=6, PTposting=7, PTcontrol=8,
                        PTstatusSent=9,	PTstatusEdit=10,
                        PTstatusCanceled=11, PTnntp=12,
                        PTgroup=13, PTfolder=14, PTnull=15 };
												
		KNLVItemBase(KNListView *view);      // restricted to KNListView to prevent that the
		KNLVItemBase(KNLVItemBase *item);    // static_cast in ~KNLVItemBase fails. (single selection in multi-mode hack)
		~KNLVItemBase();
		
		void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
		int width(const QFontMetrics &fm, const QListView *lv, int column);
		void paintFocus(QPainter *, const QColorGroup & cg, const QRect & r);
		void setOpen(bool o);
		void sortChildItems(int column, bool a);
		
		static void initIcons();
		static void clearIcons();
		static void setTotalExpand(bool b)	{ totalExpand=b; }		
		static QPixmap& icon(pixmapType t);
		
	protected:
		virtual bool greyOut()					{ return false; }
		virtual bool firstColBold()     { return false; }
	  static bool totalExpand;
		static QPixmap *pms[15];
};

#endif









