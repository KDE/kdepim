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

#include "knlboxitem.h"



class KNListBox : public QListBox  {
	
	public:
		KNListBox(QWidget *parent=0, const char *name=0);
		~KNListBox();
		
		KNLBoxItem *itemAt(int idx) {	return (KNLBoxItem*) item(idx); }
};

#endif

