// -*- C++ -*-
/* addressEditor.h		KPilot
**
** Copyright (C) 1998-2000 by Dan Pilone
**
** This is a dialog window that is used to edit a single address record.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_ADDRESSEDITOR_H
#define _KPILOT_ADDRESSEDITOR_H

#include <kdialogbase.h>

class QLineEdit;
class PilotAddress;
struct AddressAppInfo;

class AddressEditor : public KDialogBase 
{
	Q_OBJECT


public:
	AddressEditor(PilotAddress *address,
		struct AddressAppInfo *appInfo,
		QWidget *parent, const char *name=0L);
	~AddressEditor();

 
signals:
	void recordChangeComplete ( PilotAddress* );

public slots:
	void slotOk();
	void slotCancel();
	void updateRecord(PilotAddress *);

private:
	bool fDeleteOnCancel;

	PilotAddress* fAddress;
	struct AddressAppInfo *fAppInfo;
	// entry fields
	QLineEdit *fCustom4Field;
	QLineEdit *fCustom3Field;
	QLineEdit *fCustom2Field;
	QLineEdit *fCustom1Field;
	QLineEdit *fCountryField;
	QLineEdit *fZipField;
	QLineEdit *fStateField;
	QLineEdit *fCityField;
	QLineEdit *fAddressField;
	QLineEdit *fPhoneField[5];
	QLineEdit *fCompanyField;
	QLineEdit *fTitleField;
	QLineEdit *fFirstNameField;
	QLineEdit *fLastNameField;
	// phone labels (changing!)
	QLabel    *m_phoneLabel[5];
	
	void initLayout();
	void fillFields();
	QString phoneLabelText(PilotAddress *, int i);
};
#endif

