/* addressEditor.cc		KPilot
**
** Copyright (C) 2000 by Dan Pilone
**
** This is a dialog window that edits one single address record.
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


#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#ifndef QLINEEDIT_H
#include <qlineedit.h>
#endif
#ifndef QLAYOUT_H
#include <qlayout.h>
#endif
#ifndef QLABEL_H
#include <qlabel.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif

#ifndef _KPILOT_PILOTADDRESS_H
#include "pilotAddress.h"
#endif

#include "addressEditor.moc"

static const char *addressEditor_id =
	"$Id$";

AddressEditor::AddressEditor(PilotAddress *p,
	struct AddressAppInfo *appInfo,
	QWidget *parent,
	const char *name) :
	KDialogBase(KDialogBase::Plain,
		i18n("Address Editor"),
		Ok | Cancel,
		Cancel,
		parent,
		name,
		false /* non-modal */),
	fDeleteOnCancel(p == 0L),
	fAddress(p),
	fAppInfo(appInfo)
{
	FUNCTIONSETUP;

	initLayout(appInfo);
	fillFields();

	connect(parent,SIGNAL(recordChanged(PilotAddress *)),
		this,SLOT(updateRecord(PilotAddress *)));

	(void) addressEditor_id;
}

AddressEditor::~AddressEditor()
{
	FUNCTIONSETUP;

	if (fDeleteOnCancel && fAddress)
	{
		DEBUGKPILOT << fname
			<< ": Deleting private address record."
			<< endl;

		delete fAddress;
		fAddress = 0L;
	}

	DEBUGKPILOT << fname
		<< ": Help! I'm deleting!"
		<< endl;
}

void AddressEditor::fillFields()
{
	FUNCTIONSETUP;

	if(fAddress == 0L)
	{
	        fAddress = new PilotAddress(*fAppInfo);
		fDeleteOnCancel = true;
	}

	fLastNameField->setText(fAddress->getField(entryLastname));
	fFirstNameField->setText(fAddress->getField(entryFirstname));
	fCompanyField->setText(fAddress->getField(entryCompany));
	fPhoneField[0]->setText(fAddress->getField(entryPhone1));
	fPhoneField[1]->setText(fAddress->getField(entryPhone2));
	fPhoneField[2]->setText(fAddress->getField(entryPhone3));
	fPhoneField[3]->setText(fAddress->getField(entryPhone4));
	fPhoneField[4]->setText(fAddress->getField(entryPhone5));
	fAddressField->setText(fAddress->getField(entryAddress));
	fCityField->setText(fAddress->getField(entryCity));
	fStateField->setText(fAddress->getField(entryState));
	fZipField->setText(fAddress->getField(entryZip));
	fCountryField->setText(fAddress->getField(entryCountry));
	fTitleField->setText(fAddress->getField(entryTitle));
	fCustom1Field->setText(fAddress->getField(entryCustom1));
	fCustom2Field->setText(fAddress->getField(entryCustom2));
	fCustom3Field->setText(fAddress->getField(entryCustom3));
	fCustom4Field->setText(fAddress->getField(entryCustom4));
}




#define MakeField(label,field,row,column) t=new QLabel(i18n(label),p); \
		field = new QLineEdit(p); \
		t->setBuddy(field); \
		grid->addWidget(t,row,column); \
		grid->addWidget(field,row,column+1);

