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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
static const char *addresswidget_id="$Id$";



#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <iostream.h>
#include <string.h>
#include <stdlib.h>

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
#ifndef QTOOLTIP_H
#include <qtooltip.h>
#endif
#ifndef QTEXTVIEW_H
#include <qtextview.h>
#endif

#ifndef _KAPP_H
#include <kapp.h>
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
#ifndef _KPILOT_KPILOTOPTIONS_H
#include "kpilotOptions.h"
#endif
#ifndef _KPILOT_LISTITEMS_H
#include "listItems.h"
#endif
#ifndef _KPILOT_STRTOKEN_H
#include "strToken.h"
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

AddressWidget::AddressWidget(QWidget* parent, const QString& path) :
	PilotComponent(parent,"component_address",path), 
	fAddrInfo(0)
{
	FUNCTIONSETUP;

	setupWidget();
	fAddressList.setAutoDelete(true);
}

AddressWidget::~AddressWidget()
{
    	FUNCTIONSETUP;
}

int AddressWidget::getAllAddresses(PilotDatabase *addressDB,KConfig& config)
{
	FUNCTIONSETUP;

	int currentRecord = 0;
	PilotRecord* pilotRec;
	PilotAddress* address;
	bool showSecrets=0;

	config.setGroup(QString::null);
	showSecrets = (bool) config.readNumEntry("ShowSecrets");



	DEBUGKPILOT << fname 
		<< ": Reading AddressDB..."
		<< endl;

	while((pilotRec = addressDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) && 
			(!(pilotRec->isSecret()) || showSecrets))
		{
			address = new PilotAddress(fAddressAppInfo, pilotRec);
			if (address == 0L)
			{
				kdWarning() << __FUNCTION__ << ": Couldn't allocate "
					"record " << currentRecord++ <<
					endl;
				break;
			}
			fAddressList.append(address);
		}
		delete pilotRec;
		currentRecord++;
	}

	DEBUGKPILOT << fname 
		<< ": Total " << currentRecord << " records" << endl;

	return currentRecord;
	/* NOTREACHED */
	(void) addresswidget_id;
}

void
AddressWidget::initialize()
{
    	FUNCTIONSETUP;

	PilotDatabase* addressDB = new PilotLocalDatabase(dbPath(),"AddressDB");
	unsigned char buffer[BUFFERSIZE];
	int appLen;

	KConfig& config = KPilotConfig::getConfig();

	fAddressList.clear();

	if(addressDB->isDBOpen())
	{
		appLen = addressDB->readAppBlock(buffer, BUFFERSIZE);
		unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);

		populateCategories(fCatList,&fAddressAppInfo.category);
		getAllAddresses(addressDB,config);

	}
	else
	{
		populateCategories(fCatList,0L);
		kdWarning() << __FUNCTION__ 
			<< ": Could not open local AddressDB" << endl;
	}

	delete addressDB;

	updateWidget();
}

void
AddressWidget::preHotSync(char*)
{
	FUNCTIONSETUP;
}

void
AddressWidget::postHotSync()
{
	FUNCTIONSETUP;

	fAddressList.clear();
	initialize();
}


void
AddressWidget::setupWidget()
{
	FUNCTIONSETUP;

	QLabel *label;
	QGridLayout *grid=new QGridLayout(this,6,4,SPACING);

	fCatList = new QComboBox(this);
	grid->addWidget(fCatList,0,1);
	connect(fCatList, SIGNAL(activated(int)), 
		this, SLOT(slotSetCategory(int)));
	QToolTip::add(fCatList,
		i18n("Select the category of addresses\n"
			"to display here."));

	label = new QLabel(i18n("Category:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label,0,0);

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox,1,1,0,1);
	connect(fListBox, SIGNAL(highlighted(int)), 
		this, SLOT(slotShowAddress(int)));
	connect(fListBox, SIGNAL(selected(int)), 
		this, SLOT(slotEditRecord()));
	QToolTip::add(fListBox,
		i18n("This list displays all the addresses\n"
			"in the selected category. Click on\n"
			"one to display it to the right."));

	label = new QLabel(i18n("Address Info:"), this);
	grid->addWidget(label,0,2);

	// address info text view
	fAddrInfo = new QTextView(this);
	fAddrInfo->setPaper(this->backgroundColor());
	grid->addMultiCellWidget(fAddrInfo, 1, 4, 2, 2);
			
	QPushButton* button ;

	fEditButton = new QPushButton(i18n("Edit Record"), this);
	grid->addWidget(fEditButton,2,0);
	connect(fEditButton, SIGNAL(clicked()), 
		this, SLOT(slotEditRecord()));
	QToolTip::add(fEditButton,
		i18n("You can edit an address when it is selected."));

	button = new QPushButton(i18n("New Record"), this);
	grid->addWidget(button,2,1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	QToolTip::add(button,
		i18n("Add a new address to the address book."));

	fDeleteButton = new QPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton,3,0);
	connect(fDeleteButton, SIGNAL(clicked()), 
		this, SLOT(slotDeleteRecord()));
	QToolTip::add(fDeleteButton,
		i18n("Delete the selected address from the address book."));

	button = new QPushButton(i18n("Import List"), this);
	grid->addWidget(button,4,0);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportAddressList()));
	QToolTip::add(button,
		i18n("You can import a CSV list of addresses from a file."));

	button = new QPushButton(i18n("Export List"), this);
	grid->addWidget(button,4,1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExportAddressList()));
	QToolTip::add(button,
		i18n("Exports all the addresses in the address book to "
			"a CSV file."));
}

