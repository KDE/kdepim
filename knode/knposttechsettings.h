/***************************************************************************
                          knposttechsettings.h  -  description
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


#ifndef KNPOSTTECHSETTINGS_H
#define KNPOSTTECHSETTINGS_H

#include "knsettingswidget.h"

class QComboBox;
class QCheckBox;
class QListBox;
class QPushButton;
class QLineEdit;


class KNPostTechSettings : public KNSettingsWidget  {
	
	Q_OBJECT

	public:
		KNPostTechSettings(QWidget *p);
		~KNPostTechSettings();
		
		void init();
		void apply();
		
	protected:
		void enableEdit(bool e);
		QComboBox *charset, *encoding;
		QCheckBox *allow8bitCB, *genMIdCB;
		QListBox *lb;
		QPushButton *addBtn, *delBtn, *okBtn;
		QLineEdit *host, *hName, *hValue;
		bool saveHdrs, editEnabled;
		int currentItem;
		
	protected slots:
		void slotGenMIdCBtoggled(bool b);
		void slotAddBtnClicked();
		void slotDelBtnClicked();
		void slotOkBtnClicked();
		void slotItemSelected(int i);	

};

#endif
