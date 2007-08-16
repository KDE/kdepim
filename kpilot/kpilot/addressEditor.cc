// -*- C++ -*-
/* KPilot
**
** Copyright (C) 2000 by Dan Pilone <dan@kpilot.org>
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


#include "options.h"

#include <QFrame>
#include <qlabel.h>
#include <qlayout.h>

#include <KLineEdit>

#include "pilotAddress.h"

#include "addressEditor.moc"


AddressEditor::AddressEditor(PilotAddress * p,
	PilotAddressInfo *appInfo,
	QWidget * parent,
	const char *name) :
	KDialog(parent),
	fDeleteOnCancel(p == 0L),
	fAddress(p),
	fAppInfo(appInfo)
{
	FUNCTIONSETUP;

	setModal(false);
	setCaption(i18n("Address Editor"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);

	initLayout();
	fillFields();

	connect(parent, SIGNAL(recordChanged(PilotAddress *)),
		this, SLOT(updateRecord(PilotAddress *)));
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
	connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
}

AddressEditor::~AddressEditor()
{
	FUNCTIONSETUP;

	if (fDeleteOnCancel && fAddress)
	{
		DEBUGKPILOT << "Deleting private address record.";
		KPILOT_DELETE(fAddress);
	}
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
		field = new KLineEdit(p); \
		field->setMinimumWidth(20*SPACING); \
		t->setBuddy(field); \
		grid->addWidget(t,row,column); \
		grid->addWidget(field,row,column+1);

#define MakeFieldL(text,label,field,row,column) \
		label = new QLabel(text,p); \
		field = new KLineEdit(p); \
		field->setMinimumWidth(20*SPACING); \
		label->setBuddy(field); \
		grid->addWidget(label,row,column); \
		grid->addWidget(field,row,column+1);

void AddressEditor::initLayout()
{
	FUNCTIONSETUP;

	QFrame *p = new QFrame();
	setMainWidget(p);
	QGridLayout *grid = new QGridLayout(p, 10, 5);
	grid->setMargin(0);
	grid->setSpacing(SPACING);

	QLabel *t;

	MakeField(i18n("Last name:"), fLastNameField, 0, 0);
	MakeField(i18n("First name:"), fFirstNameField, 1, 0);
	MakeField(i18nc("Tile of the person.", "Title:"), fTitleField, 2, 0);
	MakeField(i18nc("Company where the person works.", "Company:"), fCompanyField, 3, 0);

	PhoneSlot slot = PhoneSlot::begin();
	for (int i = 0; slot.isValid(); ++i,++slot)
	{
		MakeFieldL(phoneLabelText(NULL, slot),
			m_phoneLabel[i], fPhoneField[i], 4 + i, 0);
	}

	MakeField(i18nc("Address where the person lives.", "Address:"), fAddressField, 0, 4);
	MakeField(i18nc("City where the person lives.", "City:"), fCityField, 1, 4);
	MakeField(i18nc("State where the person lives.", "State:"), fStateField, 2, 4);
	MakeField(i18nc("Zip or postalcode of Address.", "Zip code:"), fZipField, 3, 4);
	MakeField(i18nc("Country where the person lives", "Country:"), fCountryField, 4, 4);
	MakeField(i18nc("Custom field", "Custom 1:"), fCustom1Field, 5, 4);
	MakeField(i18nc("Custom field", "Custom 2:"), fCustom2Field, 6, 4);
	MakeField(i18nc("Custom field", "Custom 3:"), fCustom3Field, 7, 4);
	MakeField(i18nc("Custom field", "Custom 4:"), fCustom4Field, 8, 4);

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
	reject();
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
	accept();
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

