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


#include "options.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kdebug.h>
#include "pilotAddress.h"
#include "addressEditor.moc"

AddressEditor::AddressEditor(PilotAddress *p,
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
	fAddress(p)
{
	FUNCTIONSETUP;

	initLayout();
	fillFields();

	connect(parent,SIGNAL(recordChanged(PilotAddress *)),
		this,SLOT(updateRecord(PilotAddress *)));
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
		fAddress = new PilotAddress();
		fDeleteOnCancel = true;
	}

	fLastNameField->setText(fAddress->getField(entryLastname));
	fFirstNameField->setText(fAddress->getField(entryFirstname));
	fCompanyField->setText(fAddress->getField(entryCompany));
	fPhone1Field->setText(fAddress->getField(entryPhone1));
	fPhone2Field->setText(fAddress->getField(entryPhone2));
	fPhone3Field->setText(fAddress->getField(entryPhone3));
	fPhone4Field->setText(fAddress->getField(entryPhone4));
	fPhone5Field->setText(fAddress->getField(entryPhone5));
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

void AddressEditor::initLayout()
{
	QFrame *p = plainPage();
	QGridLayout *grid = new QGridLayout(p,6,6,0,SPACING);

	QLabel *t;

	MakeField(I18N_NOOP("Last Name:"),fLastNameField,1,1);
	MakeField(I18N_NOOP("First Name:"),fFirstNameField,2,1);
	MakeField(I18N_NOOP("Title:"),fTitleField,3,1);
	MakeField(I18N_NOOP("Company:"),fCompanyField,4,1);
	MakeField(I18N_NOOP("Phone 1:"),fPhone1Field,5,1);
	MakeField(I18N_NOOP("Phone 2:"),fPhone2Field,6,1);
	MakeField(I18N_NOOP("Phone 3:"),fPhone3Field,7,1);
	MakeField(I18N_NOOP("Phone 4:"),fPhone4Field,8,1);
	MakeField(I18N_NOOP("Phone 5:"),fPhone5Field,9,1);


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
	fAddress->setField(entryLastname, fLastNameField->text().local8Bit());
	fAddress->setField(entryFirstname, fFirstNameField->text().local8Bit());
	fAddress->setField(entryCompany, fCompanyField->text().local8Bit());
	fAddress->setField(entryPhone1, fPhone1Field->text().local8Bit());
	fAddress->setField(entryPhone2, fPhone2Field->text().local8Bit());
	fAddress->setField(entryPhone3, fPhone3Field->text().local8Bit());
	fAddress->setField(entryPhone4, fPhone4Field->text().local8Bit());
	fAddress->setField(entryPhone5, fPhone5Field->text().local8Bit());
	fAddress->setField(entryAddress, fAddressField->text().local8Bit());
	fAddress->setField(entryCity, fCityField->text().local8Bit());
	fAddress->setField(entryState, fStateField->text().local8Bit());
	fAddress->setField(entryZip, fZipField->text().local8Bit());
	fAddress->setField(entryCountry, fCountryField->text().local8Bit());
	fAddress->setField(entryTitle, fTitleField->text().local8Bit());
	fAddress->setField(entryCustom1, fCustom1Field->text().local8Bit());
	fAddress->setField(entryCustom2, fCustom2Field->text().local8Bit());
	fAddress->setField(entryCustom3, fCustom3Field->text().local8Bit());
	fAddress->setField(entryCustom4, fCustom4Field->text().local8Bit());

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

// $Log:$
