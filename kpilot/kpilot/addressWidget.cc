// addressWidget.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$

static char *id="$Id$";


// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		added "all" category.  The same kind of modifications
//		that were made to memoWidget could also be made right here.
//
//		Remaining questions are marked with QADE.

#include <kfiledialog.h>
#include <iostream.h>
#include <pi-macros.h>
#include <string.h>
#include <stdlib.h>

#include <qlist.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpushbt.h>
#include <qtextstream.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "strToken.h"
#include "addressWidget.moc"
#include "pi-dlp.h"
#include "pi-address.h"
#include "kpilot.h"
#include "addressEditor.h"
#include "kpilotOptions.h"
#include "options.h"


// This is the size of several (automatic) buffers,
// used to retrieve data from the database. 
// I have no idea if 0xffff is some magic number or not.
//
//
#define BUFFERSIZE	(0xffff)

AddressWidget::AddressWidget(KPilotInstaller* installer, QWidget* parent)
  : PilotComponent(parent), fTextWidget(0L)
{
	FUNCTIONSETUP;

	setGeometry(0, 0, 
		parent->geometry().width(), parent->geometry().height());
	setupWidget();
	fAddressList.setAutoDelete(true);
	installer->addComponentPage(this, i18n("Address Book"));
}

AddressWidget::~AddressWidget()
{
    	FUNCTIONSETUP;
}

void AddressWidget::setupCategories()
{
	FUNCTIONSETUP;
	int i;

	// Fill up the categories list box with
	// the categories defined by the user. 
	// These presumably are in the language 
	// the user uses, so no translation is necessary.
	//
	//
	for(i = 0; i < 15; i++)
	{
		if(strlen(fAddressAppInfo.category.name[i]))
		{
			if (debug_level & UI_MINOR)
			{
				kdDebug() << fname << 
				": Adding category: " << 
				fAddressAppInfo.category.name[i] << 
				" with ID: " << 
				(int)fAddressAppInfo.category.ID[i] << 
				endl;
			}
			fCatList->insertItem(
				fAddressAppInfo.category.name[i]);
		}
	}
}

int AddressWidget::getAllAddresses(PilotDatabase *addressDB,KConfig *config)
{
	FUNCTIONSETUP;

	int currentRecord = 0;
	PilotRecord* pilotRec;
	PilotAddress* address;
	bool showSecrets=0;

	config->setGroup(QString());
	showSecrets = (bool) config->readNumEntry("ShowSecrets");



	if (debug_level & DB_TEDIOUS)
	{
		kdDebug() << fname << ": Reading AddressDB...";
	}

	while((pilotRec = addressDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) && 
			(!(pilotRec->isSecret()) || showSecrets))
		{
			address = new PilotAddress(pilotRec);
			if (address == 0L)
			{
				kdDebug() << fname << ": Couldn't allocate "
					"record " << currentRecord++ <<
					endl;
				break;
			}
			fAddressList.append(address);
		}
		delete pilotRec;
		currentRecord++;
		// Print out a . after every 16 records (whenever
		// the low four bits of the record number are 0)
		//
		//
		if ((debug_level & DB_TEDIOUS) && !(currentRecord & 0xf))
		{
			kdDebug() << '.' ;
		}
	}

	if (debug_level & DB_TEDIOUS)
	{
		kdDebug() << '(' << currentRecord << " records)" << endl;
	}

	return currentRecord;
}

void
AddressWidget::initialize()
{
    	FUNCTIONSETUP;

	PilotDatabase* addressDB = 
		KPilotLink::getPilotLink()->openLocalDatabase("AddressDB");
	unsigned char buffer[BUFFERSIZE];
	int appLen;

	KConfig* config = KGlobal::config();

	fCatList->clear();
	fCatList->insertItem(i18n("All"));

	fAddressList.clear();

	if(addressDB->isDBOpen())
	{
		appLen = addressDB->readAppBlock(buffer, BUFFERSIZE);
		unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);

		setupCategories();
		getAllAddresses(addressDB,config);

		KPilotLink::getPilotLink()->closeDatabase(addressDB);
	}
	else
	{
		kdDebug() << fname << ": Could not open local AddressDB" << endl;
	}


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

bool
AddressWidget::saveData()
{
	FUNCTIONSETUP;

	return true;
}


