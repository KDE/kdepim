/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2004 by Adriaan de Groot
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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

#include <qptrlist.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtextview.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include "kpilotConfig.h"
#include "listItems.h"
#include "addressEditor.h"
#include "pilotLocalDatabase.h"

#include "addressWidget.moc"


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


#ifdef DEBUG
	DEBUGKPILOT << fname << ": Reading AddressDB..." << endl;
#endif

	while ((pilotRec = addressDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) &&
			(!(pilotRec->isSecret()) || KPilotSettings::showSecrets()))
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

void AddressWidget::showComponent()
{
	FUNCTIONSETUP;
	if ( fPendingAddresses>0 ) return;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Reading from directory " << dbPath() << endl;
#endif

	PilotDatabase *addressDB =
		new PilotLocalDatabase(dbPath(), CSL1("AddressDB"));
	unsigned char buffer[PilotRecord::APP_BUFFER_SIZE];
	int appLen;

	fAddressList.clear();

	if (addressDB->isDBOpen())
	{
		appLen = addressDB->readAppBlock(buffer, PilotRecord::APP_BUFFER_SIZE);
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

	KPILOT_DELETE( addressDB );

	updateWidget();
}

void AddressWidget::hideComponent()
{
	FUNCTIONSETUP;
	if (fPendingAddresses==0 )
	{
		fAddressList.clear();
		fListBox->clear();

		updateWidget();
	}
}

/* virtual */ bool AddressWidget::preHotSync(QString &s)
{
	FUNCTIONSETUP;

	if ( fPendingAddresses )
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

	if ( shown )
	{
		fAddressList.clear();
		showComponent();
	}
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
	QString wt;

	fEditButton = new QPushButton(i18n("Edit Record..."), this);
	grid->addWidget(fEditButton, 2, 0);
	connect(fEditButton, SIGNAL(clicked()), this, SLOT(slotEditRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>You can edit an address when it is selected.</qt>") :
		i18n("<qt><i>Editing is disabled by the 'internal editors' setting.</i></qt>");
	QWhatsThis::add(fEditButton,wt);

	button = new QPushButton(i18n("New Record..."), this);
	grid->addWidget(button, 2, 1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Add a new address to the address book.</qt>") :
		i18n("<qt><i>Adding is disabled by the 'internal editors' setting.</i></qt>") ;
	QWhatsThis::add(button, wt);
	button->setEnabled(KPilotSettings::internalEditors());


	fDeleteButton = new QPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton, 3, 0);
	connect(fDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteRecord()));
	wt = KPilotSettings::internalEditors() ?
		i18n("<qt>Delete the selected address from the address book.</qt>") :
		i18n("<qt><i>Deleting is disabled by the 'internal editors' setting.</i></qt>") ;

	button = new QPushButton(i18n("Export..."), this);
	grid->addWidget(button, 3,1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExport()));
	QWhatsThis::add(button,
		i18n("<qt>Export all addresses in the selected category to CSV format.</qt>") );

	QWhatsThis::add(fDeleteButton,wt);
}

void AddressWidget::updateWidget()
{
	FUNCTIONSETUP;

	int addressDisplayMode = KPilotSettings::addressDisplayMode();

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
				title.remove(QRegExp(CSL1("\n.*")));
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
		if (!address->getField(entryCompany).isEmpty())
		{
			title.append(address->getField(entryCompany));
		}
		if (!address->getField(entryLastname).isEmpty())
		{
			if (!title.isEmpty())
			{
				title.append( CSL1(", "));
			}

			title.append(address->getField(entryLastname));
		}
		break;
	case 0:
	default:
		if (!address->getField(entryLastname).isEmpty())
		{
			title.append(address->getField(entryLastname));
		}

		if (!address->getField(entryFirstname).isEmpty())
		{
			if (!title.isEmpty())
			{
				title.append( CSL1(", "));
			}
			title.append(address->getField(entryFirstname));
		}
		break;
	}

	if (title.isEmpty())	// One last try
	{
		if (!fAddressList.current()->getField(entryCompany).isEmpty())
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

	enabled &= KPilotSettings::internalEditors();
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
	if ( !shown ) return;

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
	if ( !shown ) return;

	// Response to bug 18072: Don't even try to
	// add records to an empty or unopened database,
	// since we don't have the DBInfo stuff to deal with it.
	//
	//
	PilotDatabase *myDB = new PilotLocalDatabase(dbPath(), CSL1("AddressDB"));

	if (!myDB || !myDB->isDBOpen())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Tried to open "
			<< dbPath()
			<< "/AddressDB"
			<< " and got pointer @"
			<< (long) myDB
			<< " with status "
			<< ( myDB ? myDB->isDBOpen() : false )
			<< endl;
#endif

		KMessageBox::sorry(this,
			i18n("You cannot add addresses to the address book "
				"until you have done a HotSync at least once "
				"to retrieve the database layout from your Pilot."),
			i18n("Cannot Add New Address"));

		if (myDB)
			KPILOT_DELETE( myDB );

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
	if ( !shown && fPendingAddresses==0 ) return;

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
	if ( !shown && fPendingAddresses==0 ) hideComponent();
}

void AddressWidget::slotUpdateRecord(PilotAddress * address)
{
	FUNCTIONSETUP;
	if ( !shown && fPendingAddresses==0 ) return;

	writeAddress(address);
	int currentRecord = fListBox->currentItem();

	// TODO: Just change the record
	updateWidget();
	fListBox->setCurrentItem(currentRecord);

	emit(recordChanged(address));

	fPendingAddresses--;
	if ( !shown && fPendingAddresses==0 ) hideComponent();
}

void AddressWidget::slotEditCancelled()
{
	FUNCTIONSETUP;

	fPendingAddresses--;
	if ( !shown && fPendingAddresses==0 ) hideComponent();
}

void AddressWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;
	if ( !shown ) return;

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
	showComponent();
}



