/***************************************************************************
                     knlboxitem.h - description
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

#ifndef KNLBOXITEM_H
#define KNLBOXITEM_H

#include <qlistbox.h>
#include <qpixmap.h>


class KNLBoxItem : public QListBoxItem  {
	
	public:
		KNLBoxItem(const QString& text, void *d=0, QPixmap *_pm=0);
		~KNLBoxItem();
		
		void setPixmap(QPixmap _pm) { pm=_pm; }
		void* data() {	return mData; }
		void setData(void *d)	{ mData=d; }		
		
	private:
		QPixmap pm;
		void *mData;
			
	protected:
		virtual void paint( QPainter * );
		virtual int height( const QListBox * ) const;
		virtual int width( const QListBox * ) const;
};

#endif