void AddressEditor::initLayout(const struct AddressAppInfo *addressInfo)
{
	QFrame *p = plainPage();
	QGridLayout *grid = new QGridLayout(p,6,6,0,SPACING);

	QLabel *t;

	MakeField(I18N_NOOP("Last Name:"),fLastNameField,1,1);
	MakeField(I18N_NOOP("First Name:"),fFirstNameField,2,1);
	MakeField(I18N_NOOP("Title:"),fTitleField,3,1);
	MakeField(I18N_NOOP("Company:"),fCompanyField,4,1);

	// Vaguely copied from the addressWidget
	//
	//	
	for (int i=0; i<5; i++)
	{
		MakeField(addressInfo->phoneLabels[i],
			fPhoneField[i],5+i,1);
        }

	MakeField(I18N_NOOP("Address:"),fAddressField,1,4);
	MakeField(I18N_NOOP("City:"),fCityField,2,4);
	MakeField(I18N_NOOP("State:"),fStateField,3,4);
	MakeField(I18N_NOOP("Zip Code:"),fZipField,4,4);
	MakeField(I18N_NOOP("Country:"),fCountryField,5,4);
	MakeField(I18N_NOOP("Custom 1:"),fCustom1Field,6,4);
	MakeField(I18N_NOOP("Custom 2:"),fCustom2Field,7,4);
	MakeField(I18N_NOOP("Custom 3:"),fCustom3Field,8,4);
	MakeField(I18N_NOOP("Custom 4:"),fCustom4Field,9,4);

	grid->addRowSpacing(0,SPACING);
	grid->addRowSpacing(10,SPACING);
	grid->addColSpacing(3,SPACING);
	grid->addColSpacing(6,SPACING);
	grid->setColStretch(2,50);
	grid->setColStretch(5,50);
	grid->setRowStretch(10,100);
}

/* slot */ void AddressEditor::slotCancel()
{
	if (fDeleteOnCancel && fAddress)
	{
		delete fAddress;
		fAddress = 0L;
	}
	KDialogBase::slotCancel();
}

/* slot */ void AddressEditor::slotOk()
{
	FUNCTIONSETUP;

	// Commit changes here
	fAddress->setField(entryLastname, fLastNameField->text().latin1());
	fAddress->setField(entryFirstname, fFirstNameField->text().latin1());
	fAddress->setField(entryCompany, fCompanyField->text().latin1());
	fAddress->setField(entryPhone1, fPhoneField[0]->text().latin1());
	fAddress->setField(entryPhone2, fPhoneField[1]->text().latin1());
	fAddress->setField(entryPhone3, fPhoneField[2]->text().latin1());
	fAddress->setField(entryPhone4, fPhoneField[3]->text().latin1());
	fAddress->setField(entryPhone5, fPhoneField[4]->text().latin1());
	fAddress->setField(entryAddress, fAddressField->text().latin1());
	fAddress->setField(entryCity, fCityField->text().latin1());
	fAddress->setField(entryState, fStateField->text().latin1());
	fAddress->setField(entryZip, fZipField->text().latin1());
	fAddress->setField(entryCountry, fCountryField->text().latin1());
	fAddress->setField(entryTitle, fTitleField->text().latin1());
	fAddress->setField(entryCustom1, fCustom1Field->text().latin1());
	fAddress->setField(entryCustom2, fCustom2Field->text().latin1());
	fAddress->setField(entryCustom3, fCustom3Field->text().latin1());
	fAddress->setField(entryCustom4, fCustom4Field->text().latin1());

	emit(recordChangeComplete(fAddress));
	KDialogBase::slotOk();
}

/* slot */ void AddressEditor::updateRecord(PilotAddress *p)
{
	if (p != fAddress)
	{
		// Not meant for me
		//
		//
		return;
	}

	if (p->isDeleted())
	{
		delayedDestruct();
		return;
	}
	else
	{
		fillFields();
	}
}

// $Log$
// Revision 1.6  2001/04/04 21:20:32  stern
// Added support for category information and copy constructors
//
// Revision 1.5  2001/03/19 23:12:39  stern
// Made changes necessary for upcoming abbrowser conduit.
//
// Mainly, I added two public methods to PilotAddress that allow for easier
// setting and getting of phone fields.
//
// I also have added some documentation throughout as I have tried to figure
// out how everything works.
//
// Revision 1.4  2001/03/11 10:50:40  adridg
// Make address editor reflect real field names
//
// Revision 1.3  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.2  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
