/* addressWidget.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the addressWidget, that part of KPilot that
** displays address records from the Pilot.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *addresswidget_id =
	"$Id$";



#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <iostream>
#include <cstring>
#include <cstdlib>

#ifndef QLIST_H
#include <qlist.h>
#endif
#ifndef QLISTBOX_H
#include <qlistbox.h>
#endif
#ifndef QFILE_H
#include <qfile.h>
#endif
#ifndef QPUSHBT_H
#include <qpushbt.h>
#endif
#ifndef QTEXTSTREAM_H
#include <qtextstream.h>
#endif
#ifndef QLAYOUT_H
#include <qlayout.h>
#endif
#ifndef QLABEL_H
#include <qlabel.h>
#endif
#ifndef QMULTILINEEDIT_H
#include <qmultilineedit.h>
#endif
#ifndef QCOMBOBOX_H
#include <qcombobox.h>
#endif
#include <qwhatsthis.h>
#ifndef QTEXTVIEW_H
#include <qtextview.h>
#endif

#ifndef _KAPP_H
#include <kapplication.h>
#endif
#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif
#ifndef _KDEBUG_H
#include <kdebug.h>
#endif
#ifndef _KFILEDIALOG_H
#include <kfiledialog.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif
#ifndef _KPILOT_LISTITEMS_H
#include "listItems.h"
#endif
#ifndef _KPILOT_ADDRESSEDITOR_H
#include "addressEditor.h"
#endif
#ifndef _KPILOT_PILOTLOCALDATABASE_H
#include "pilotLocalDatabase.h"
#endif

#include "addressWidget.moc"

// This is the size of several (automatic) buffers,
// used to retrieve data from the database.
// I have no idea if 0xffff is some magic number or not.
//
//
#define BUFFERSIZE	(0xffff)

AddressWidget::AddressWidget(QWidget * parent,
	const QString & path) :
	PilotComponent(parent, "component_address", path),
	fAddrInfo(0),
	fPendingAddresses(0)
{
	FUNCTIONSETUP;

	setupWidget();
	fAddressList.setAutoDelete(true);

	/* NOTREACHED */
	(void) addresswidget_id;
}

AddressWidget::~AddressWidget()
{
	FUNCTIONSETUP;
}

int AddressWidget::getAllAddresses(PilotDatabase * addressDB)
{
	FUNCTIONSETUP;

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotAddress *address;
	bool showSecrets = KPilotConfig::getConfig().getShowSecrets();


#ifdef DEBUG
	DEBUGKPILOT << fname << ": Reading AddressDB..." << endl;
#endif

	while ((pilotRec = addressDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) &&
			(!(pilotRec->isSecret()) || showSecrets))
		{
			address = new PilotAddress(fAddressAppInfo, pilotRec);
			if (address == 0L)
			{
				kdWarning() << k_funcinfo
					<< ": Couldn't allocate record "
					<< currentRecord++
					<< endl;
				break;
			}
			fAddressList.append(address);
		}
		delete pilotRec;

		currentRecord++;
	}

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Total " << currentRecord << " records" << endl;
#endif

	return currentRecord;
}

void AddressWidget::initialize()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Reading from directory " << dbPath() << endl;
#endif

	PilotDatabase *addressDB =
		new PilotLocalDatabase(dbPath(), "AddressDB");
	unsigned char buffer[BUFFERSIZE];
	int appLen;

	fAddressList.clear();

	if (addressDB->isDBOpen())
	{
		appLen = addressDB->readAppBlock(buffer, BUFFERSIZE);
		unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);

		populateCategories(fCatList, &fAddressAppInfo.category);
		getAllAddresses(addressDB);

	}
	else
	{
		populateCategories(fCatList, 0L);
		kdWarning() << k_funcinfo
			<< ": Could not open local AddressDB" << endl;
	}

	delete addressDB;

	updateWidget();
}

/* virtual */ bool AddressWidget::preHotSync(QString &s)
{
	FUNCTIONSETUP;

	if (fPendingAddresses)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": fPendingAddress="
			<< fPendingAddresses
			<< endl;
#endif

#if KDE_VERSION<220
		s = i18n("There are still %1 address editing windows open.")
			.arg(QString::number(fPendingAddresses));
#else
		s = i18n("There is still an address editing window open.",
			"There are still %n address editing windows open.",
			fPendingAddresses);
#endif
		return false;
	}

	return true;
}

