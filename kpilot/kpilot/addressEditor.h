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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _ADDRESSEDITOR_H
#define _ADDRESSEDITOR_H

#include <kdialogbase.h>

class QLineEdit;
class PilotAddress;

class AddressEditor : public KDialogBase 
{
	Q_OBJECT


public:
	AddressEditor(PilotAddress* address, 
		QWidget *parent=NULL, const char *name=NULL);
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
	QLineEdit *fCustom4Field;
	QLineEdit *fCustom3Field;
	QLineEdit *fCustom2Field;
	QLineEdit *fCustom1Field;
	QLineEdit *fCountryField;
	QLineEdit *fZipField;
	QLineEdit *fStateField;
	QLineEdit *fCityField;
	QLineEdit *fAddressField;
	QLineEdit *fPhone5Field;
	QLineEdit *fPhone4Field;
	QLineEdit *fPhone3Field;
	QLineEdit *fPhone2Field;
	QLineEdit *fPhone1Field;
	QLineEdit *fCompanyField;
	QLineEdit *fTitleField;
	QLineEdit *fFirstNameField;
	QLineEdit *fLastNameField;
  // QButton *fCancelButton;
  // QButton *fOkButton;
  void initLayout();
  void fillFields();
};
#endif


// $Log:$
