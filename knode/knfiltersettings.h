/***************************************************************************
                          knfiltersettings.h  -  description
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


#ifndef KNFILTERSETTINGS_H
#define KNFILTERSETTINGS_H

#include "knsettingswidget.h"

class QPushButton;

class KNListBox;
class KNFilterManager;
class KNArticleFilter;


class KNFilterSettings : public KNSettingsWidget  {
	
	Q_OBJECT

	public:
		KNFilterSettings(KNFilterManager *fm, QWidget *p);
		~KNFilterSettings();
		
		void addItem(KNArticleFilter *f);
		void removeItem(KNArticleFilter *f);
		void updateItem(KNArticleFilter *f);
		void addMenuItem(KNArticleFilter *f);
		void removeMenuItem(KNArticleFilter *f);
		QValueList<int> menuOrder();
			
	protected:
		int findItem(KNListBox *l, KNArticleFilter *f);
		KNListBox *flb, *mlb;
		QPushButton *addBtn, *delBtn, *editBtn, *upBtn, *downBtn, *sepAddBtn, *sepRemBtn;
		KNFilterManager *fiManager;
		
	protected slots:
		void slotAddBtnClicked();
		void slotDelBtnClicked();
		void slotEditBtnClicked();
		void slotUpBtnClicked();
		void slotDownBtnClicked();
		void slotSepAddBtnClicked();
		void slotSepRemBtnClicked();
		void slotItemSelected(int i);
};

#endif