void AddressWidget::postHotSync()
{
	FUNCTIONSETUP;

	fAddressList.clear();
	initialize();
}


void AddressWidget::setupWidget()
{
	FUNCTIONSETUP;

	QLabel *label;
	QGridLayout *grid = new QGridLayout(this, 6, 4, SPACING);

	fCatList = new QComboBox(this);
	grid->addWidget(fCatList, 0, 1);
	connect(fCatList, SIGNAL(activated(int)),
		this, SLOT(slotSetCategory(int)));
	QWhatsThis::add(fCatList,
		i18n("<qt>Select the category of addresses to display here.</qt>"));

	label = new QLabel(i18n("Category:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label, 0, 0);

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox, 1, 1, 0, 1);
	connect(fListBox, SIGNAL(highlighted(int)),
		this, SLOT(slotShowAddress(int)));
	connect(fListBox, SIGNAL(selected(int)),
		this, SLOT(slotEditRecord()));
	QWhatsThis::add(fListBox,
		i18n("<qt>This list displays all the addresses "
			"in the selected category. Click on "
			"one to display it to the right.</qt>"));

	label = new QLabel(i18n("Address info:"), this);
	grid->addWidget(label, 0, 2);

	// address info text view
	fAddrInfo = new QTextView(this);
	grid->addMultiCellWidget(fAddrInfo, 1, 4, 2, 2);

	QPushButton *button;

	fEditButton = new QPushButton(i18n("Edit Record"), this);
	grid->addWidget(fEditButton, 2, 0);
	connect(fEditButton, SIGNAL(clicked()), this, SLOT(slotEditRecord()));
	QWhatsThis::add(fEditButton,
		i18n("<qt>You can edit an address when it is selected.</qt>"));

	button = new QPushButton(i18n("New Record"), this);
	grid->addWidget(button, 2, 1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	QWhatsThis::add(button, i18n("<qt>Add a new address to the address book.</qt>"));

	fDeleteButton = new QPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton, 3, 0);
	connect(fDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteRecord()));
	QWhatsThis::add(fDeleteButton,
		i18n("<qt>Delete the selected address from the address book.</qt>"));
}

void AddressWidget::updateWidget()
{
	FUNCTIONSETUP;

	int addressDisplayMode =
		KPilotConfig::getConfig().setAddressGroup().
		getAddressDisplayMode();

	int listIndex = 0;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Display Mode=" << addressDisplayMode << endl;
#endif

	int currentCatID = findSelectedCategory(fCatList,
		&(fAddressAppInfo.category));

	fListBox->clear();
	fAddressList.first();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Adding records..." << endl;
#endif

	while (fAddressList.current())
	{
		if ((currentCatID == -1) ||
			(fAddressList.current()->getCat() == currentCatID))
		{
			QString title = createTitle(fAddressList.current(),
				addressDisplayMode);

			if (!title.isEmpty())
			{
				PilotListItem *p = new PilotListItem(title,
					listIndex,
					fAddressList.current());

				fListBox->insertItem(p);
			}
		}
		listIndex++;
		fAddressList.next();
	}

#ifdef DEBUG
	DEBUGKPILOT << fname << ": " << listIndex << " records" << endl;
#endif

	slotUpdateButtons();
}



QString AddressWidget::createTitle(PilotAddress * address, int displayMode)
{
	// FUNCTIONSETUP;

	QString title;

	switch (displayMode)
	{
	case 1:
		if (address->getField(entryCompany) &&
			strcmp(address->getField(entryCompany), ""))
		{
			title.append(address->getField(entryCompany));
		}
		if (address->getField(entryLastname) &&
			strcmp(address->getField(entryLastname), ""))
		{
			if (!title.isEmpty())
			{
				title.append( ", ");
			}

			title.append(address->getField(entryLastname));
		}
		break;
	case 0:
	default:
		if (address->getField(entryLastname) &&
			strcmp(address->getField(entryLastname), ""))
		{
			title.append(address->getField(entryLastname));
		}

		if (address->getField(entryFirstname) &&
			strcmp(address->getField(entryFirstname), ""))
		{
			if (!title.isEmpty())
			{
				title.append( ", ");
			}
			title.append(address->getField(entryFirstname));
		}
		break;
	}

	if (title.isEmpty())	// One last try
	{
		if (fAddressList.current()->getField(entryCompany))
		{
			title.append(fAddressList.current()->
				getField(entryCompany));
		}
		if (title.isEmpty())
		{
			title = i18n("[unknown]");
		}
	}

	return title;
}