void
AddressWidget::updateWidget()
{
	FUNCTIONSETUP;

	int addressDisplayMode = KPilotOptionsAddress::getDisplayMode(
		KPilotConfig::getConfig());
	int listIndex = 0;

#ifdef DEBUG
	{
		kdDebug() << fname << ": Display Mode=" << 
			addressDisplayMode << endl ;
	}
#endif

	int currentCatID = findSelectedCategory(fCatList,
		&(fAddressAppInfo.category));

	fListBox->clear();
	fAddressList.first();
#ifdef DEBUG
	{
		kdDebug() << fname << ": Adding records..."
			<< endl;
	}
#endif

	while(fAddressList.current())
	{
		if((currentCatID==-1) || 
			(fAddressList.current()->getCat() == currentCatID))
		{
			// Title is dynamically allocated
			// by createTitle; older comments
			// suggest that that storage space is
			// freed by the listbox, so we'll leave it
			// that way.
			//
			//
			char *title=createTitle(fAddressList.current(),
				addressDisplayMode);
			if (title)
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
	{
		kdDebug() << fname
			<< ": " << listIndex << " records" << endl;
	}
#endif

	slotUpdateButtons();
}



char *AddressWidget::createTitle(PilotAddress *address,int displayMode)
{
	FUNCTIONSETUP;

	// The list will delete it..
	char *title = new char[255];
	if (title == 0L)
	{
		kdWarning() << __FUNCTION__ 
			<< ": Cannot allocate title string." << endl;
		return 0L;
	}

	title[0] = (char)0; // in case the first copy fails

	switch(displayMode)
	{
		case 1 :
			if(address->getField(entryCompany) &&
				strcmp(address->getField(entryCompany),""))
			{
				strcpy(title,address->getField(entryCompany));
			}
			if(address->getField(entryLastname) &&
				strcmp(address->getField(entryLastname),""))
			{
				if(title[0])
				{
					strcat(title, ", ");
				}
				strcat(title,
					address->getField(entryLastname));
			}
			break;
		case 0 :
		default :
			if(address->getField(entryLastname) &&
				strcmp(address->getField(entryLastname), ""))
			{
				strcpy(title, 
					address->getField(entryLastname));
			}

			if(address->getField(entryFirstname) && 
				strcmp(address->getField(entryFirstname), ""))
			{
				if(title[0])
				{
					strcat(title, ", ");
				}
				strcat(title, 
					address->getField(entryFirstname));
			}
		break;
	}

	if(title[0] == 0L) // One last try
	{
		if (fAddressList.current()->getField(entryCompany))
		{
			strcpy(title, fAddressList.current()->
				getField(entryCompany));
		}
		if (title[0] == 0)
		{
			QString t = i18n("[unknown]");
			strcpy(title,t.local8Bit());
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

void
AddressWidget::slotSetCategory(int)
{
	FUNCTIONSETUP;

  updateWidget();
}

void
AddressWidget::slotEditRecord()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();
	if(item == -1) return;

	PilotListItem *p = (PilotListItem *)fListBox->item(item);
	PilotAddress *selectedRecord = (PilotAddress *)p->rec();

	if (selectedRecord->id() == 0)
	{
		KMessageBox::error(0L, 
			i18n("Cannot edit new records until\n"
				"Hot-Synced with Pilot."),
			i18n("Hot-Sync Required"));
		return;
	}

	AddressEditor* editor = new AddressEditor(selectedRecord,
		&fAddressAppInfo,this);
	connect(editor, SIGNAL(recordChangeComplete(PilotAddress*)),
		this, SLOT(slotUpdateRecord(PilotAddress*)));
	editor->show();
}

void
AddressWidget::slotCreateNewRecord()
{
	FUNCTIONSETUP;

	// Response to bug 18072: Don't even try to
	// add records to an empty or unopened database,
	// since we don't have the DBInfo stuff to deal with it.
	//
	//
	PilotDatabase *myDB = new PilotLocalDatabase(dbPath(),"AddressDB");
	if (!myDB || !myDB->isDBOpen())
	{
		KMessageBox::sorry(this,
			i18n("You can't add addresses to the address book\n"
			     "until you have done a HotSync at least once\n"
			     "to retrieve the database layout from your Palm."),
			i18n("Can't add new address"));

		if (myDB) delete myDB;
		return;
	}

	AddressEditor* editor = new AddressEditor(0L,
		&fAddressAppInfo,this);
	connect(editor, SIGNAL(recordChangeComplete(PilotAddress*)),
		this, SLOT(slotAddRecord(PilotAddress*)));
	editor->show();
}

void
AddressWidget::slotAddRecord(PilotAddress* address)
{
	FUNCTIONSETUP;

	int currentCatID = findSelectedCategory(fCatList,
		&(fAddressAppInfo.category),true);


	address->setCat(currentCatID);
	fAddressList.append(address);
	writeAddress(address);
	// TODO: Just add the new record to the lists
	updateWidget();

	// k holds the item number of the address just added.
	//
	//
	int k = fListBox->count() - 1 ;
	fListBox->setCurrentItem(k); // Show the newest one
	fListBox->setBottomItem(k);
}

void
AddressWidget::slotUpdateRecord(PilotAddress* address)
{
	FUNCTIONSETUP;

	writeAddress(address);
	int currentRecord = fListBox->currentItem();
	// TODO: Just change the record
	updateWidget();
	fListBox->setCurrentItem(currentRecord);

	emit(recordChanged(address));
}

void
AddressWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();
	if(item == -1) return;

	PilotListItem * p = (PilotListItem *)fListBox->item(item);
	PilotAddress * selectedRecord = (PilotAddress *)p->rec();

	if (selectedRecord->id() == 0)
	{
		KMessageBox::error(this, 
			i18n("Cannot delete new records until\n"
				"Hot-Synced with pilot."), 
			i18n("Hot-Sync Required"));
		return;
	}

	if(KMessageBox::questionYesNo(this, 
		i18n("Delete currently selected record?"),
		i18n("Delete Record?")) == KMessageBox::No)
		return;

	selectedRecord->makeDeleted();
	writeAddress(selectedRecord);
	emit(recordChanged(selectedRecord));
	initialize();
}



void
AddressWidget::slotShowAddress(int which)
{
    FUNCTIONSETUP;
    
    PilotListItem *p    = (PilotListItem *)fListBox->item(which);
    PilotAddress  *addr = (PilotAddress *)p->rec();
    int i;

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
    if(addr->getField(entryTitle))
    {
	text += addr->getField(entryTitle);
	text += " ";
    }
    text += "<b><big>";
    if(addr->getField(entryFirstname))
    {
	text += addr->getField(entryFirstname);
	text += " ";
    }
    text += addr->getField(entryLastname);
    text += "</big></b>";
    text += "</p>";

    // company
    if(addr->getField(entryCompany))
    {
	text += "<p>";
	text += addr->getField(entryCompany);
	text += "</p>";
    }

    // phone numbers (+ labels)
    text += "<p>";
    for(i = entryPhone1; i <= entryPhone5; i++)
	if(addr->getField(i))
	{
	    text += "<small>";
	    text += fAddressAppInfo
		    .phoneLabels[addr->getPhoneLabelIndex(i-entryPhone1)];
	    text += ": </small>";
 	    if(addr->getShownPhone() == i-entryPhone1)
		text += "<b>";
	    text += addr->getField(i);
 	    if(addr->getShownPhone() == i-entryPhone1)
		text += "</b>";
	    text += "<br/>";
	}
    text += "</p>";

    // address, city, state, country
    text += "<p>";
    if(addr->getField(entryAddress))
    {
	text += addr->getField(entryAddress);
	text += "<br/>";
    }
    if(addr->getField(entryCity))
    {
	text += addr->getField(entryCity);
	text += " ";
    }
    if(addr->getField(entryState))
    {
	text += addr->getField(entryState);
	text += " ";
    }
    if(addr->getField(entryZip))
    {
	text += addr->getField(entryZip);
    }
    text += "<br/>";
    if(addr->getField(entryCountry))
    {
	text += addr->getField(entryCountry);
	text += "<br/>";
    }
    text += "</p>";

    // custom fields
    text += "<p>";
    for(i = entryCustom1; i <= entryCustom4; i++)
	if(addr->getField(i))
	{
	    text += addr->getField(i);
	    text += "<br/>";
	}
    text += "</p>";

    // note
    if(addr->getField(entryNote))
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



void
AddressWidget::slotImportAddressList()
    {
    	FUNCTIONSETUP;

    char importFormat[255];
    char delim[2];
    char nextField[255];
    bool useKeyField;

    KConfig* config = KGlobal::config();
    PilotAddress* currentAddress;

    QString fileName = KFileDialog::getOpenFileName();
    if(fileName == 0L)
 	return;
    QFile inFile(fileName);
    if(inFile.open(IO_ReadOnly) == FALSE)
 	{
		kdWarning() << __FUNCTION__ << ": Can't open file "
			<< fileName
			<< " read-only.\n";
 	// show error!
	KMessageBox::error(0L,
			   i18n("Can't open the address file for import:")+fileName,
			   i18n("Import Address Error"));
 	return;
 	}
    QTextStream inputStream(&inFile);
    config->setGroup("Address Widget");
    useKeyField = config->readNumEntry("UseKeyField");
    QString nextRecord;

	// Moved this out of the while loop since it should be constant.
	//
	//
	strcpy(importFormat, config->readEntry("IncomingFormat").latin1());
	delim[0] = importFormat[3];
	delim[1] = 0L;

#ifdef DEBUG
	{
		kdDebug() << fname << ": Input format is " << importFormat <<
			endl;
		kdDebug() << fname << ": Delimiter is " << delim << endl;
	}
#endif

    while(inputStream.eof() == false)
	{

	
	StrTokenizer* formatTokenizer = new StrTokenizer(importFormat, delim);
	StrTokenizer* dataTokenizer;
	const char* nextToken = formatTokenizer->getNextField();

 	nextRecord = inputStream.readLine();
	nextRecord = nextRecord.stripWhiteSpace();
 	if(inputStream.eof() || nextRecord.isNull())
	{
 	    break;
	}
#ifdef DEBUG
	{
		kdDebug() << fname << ": Read line " << nextRecord;
	}
#endif

	dataTokenizer = new StrTokenizer(nextRecord.latin1(), delim);
	strcpy(nextField, dataTokenizer->getNextField());
 	if(useKeyField)
	{
	    currentAddress = findAddress(nextField, nextToken);
	}
	else
	{
	    currentAddress = 0L;
	}
 	if(currentAddress == 0L)
 	    {
 	    currentAddress = new PilotAddress(fAddressAppInfo);
 	    fAddressList.append(currentAddress);
 	    }
 	setFieldBySymbol(currentAddress, nextToken, nextField);
  	while((nextToken = formatTokenizer->getNextField()) != NULL)
 	    {
	    strcpy(nextField, dataTokenizer->getNextField());
 	    setFieldBySymbol(currentAddress, nextToken, nextField);
 	    }
	delete formatTokenizer;
	delete dataTokenizer;
	writeAddress(currentAddress);
 	}
    inFile.close();
    updateWidget();
    }

PilotAddress*
AddressWidget::findAddress(const char* text, const char* symbol)
    {
    	FUNCTIONSETUP;

    for(fAddressList.first() ; fAddressList.current(); fAddressList.next())
	if(strcmp(text, getFieldBySymbol(fAddressList.current(), symbol)) == 0)
	    return fAddressList.current();
    return 0L;
    }

void
AddressWidget::setFieldBySymbol(PilotAddress* rec, const char* symbol, const char* text)
{
	FUNCTIONSETUP;
	int rc=-1;

	if((rc=strcasecmp(symbol, "%LN")) == 0)
	rec->setField(entryLastname, text);
	else if((rc=strcasecmp(symbol, "%FN")) == 0)
	rec->setField(entryFirstname, text);
	else if((rc=strcasecmp(symbol, "%CO")) == 0)
	rec->setField(entryCompany, text);
	else if((rc=strcasecmp(symbol, "%P1")) == 0)
	rec->setField(entryPhone1, text);
	else if((rc=strcasecmp(symbol, "%P2")) == 0)
	rec->setField(entryPhone2, text);
	else if((rc=strcasecmp(symbol, "%P3")) == 0)
	rec->setField(entryPhone3, text);
	else if((rc=strcasecmp(symbol, "%P4")) == 0)
	rec->setField(entryPhone4, text);
	else if((rc=strcasecmp(symbol, "%P5")) == 0)
	rec->setField(entryPhone5, text);
	else if((rc=strcasecmp(symbol, "%AD")) == 0)
	rec->setField(entryAddress, text);
	else if((rc=strcasecmp(symbol, "%CI")) == 0)
	rec->setField(entryCity, text);
	else if((rc=strcasecmp(symbol, "%ST")) == 0)
	rec->setField(entryState, text);
	else if((rc=strcasecmp(symbol, "%ZI")) == 0)
	rec->setField(entryZip, text);
	else if((rc=strcasecmp(symbol, "%CT")) == 0)
	rec->setField(entryCountry, text);
	else if((rc=strcasecmp(symbol, "%TI")) == 0)
	rec->setField(entryTitle, text);
	else if((rc=strcasecmp(symbol, "%C1")) == 0)
	rec->setField(entryCustom1, text);
	else if((rc=strcasecmp(symbol, "%C2")) == 0)
	rec->setField(entryCustom2, text);
	else if((rc=strcasecmp(symbol, "%C3")) == 0)
	rec->setField(entryCustom3, text);
	else if((rc=strcasecmp(symbol, "%C4")) == 0)
	rec->setField(entryCustom4, text);
	//     else if(strcasecmp(symbol, "%NO") == 0)
	// 	rec->setField(entryNote, text);
	else
	{
		kdWarning(KPILOT_AREA) << __FUNCTION__
			<< ": Unknown field "
			<< symbol 
			<< endl;
	}

	DEBUGKPILOT << fname << ": Set field " 
			<< symbol
			<< " to "
			<< text
			<< endl;
}

const char*
AddressWidget::getFieldBySymbol(PilotAddress* rec, const char* symbol)
    {
    	FUNCTIONSETUP;

    if(strcasecmp(symbol, "%LN") == 0)
	return rec->getField(entryLastname);
    else if(strcasecmp(symbol, "%FN") == 0)
	return rec->getField(entryFirstname);
    else if(strcasecmp(symbol, "%CO") == 0)
	return rec->getField(entryCompany);
    else if(strcasecmp(symbol, "%P1") == 0)
	return rec->getField(entryPhone1);
    else if(strcasecmp(symbol, "%P2") == 0)
	return rec->getField(entryPhone2);
    else if(strcasecmp(symbol, "%P3") == 0)
	return rec->getField(entryPhone3);
    else if(strcasecmp(symbol, "%P4") == 0)
	return rec->getField(entryPhone4);
    else if(strcasecmp(symbol, "%P5") == 0)
	return rec->getField(entryPhone5);
    else if(strcasecmp(symbol, "%AD") == 0)
	return rec->getField(entryAddress);
    else if(strcasecmp(symbol, "%CI") == 0)
	return rec->getField(entryCity);
    else if(strcasecmp(symbol, "%ST") == 0)
	return rec->getField(entryState);
    else if(strcasecmp(symbol, "%ZI") == 0)
	return rec->getField(entryZip);
    else if(strcasecmp(symbol, "%CT") == 0)
	return rec->getField(entryCountry);
    else if(strcasecmp(symbol, "%TI") == 0)
	return rec->getField(entryTitle);
    else if(strcasecmp(symbol, "%C1") == 0)
	return rec->getField(entryCustom1);
    else if(strcasecmp(symbol, "%C2") == 0)
	return rec->getField(entryCustom2);
    else if(strcasecmp(symbol, "%C3") == 0)
	return rec->getField(entryCustom3);
    else if(strcasecmp(symbol, "%C4") == 0)
	return rec->getField(entryCustom4);
//     else if(strcasecmp(symbol, "%NO") == 0)
// 	return rec->getField(entryNote);
    return 0L;
    }

void
AddressWidget::writeAddress(PilotAddress* which,PilotDatabase *addressDB)
{
	FUNCTIONSETUP;

	// Open a database (myDB) only if needed,
	// i.e. only if the passed-in addressDB
	// isn't valid.
	//
	//
	PilotDatabase *myDB=addressDB;
	bool usemyDB=false;

	if (myDB==0L || !myDB->isDBOpen())
	{
		myDB = new PilotLocalDatabase(dbPath(),"AddressDB");
		usemyDB=true;
	}

	// Still no valid address database...
	//
	//
	if (!myDB->isDBOpen())
	{
		DEBUGKPILOT << fname << ": Address database is not open.\n";
		return;
	}


	// Do the actual work.
	PilotRecord* pilotRec = which->pack();
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

void
AddressWidget::slotExportAddressList()
    {
    	FUNCTIONSETUP;

    char exportFormat[255];
    char delim[2];
    char* nextToken;
    KConfig* config = KGlobal::config();
    PilotAddress* currentAddress;

    QString fileName = KFileDialog::getSaveFileName();
    if(fileName == 0L)
 	return;
    QFile outFile(fileName);
    if(outFile.open(IO_WriteOnly | IO_Truncate) == FALSE)
 	{
		QString message = i18n("Can't open the "
			"address file %1 for export").arg(fileName);
		kdWarning() << __FUNCTION__ << ": " << message << endl;
		KMessageBox::error(0L,
			i18n("Export Address Error"),
			message);
 	return;
 	}
    QTextStream outStream(&outFile);
    config->setGroup("Address Widget");
      
    for(currentAddress = fAddressList.first(); currentAddress; currentAddress = fAddressList.next())
	{
	strcpy(exportFormat, config->readEntry("OutgoingFormat").latin1());
	delim[0] = exportFormat[3];
	delim[1] = 0L;

	nextToken = strtok(exportFormat, delim);
	outStream << getFieldBySymbol(currentAddress, nextToken);
	
	while((nextToken = strtok(NULL, delim)) != NULL)
	    {
	    outStream << delim;
	    outStream << getFieldBySymbol(currentAddress, nextToken);
	    }
	outStream << "\n";
	}
    outFile.close();
    }

// $Log$
// Revision 1.36  2001/08/27 22:51:41  adridg
// MartinJ's beautification of the address viewer
//
// Revision 1.35  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.34  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.33  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.32  2001/03/24 15:59:22  adridg
// Some populateCategories changes for bug #22112
//
// Revision 1.31  2001/03/19 23:12:39  stern
// Made changes necessary for upcoming abbrowser conduit.
//
// Mainly, I added two public methods to PilotAddress that allow for easier
// setting and getting of phone fields.
//
// I also have added some documentation throughout as I have tried to figure
// out how everything works.
//
// Revision 1.30  2001/03/11 10:50:38  adridg
// Make address editor reflect real field names
//
// Revision 1.29  2001/03/04 20:54:19  adridg
// Minor simplification
//
// Revision 1.28  2001/03/04 11:22:12  adridg
// In response to bug 21392, replaced fixed-length lookup table by a subclass
// of QListBoxItem inserted into list box. This subclass carries data to
// lookup the relevant pilot record.
//
// Revision 1.27  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.26  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.25  2001/02/07 14:21:37  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.24  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.23  2001/01/04 22:19:37  adridg
// Stuff for Chris and Bug 18072
//
// Revision 1.22  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.21  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added FUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.20  2000/12/13 16:59:27  adridg
// Removed dead code, i18n stupidities
//
// Revision 1.19  2000/12/05 07:43:28  adridg
// Fixed UI weirdness?
//
// Revision 1.18  2000/11/26 01:44:54  adridg
// Last of Heiko's patches
//
// Revision 1.17  2000/11/17 08:37:58  adridg
// Minor
//
// Revision 1.16  2000/11/14 23:02:28  adridg
// Layout and i18n issues
//
// Revision 1.15  2000/11/14 06:33:22  adridg
// Using Qt Layout code now
//
// Revision 1.14  2000/11/10 16:11:21  adridg
// Re-patched array overflows in category boxes
//
// Revision 1.13  2000/11/10 08:33:24  adridg
// General administrative
//
// Revision 1.12  2000/10/29 22:16:39  adridg
// Misc fixes
//
// Revision 1.11  2000/10/29 14:12:37  habenich
// removed compiler warnings
//
// Revision 1.10  2000/10/26 10:10:09  adridg
// Many fixes
//
// Revision 1.9  2000/10/25 12:19:44  adridg
// Conflicts resolved, pilot-link-0.0.0 bug workaround
//