void
AddressWidget::setupWidget()
{
	FUNCTIONSETUP;

	fCatList = new QComboBox(this);
	fCatList->move(110, 25);
	connect(fCatList, SIGNAL(activated(int)), 
		this, SLOT(slotSetCategory(int)));
	QLabel* label = new QLabel(i18n("Addresses:"), this);
	label->move(10, 30);
	fListBox = new QListBox(this);
	fListBox->setGeometry(10, 60, 200, 150);
	connect(fListBox, SIGNAL(highlighted(int)), 
		this, SLOT(slotShowAddress(int)));
	connect(fListBox, SIGNAL(selected(int)), 
		this, SLOT(slotEditRecord()));
	label = new QLabel(i18n("Address Info:"), this);
	label->move(290, 0);
	fTextWidget = new QMultiLineEdit(this, "textArea");
	fTextWidget->setGeometry(230, 30, 260, 290);
	fTextWidget->setFont(QFont("fixed", 10));
	fTextWidget->setReadOnly(TRUE);
	QPushButton* button = new QPushButton(i18n("Edit Record"), this);
	button->move(10, 220);
	connect(button, SIGNAL(clicked()), this, SLOT(slotEditRecord()));
	button = new QPushButton(i18n("New Record"), this);
	button->move(110, 220);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	button = new QPushButton(i18n("Delete Record"), this);
	button->move(60, 250);
	connect(button, SIGNAL(clicked()), this, SLOT(slotDeleteRecord()));
	button = new QPushButton(i18n("Import List"), this);
	button->move(10, 290);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportAddressList()));
	button = new QPushButton(i18n("Export List"), this);
	button->move(110, 290);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExportAddressList()));
}

void
AddressWidget::updateWidget()
{
	FUNCTIONSETUP;

	int addressDisplayMode = KPilotOptionsAddress::getDisplayMode();
	int listIndex = 0;
	int currentEntry = 0;

	if (debug_level & UI_MINOR)
	{
		kdDebug() << fname << ": Display Mode=" << 
			addressDisplayMode << '\n' ;
	}

	if (fCatList->currentItem()==-1)
	{
		kdDebug() << fname <<
			": No category selected in address book.\n";
		return;
	}
	// Semantics of currentCatID are:
	//
	// >=0 is a specific category based on the text -> category number
	//      mapping defined by the Pilot,
	// ==-1 means "All" category selected.
	//
	//
	int currentCatID = 0;

	if (fCatList->currentItem()==0)
	{
		currentCatID=-1;
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname << 
				": Category all selected.\n";
		}
	}
	else
	{
		// If a category is deleted after others 
		// have been added, none of the
		// category numbers are changed.  
		// So we need to find the category number
		// for this category.
		while(strcmp(fAddressAppInfo.category.name[currentCatID], 
			fCatList->text(fCatList->currentItem()).latin1()) &&
			(currentCatID < fCatList->count()))
		{
			currentCatID++;
		}

		if (currentCatID >= fCatList->count())
		{
			kdDebug() << fname << 
				": Can't find selected category!\n";
			currentCatID=-1; // All category
		}
	}

	fListBox->clear();
	fAddressList.first();
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Adding records...";
	}

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
				fListBox->insertItem(title, -1);
				fLookupTable[currentEntry++] = listIndex;
			}
		}
		listIndex++;
		fAddressList.next();
		// Print a . every 16 records (when the low 4
		// bits of the index are 0)
		//
		//
		if ((debug_level & UI_TEDIOUS) && !(listIndex & 0xf))
		{
			kdDebug() << '.' ;
		}
	}
	fTextWidget->clear();
	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << '(' << listIndex << " records)" << endl;
	}
}

char *AddressWidget::createTitle(PilotAddress *address,int displayMode)
{
	FUNCTIONSETUP;

	// The list will delete it..
	char *title = new char[255];
	if (title == 0L)
	{
		kdDebug() << fname << ": Cannot allocate title string." << endl;
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
		strcpy(title, fAddressList.current()->getField(entryCompany));
	}

	return title;
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
    if(item == -1)
      return;
    item = fLookupTable[item];
    if(fAddressList.at(item)->id() == 0x0)
      {
	KMessageBox::error(0L, 
		       i18n("Cannot edit new records until \r\n"
			    "Hot-Synced with pilot."),
		       i18n("Hot-Sync Required"));
			 
	return;
      }
    AddressEditor* editor = new AddressEditor(fAddressList.at(item));
    connect(editor, SIGNAL(recordChangeComplete(PilotAddress*)),
	    this, SLOT(slotUpdateRecord(PilotAddress*)));
    editor->show();
  }