/* slot */ void AddressWidget::slotUpdateButtons()
{
	FUNCTIONSETUP;

	bool enabled = (fListBox->currentItem() != -1);

	fEditButton->setEnabled(enabled);
	fDeleteButton->setEnabled(enabled);
}

void AddressWidget::slotSetCategory(int)
{
	FUNCTIONSETUP;

	updateWidget();
}

void AddressWidget::slotEditRecord()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();

	if (item == -1)
		return;

	PilotListItem *p = (PilotListItem *) fListBox->item(item);
	PilotAddress *selectedRecord = (PilotAddress *) p->rec();

	if (selectedRecord->id() == 0)
	{
		KMessageBox::error(0L,
			i18n("Cannot edit new records until "
				"HotSynced with Pilot."),
			i18n("HotSync Required"));
		return;
	}

	AddressEditor *editor = new AddressEditor(selectedRecord,
		&fAddressAppInfo, this);

	connect(editor, SIGNAL(recordChangeComplete(PilotAddress *)),
		this, SLOT(slotUpdateRecord(PilotAddress *)));
	connect(editor, SIGNAL(cancelClicked()),
		this, SLOT(slotEditCancelled()));
	editor->show();

	fPendingAddresses++;
}

void AddressWidget::slotCreateNewRecord()
{
	FUNCTIONSETUP;

	// Response to bug 18072: Don't even try to
	// add records to an empty or unopened database,
	// since we don't have the DBInfo stuff to deal with it.
	//
	//
	PilotDatabase *myDB = new PilotLocalDatabase(dbPath(), "AddressDB");

	if (!myDB || !myDB->isDBOpen())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Tried to open "
			<< dbPath()
			<< "/AddressDB"
			<< " and got pointer @"
			<< (int) myDB
			<< " with status "
			<< ( myDB ? myDB->isDBOpen() : false )
			<< endl;
#endif

		KMessageBox::sorry(this,
			i18n("You can't add addresses to the address book "
				"until you have done a HotSync at least once "
				"to retrieve the database layout from your Pilot."),
			i18n("Can't Add New Address"));

		if (myDB)
			delete myDB;

		return;
	}

	AddressEditor *editor = new AddressEditor(0L,
		&fAddressAppInfo, this);

	connect(editor, SIGNAL(recordChangeComplete(PilotAddress *)),
		this, SLOT(slotAddRecord(PilotAddress *)));
	connect(editor, SIGNAL(cancelClicked()),
		this, SLOT(slotEditCancelled()));
	editor->show();

	fPendingAddresses++;
}

void AddressWidget::slotAddRecord(PilotAddress * address)
{
	FUNCTIONSETUP;

	int currentCatID = findSelectedCategory(fCatList,
		&(fAddressAppInfo.category), true);


	address->setCat(currentCatID);
	fAddressList.append(address);
	writeAddress(address);
	// TODO: Just add the new record to the lists
	updateWidget();

	// k holds the item number of the address just added.
	//
	//
	int k = fListBox->count() - 1;

	fListBox->setCurrentItem(k);	// Show the newest one
	fListBox->setBottomItem(k);

	fPendingAddresses--;
}

void AddressWidget::slotUpdateRecord(PilotAddress * address)
{
	FUNCTIONSETUP;

	writeAddress(address);
	int currentRecord = fListBox->currentItem();

	// TODO: Just change the record
	updateWidget();
	fListBox->setCurrentItem(currentRecord);

	emit(recordChanged(address));

	fPendingAddresses--;
}

void AddressWidget::slotEditCancelled()
{
	FUNCTIONSETUP;

	fPendingAddresses--;
}

void AddressWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();

	if (item == -1)
		return;

	PilotListItem *p = (PilotListItem *) fListBox->item(item);
	PilotAddress *selectedRecord = (PilotAddress *) p->rec();

	if (selectedRecord->id() == 0)
	{
		KMessageBox::error(this,
			i18n("New records cannot be deleted until "
				"HotSynced with pilot."),
			i18n("HotSync Required"));
		return;
	}

	if (KMessageBox::questionYesNo(this,
			i18n("Delete currently selected record?"),
			i18n("Delete Record?")) == KMessageBox::No)
		return;

	selectedRecord->makeDeleted();
	writeAddress(selectedRecord);
	emit(recordChanged(selectedRecord));
	initialize();
}



