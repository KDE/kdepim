/***************************************************************************
                     knlistbox.h - description
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

#ifndef KNLISTBOX_H
#define KNLISTBOX_H

#include "qlistbox.h"

class QPixMap;


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


//==============================================================================


class KNListBox : public QListBox  {
	
	public:
		KNListBox(QWidget *parent=0, const char *name=0);
		~KNListBox();
		
		KNLBoxItem *itemAt(int idx) {	return static_cast<KNLBoxItem*>(item(idx)); }
};


#endif

