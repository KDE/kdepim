/***************************************************************************
                          knaccnewssettings.h  -  description
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


#ifndef KNACCNEWSSETTINGS_H
#define KNACCNEWSSETTINGS_H

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpixmap.h>

#include "knsettingswidget.h"

class QPushButton;

class KNAccountManager;
class KNListBox;
class KNNntpAccount;

class KNAccNewsSettings : public KNSettingsWidget  {

	Q_OBJECT	

	public:
		KNAccNewsSettings(QWidget *p, KNAccountManager *am);
		~KNAccNewsSettings();
				
		void addItem(KNNntpAccount *a);
		void removeItem(KNNntpAccount *a);
		
		QString name()  const  { return n_ame->text(); }
		QString server() const  { return s_erver->text(); }
		int port() const  { return p_ort->value(); }
		bool logonNeeded() const  { return logonCB->isChecked(); }
		QString user() const { return u_ser->text(); }
		QString pass() const { return p_ass->text(); }
				
	protected:
		void enableEdit(bool b);
		KNListBox *lb;
		QPushButton *addBtn, *delBtn, *okBtn;
		QLineEdit *n_ame, *s_erver, *u_ser, *p_ass;
		QSpinBox *p_ort;
		QCheckBox *logonCB;
		KNAccountManager *aManager;
		int currentItem;
		QPixmap pm;
		
	protected slots:
		void slotItemSelected(int id);
		void slotAddBtnClicked();
		void slotDelBtnClicked();
		void slotOkBtnClicked();
		void slotLogonChecked(bool b);
};

#endif
