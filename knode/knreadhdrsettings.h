/***************************************************************************
                          knreadhdrsettings.h  -  description
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


#ifndef KNREADHDRSETTINGS_H
#define KNREADHDRSETTINGS_H


#include "knsettingswidget.h"

class QLineEdit;
class QCheckBox;
class QPushButton;

class KNListBox;


class KNReadHdrSettings : public KNSettingsWidget  {
	
	Q_OBJECT

	public:
		KNReadHdrSettings(QWidget *p);
		~KNReadHdrSettings();
		
		void apply();
		
	protected:
		void init();
		void enableEdit(bool b);
		void createCBs(QWidget *p, QCheckBox **ptrs);
		
		KNListBox *lb;
		QLineEdit *name, *hdr;
		QCheckBox *nameCB[4], *hdrCB[4];
		QPushButton *addBtn, *delBtn, *upBtn, *downBtn, *okBtn;
		int currentItem;
		bool save;
		
	protected slots:
		void slotItemSelected(int i);
		void slotAddBtnClicked();
		void slotDelBtnClicked();
		void slotUpBtnClicked();
		void slotDownBtnClicked();
		void slotOkBtnClicked();

};

#endif
