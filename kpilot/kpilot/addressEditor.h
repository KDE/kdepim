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

#ifndef _KPILOT_ADDRESSEDITOR_H
#define _KPILOT_ADDRESSEDITOR_H

#ifndef _KDIALOGBASE_H
#include <kdialogbase.h>
#endif

class QLineEdit;
class PilotAddress;
struct AddressAppInfo;

class AddressEditor : public KDialogBase 
{
	Q_OBJECT


public:
	AddressEditor(PilotAddress *address,
		const struct AddressAppInfo *appInfo,
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
        const struct AddressAppInfo *fAppInfo;
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

	void initLayout(const struct AddressAppInfo *);
  void fillFields();
};
#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.7  2001/03/11 10:50:40  adridg
// Make address editor reflect real field names
//
// Revision 1.6  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.5  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
