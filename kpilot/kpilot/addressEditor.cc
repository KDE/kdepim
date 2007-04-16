// -*- C++ -*-
/* KPilot
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
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


AddressEditor::AddressEditor(PilotAddress * p,
	PilotAddressInfo *appInfo,
	QWidget * parent,
	const char *name) :
	KDialogBase(KDialogBase::Plain,
		i18n("Address Editor"),
		Ok | Cancel, Cancel,
		parent, name, false /* non-modal */ ),
	fDeleteOnCancel(p == 0L),
	fAddress(p),
	fAppInfo(appInfo)
{
	FUNCTIONSETUP;

	initLayout();
	fillFields();

	connect(parent, SIGNAL(recordChanged(PilotAddress *)),
		this, SLOT(updateRecord(PilotAddress *)));

}

AddressEditor::~AddressEditor()
{
	FUNCTIONSETUP;

	if (fDeleteOnCancel && fAddress)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Deleting private address record." << endl;
#endif

		delete fAddress;

		fAddress = 0L;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Help! I'm deleting!" << endl;
#endif
}



/*
 * Return phone label from AddressAppInfo + some sanity checking
 */
QString AddressEditor::phoneLabelText(PilotAddress * addr, const PhoneSlot &i)
{
	FUNCTIONSETUP;
	if (!addr)
	{
		return i18n("Phone");
	}

	PilotAddressInfo::EPhoneType idx = addr->getPhoneType(i);
	QString ret = fAppInfo->phoneLabel(idx) + CSL1(":");

	return ret;
}



void AddressEditor::fillFields()
{
	FUNCTIONSETUP;

	if (fAddress == 0L)
	{
		fAddress = new PilotAddress();
		fDeleteOnCancel = true;
	}

	// phone labels
	unsigned int index = 0;
	for ( PhoneSlot i = PhoneSlot::begin(); i.isValid(); ++i,++index )
	{
		m_phoneLabel[index]->setText(phoneLabelText(fAddress, i));
		fPhoneField[index]->setText(fAddress->getField(i));
	}

	// fields
	fLastNameField->setText(fAddress->getField(entryLastname));
	fFirstNameField->setText(fAddress->getField(entryFirstname));
	fCompanyField->setText(fAddress->getField(entryCompany));
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




#define MakeField(text,field,row,column) \
		t=new QLabel(text,p); \
		field = new QLineEdit(p); \
		field->setMinimumWidth(20*SPACING); \
		t->setBuddy(field); \
		grid->addWidget(t,row,column); \
		grid->addWidget(field,row,column+1);

#define MakeFieldL(text,label,field,row,column) \
		label = new QLabel(text,p); \
		field = new QLineEdit(p); \
		field->setMinimumWidth(20*SPACING); \
		label->setBuddy(field); \
		grid->addWidget(label,row,column); \
		grid->addWidget(field,row,column+1);

void AddressEditor::initLayout()
{
	FUNCTIONSETUP;

	QFrame *p = plainPage();
	QGridLayout *grid = new QGridLayout(p, 10, 5, 0, SPACING);

	QLabel *t;

	MakeField(i18n("Last name:"), fLastNameField, 0, 0);
	MakeField(i18n("First name:"), fFirstNameField, 1, 0);
	MakeField(i18n("Title:"), fTitleField, 2, 0);
	MakeField(i18n("Company:"), fCompanyField, 3, 0);

	PhoneSlot slot = PhoneSlot::begin();
	for (int i = 0; slot.isValid(); ++i,++slot)
	{
		MakeFieldL(phoneLabelText(NULL, slot),
			m_phoneLabel[i], fPhoneField[i], 4 + i, 0);
	}

	MakeField(i18n("Address:"), fAddressField, 0, 4);
	MakeField(i18n("City:"), fCityField, 1, 4);
	MakeField(i18n("State:"), fStateField, 2, 4);
	MakeField(i18n("Zip code:"), fZipField, 3, 4);
	MakeField(i18n("Country:"), fCountryField, 4, 4);
	MakeField(i18n("Custom 1:"), fCustom1Field, 5, 4);
	MakeField(i18n("Custom 2:"), fCustom2Field, 6, 4);
	MakeField(i18n("Custom 3:"), fCustom3Field, 7, 4);
	MakeField(i18n("Custom 4:"), fCustom4Field, 8, 4);

	grid->addRowSpacing(9, SPACING);
	grid->addColSpacing(2, SPACING);
	grid->setRowStretch(9, 100);
	grid->setColStretch(2, 50);
}

/* slot */ void AddressEditor::slotCancel()
{
	FUNCTIONSETUP;

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
	fAddress->setField(entryLastname, fLastNameField->text());
	fAddress->setField(entryFirstname, fFirstNameField->text());
	fAddress->setField(entryCompany, fCompanyField->text());
	fAddress->setField(entryPhone1, fPhoneField[0]->text());
	fAddress->setField(entryPhone2, fPhoneField[1]->text());
	fAddress->setField(entryPhone3, fPhoneField[2]->text());
	fAddress->setField(entryPhone4, fPhoneField[3]->text());
	fAddress->setField(entryPhone5, fPhoneField[4]->text());
	fAddress->setField(entryAddress, fAddressField->text());
	fAddress->setField(entryCity, fCityField->text());
	fAddress->setField(entryState, fStateField->text());
	fAddress->setField(entryZip, fZipField->text());
	fAddress->setField(entryCountry, fCountryField->text());
	fAddress->setField(entryTitle, fTitleField->text());
	fAddress->setField(entryCustom1, fCustom1Field->text());
	fAddress->setField(entryCustom2, fCustom2Field->text());
	fAddress->setField(entryCustom3, fCustom3Field->text());
	fAddress->setField(entryCustom4, fCustom4Field->text());

	emit(recordChangeComplete(fAddress));
	KDialogBase::slotOk();
}

/* slot */ void AddressEditor::updateRecord(PilotAddress * p)
{
	FUNCTIONSETUP;
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