void
AddressWidget::slotCreateNewRecord()
{
	FUNCTIONSETUP;

  AddressEditor* editor = new AddressEditor(0L);
  connect(editor, SIGNAL(recordChangeComplete(PilotAddress*)),
	  this, SLOT(slotAddRecord(PilotAddress*)));
  editor->show();
}

void
AddressWidget::slotAddRecord(PilotAddress* address)
{
	FUNCTIONSETUP;

  int currentCatID = 0;

  // If a category is deleted after others have been added, none of the
  // category numbers are changed.  So we need to find the category number
  // for this category.
  while(strcmp(fAddressAppInfo.category.name[currentCatID], 
	       fCatList->text(fCatList->currentItem()).latin1()))
    currentCatID++;
  address->setCat(currentCatID);
  fAddressList.append(address);
  writeAddress(address);
  updateWidget();
  fListBox->setCurrentItem(fAddressList.count() - 1); // Show the newest one
}

void
AddressWidget::slotUpdateRecord(PilotAddress* address)
{
	FUNCTIONSETUP;

  writeAddress(address);
  int currentRecord = fListBox->currentItem();
  updateWidget();
  fListBox->setCurrentItem(currentRecord);
}

void
AddressWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;

  int item = fListBox->currentItem();
  if(item == -1)
    return;
  item = (int)fLookupTable[item];
  if(fAddressList.at(item)->id() == 0x0)
    {
      KMessageBox::error(0L, 
			 i18n("Cannot delete new records until \r\n Hot-Synced with pilot."), 
			 i18n("Hot-Sync Required"));
      return;
    }
  if(KMessageBox::questionYesNo(0L, i18n("Delete currently selected record?"),
				i18n("Delete Record?")) == KMessageBox::No)
    return;
  PilotAddress* address = fAddressList.at(item);
  address->setAttrib(address->getAttrib() | dlpRecAttrDeleted);
  writeAddress(address);
  initialize();
}

void
AddressWidget::slotShowAddress(int which)
    {
    	FUNCTIONSETUP;

    char text[BUFFERSIZE];
    PilotAddress* theAdd = fAddressList.at(fLookupTable[which]);
    int i;
    int pad;
    text[0] = 0L;

    for(i = 0; i < 19; i++)
	{
	  if ((i > 2) && (i < 8))
	    {
	      strcat(text, fAddressAppInfo.phoneLabels[theAdd->getPhoneLabelIndex(i-3)]);
	      pad = 10 -
	      strlen(fAddressAppInfo.phoneLabels[theAdd->getPhoneLabelIndex(i-3)]);
	    }
	  else
	    {
	      strcat(text, fAddressAppInfo.labels[i]);
	      pad = 10 - strlen(fAddressAppInfo.labels[i]);
	    }
 	strcat(text, ": ");
	while(pad > 0)
	    {
	    strcat(text, " ");
	    pad--;
	    }
	if(theAdd->getField(i))
	    strcat(text, theAdd->getField(i));
	if(i < 18)
	    strcat(text, "\n"); // So there's no return after note, it fits nicer.
	}
    fTextWidget->setText(text);
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
	if (debug_level)
	{
		kdDebug() << fname << ": Can't open file "
			<< fileName
			<< " read-only.\n";
	}
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

	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Input format is " << importFormat <<
			endl;
		kdDebug() << fname << ": Delimiter is " << delim << endl;
	}

    while(inputStream.eof() == false)
	{

	
	StrTokenizer* formatTokenizer = new StrTokenizer(importFormat, delim);
	StrTokenizer* dataTokenizer;
	const char* nextToken = formatTokenizer->getNextField();

 	nextRecord = inputStream.readLine();
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Read line " << nextRecord;
	}

 	if(inputStream.eof() || (nextRecord == ""))
	{
 	    break;
	}

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
 	    currentAddress = new PilotAddress();
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

	if ((debug_level & SYNC_MINOR) && rc)
	{
		kdDebug() << fname << ": Unknown field "
			<< symbol << endl;
	}
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Set field " 
			<< symbol
			<< " to "
			<< text
			<< endl;
	}
}

char*
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
		myDB=KPilotLink::getPilotLink()->
			openLocalDatabase("AddressDB");
		usemyDB=true;
	}

	// Still no valid address database...
	//
	//
	if (!myDB->isDBOpen())
	{
		kdDebug() << fname << ": Address database is not open.\n";
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
		KPilotLink::getPilotLink()->closeDatabase(myDB);
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
 	// show error!
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