void AddressWidget::slotShowAddress(int which)
{
	FUNCTIONSETUP;
	if (!shown) return;

	PilotListItem *p = (PilotListItem *) fListBox->item(which);
	PilotAddress *addr = (PilotAddress *) p->rec();

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Showing "
		<< addr->getField(entryLastname)
		<< " "
		<< addr->getField(entryFirstname)
		<< endl;
#endif

	QString text(CSL1("<qt>"));
	text += addr->getTextRepresentation(true);
	text += CSL1("</qt>\n");
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
		myDB = new PilotLocalDatabase(dbPath(), CSL1("AddressDB"));
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
	markDBDirty(CSL1("AddressDB"));
	delete pilotRec;

	// Clean up in the case that we allocated our own DB.
	//
	//
	if (usemyDB)
	{
		KPILOT_DELETE( myDB );
	}
}

#define plu_quiet 1
#include "pilot-addresses.c"

void AddressWidget::slotExport()
{
	FUNCTIONSETUP;
	
	int currentCatID = findSelectedCategory(fCatList,
		&(fAddressAppInfo.category));

	QString saveFile = KFileDialog::getSaveFileName(
		QString::null,
		CSL1("*.csv|Comma Separated Values"),
		this,
		( (currentCatID==-1) ?
			i18n("Export All Addresses") :
			i18n("Export Address Category %1").arg(PilotAppCategory::codec()->toUnicode(fAddressAppInfo.category.name[currentCatID]))));

	if (saveFile.isEmpty())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": No save file selected." << endl;
#endif
		return;
	}
	if (QFile::exists(saveFile) &&
		KMessageBox::warningContinueCancel(this,
			i18n("The file <i>%1</i> exists. Overwrite?").arg(saveFile),
			i18n("Overwrite File?"),
			i18n("Overwrite"))!=KMessageBox::Continue)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Overwrite file canceled." << endl;
#endif
		return;
	}

	FILE *f = fopen(QFile::encodeName(saveFile),"w");
	if (!f)
	{
		KMessageBox::sorry(this,
			i18n("The file <i>%1</i> could not be opened for writing.").arg(saveFile));
		return;
	}
	fAddressList.first();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Adding records..." << endl;
#endif

	while (fAddressList.current())
	{
		const PilotAddress *a = fAddressList.current();
		if ((currentCatID == -1) ||
			(a->getCat() == currentCatID))
		{
			write_record_CSV(f, &fAddressAppInfo, a->address(),
				a->getAttrib(), a->getCat(), 0);
		}
		fAddressList.next();
	}

	fclose(f);
}