void AddressWidget::slotShowAddress(int which)
{
	FUNCTIONSETUP;

	PilotListItem *p = (PilotListItem *) fListBox->item(which);
	PilotAddress *addr = (PilotAddress *) p->rec();
	int i;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Showing "
		<< addr->getField(entryLastname)
		<< " "
		<< addr->getField(entryFirstname)
		<< endl;
#endif

	/*
	 * enum values from pi-address.h
	 *
	 * entryLastname, entryFirstname,
	 * entryCompany, entryPhone1, entryPhone2, entryPhone3,
	 * entryPhone4, entryPhone5, entryAddress, entryCity, entryState,
	 * entryZip, entryCountry, entryTitle, entryCustom1, entryCustom2,
	 * entryCustom3, entryCustom4, entryNote
	 */

	QString text;

	text += "<qt>";

	// title + name
	text += "<p>";
	if (addr->getField(entryTitle))
	{
		text += addr->getField(entryTitle);
		text += " ";
	}
	text += "<b><big>";
	if (addr->getField(entryFirstname))
	{
		text += addr->getField(entryFirstname);
		text += " ";
	}
	text += addr->getField(entryLastname);
	text += "</big></b>";
	text += "</p>";

	// company
	if (addr->getField(entryCompany))
	{
		text += "<p>";
		text += addr->getField(entryCompany);
		text += "</p>";
	}

	// phone numbers (+ labels)
	text += "<p>";
	for (i = entryPhone1; i <= entryPhone5; i++)
		if (addr->getField(i))
		{
			text += "<small>";
			text += fAddressAppInfo.phoneLabels[addr->
				getPhoneLabelIndex(i - entryPhone1)];
			text += ": </small>";
			if (addr->getShownPhone() == i - entryPhone1)
				text += "<b>";
			text += addr->getField(i);
			if (addr->getShownPhone() == i - entryPhone1)
				text += "</b>";
			text += "<br/>";
		}
	text += "</p>";

	// address, city, state, country
	text += "<p>";
	if (addr->getField(entryAddress))
	{
		text += addr->getField(entryAddress);
		text += "<br/>";
	}
	if (addr->getField(entryCity))
	{
		text += addr->getField(entryCity);
		text += " ";
	}
	if (addr->getField(entryState))
	{
		text += addr->getField(entryState);
		text += " ";
	}
	if (addr->getField(entryZip))
	{
		text += addr->getField(entryZip);
	}
	text += "<br/>";
	if (addr->getField(entryCountry))
	{
		text += addr->getField(entryCountry);
		text += "<br/>";
	}
	text += "</p>";

	// custom fields
	text += "<p>";
	for (i = entryCustom1; i <= entryCustom4; i++)
		if (addr->getField(i))
		{
			text += addr->getField(i);
			text += "<br/>";
		}
	text += "</p>";

	// note
	if (addr->getField(entryNote))
	{
		text += "<hr/>";
		text += "<p>";
		text += addr->getField(entryNote);
		text += "</p>";
	}

	text += "</qt>\n";
	fAddrInfo->setText(text);

	slotUpdateButtons();
}



void AddressWidget::writeAddress(PilotAddress * which,
	PilotDatabase * addressDB)
{
	FUNCTIONSETUP;

	// Open a database (myDB) only if needed,
	// i.e. only if the passed-in addressDB
	// isn't valid.
	//
	//
	PilotDatabase *myDB = addressDB;
	bool usemyDB = false;

	if (myDB == 0L || !myDB->isDBOpen())
	{
		myDB = new PilotLocalDatabase(dbPath(), "AddressDB");
		usemyDB = true;
	}

	// Still no valid address database...
	//
	//
	if (!myDB->isDBOpen())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Address database is not open" <<
			endl;
#endif
		return;
	}


	// Do the actual work.
	PilotRecord *pilotRec = which->pack();

	myDB->writeRecord(pilotRec);
	delete pilotRec;

	// Clean up in the case that we allocated our own DB.
	//
	//
	if (usemyDB)
	{
		delete myDB;
	}
}

