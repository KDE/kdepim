// -*- C++ -*-
/* addressEditor.h		KPilot
**
** Copyright (C) 1998-2000 by Dan Pilone <dan@kpilot.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_ADDRESSEDITOR_H
#define _KPILOT_ADDRESSEDITOR_H

#include <kdialog.h>

#include "pilotAddress.h"
//Added by qt3to4:
#include <QLabel>

class KLineEdit;

class AddressEditor : public KDialog
{
	Q_OBJECT


public:
	AddressEditor(PilotAddress *address,
		PilotAddressInfo *appInfo,
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
	PilotAddressInfo *fAppInfo;
	// entry fields
	KLineEdit *fCustom4Field;
	KLineEdit *fCustom3Field;
	KLineEdit *fCustom2Field;
	KLineEdit *fCustom1Field;
	KLineEdit *fCountryField;
	KLineEdit *fZipField;
	KLineEdit *fStateField;
	KLineEdit *fCityField;
	KLineEdit *fAddressField;
	KLineEdit *fPhoneField[5];
	KLineEdit *fCompanyField;
	KLineEdit *fTitleField;
	KLineEdit *fFirstNameField;
	KLineEdit *fLastNameField;
	// phone labels (changing!)
	QLabel    *m_phoneLabel[5];

	void initLayout();
	void fillFields();
	QString phoneLabelText(PilotAddress *, const PhoneSlot &i);
};
#endif

