// addressWidget.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 

// $Revision$

static const char *id="$Id$";


// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		added "all" category.  The same kind of modifications
//		that were made to memoWidget could also be made right here.
//
//		Remaining questions are marked with QADE.

#include "options.h"

#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pi-macros.h>
#include "pi-dlp.h"
#include "pi-address.h"

#include <qlist.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpushbt.h>
#include <qtextstream.h>
#include <qlayout.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include "strToken.h"
#include "kpilot.h"
#include "addressEditor.h"
#include "kpilotOptions.h"

#include "addressWidget.moc"


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
	fTextWidget->setFont(installer->fixed());
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
#ifdef DEBUG
			if (debug_level & UI_MINOR)
			{
				kdDebug() << fname << 
				": Adding category: " << 
				fAddressAppInfo.category.name[i] << 
				" with ID: " << 
				(int)fAddressAppInfo.category.ID[i] << 
				endl;
			}
#endif
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



#ifdef DEBUG
	if (debug_level & DB_TEDIOUS)
	{
		kdDebug() << fname << ": Reading AddressDB..."
			<< endl;
	}
#endif

	while((pilotRec = addressDB->readRecordByIndex(currentRecord)) != 0L)
	{
		if (!(pilotRec->isDeleted()) && 
			(!(pilotRec->isSecret()) || showSecrets))
		{
			address = new PilotAddress(pilotRec);
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

#ifdef DEBUG
	if (debug_level & DB_TEDIOUS)
	{
		kdDebug() << fname 
			<< ": Total " << currentRecord << " records" << endl;
	}
#endif

	return currentRecord;
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
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

	fAddressList.clear();

	if(addressDB->isDBOpen())
	{
		appLen = addressDB->readAppBlock(buffer, BUFFERSIZE);
		unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);

		populateCategories(fCatList,&fAddressAppInfo.category);
		getAllAddresses(addressDB,config);

		KPilotLink::getPilotLink()->closeDatabase(addressDB);
	}
	else
	{
		populateCategories(fCatList,0L);
		kdWarning() << __FUNCTION__ 
			<< ": Could not open local AddressDB" << endl;
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

	QLabel *label;
	QGridLayout *grid=new QGridLayout(this,6,4,SPACING);

	fCatList = new QComboBox(this);
	grid->addWidget(fCatList,0,1);
	connect(fCatList, SIGNAL(activated(int)), 
		this, SLOT(slotSetCategory(int)));

	label = new QLabel(i18n("Category:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label,0,0);

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox,1,1,0,1);
	connect(fListBox, SIGNAL(highlighted(int)), 
		this, SLOT(slotShowAddress(int)));
	connect(fListBox, SIGNAL(selected(int)), 
		this, SLOT(slotEditRecord()));

	label = new QLabel(i18n("Address Info:"), this);
	grid->addWidget(label,0,2);

	fTextWidget = new QMultiLineEdit(this, "textArea");
	fTextWidget->setReadOnly(TRUE);
	grid->addMultiCellWidget(fTextWidget,1,4,2,2);

	QPushButton* button ;
	
	fEditButton = new QPushButton(i18n("Edit Record"), this);
	grid->addWidget(fEditButton,2,0);
	connect(fEditButton, SIGNAL(clicked()), 
		this, SLOT(slotEditRecord()));
	button = new QPushButton(i18n("New Record"), this);
	grid->addWidget(button,2,1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCreateNewRecord()));
	fDeleteButton = new QPushButton(i18n("Delete Record"), this);
	grid->addWidget(fDeleteButton,3,0);
	connect(fDeleteButton, SIGNAL(clicked()), 
		this, SLOT(slotDeleteRecord()));
	button = new QPushButton(i18n("Import List"), this);
	grid->addWidget(button,4,0);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportAddressList()));
	button = new QPushButton(i18n("Export List"), this);
	grid->addWidget(button,4,1);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExportAddressList()));
}

void
AddressWidget::updateWidget()
{
	FUNCTIONSETUP;

	int addressDisplayMode = KPilotOptionsAddress::getDisplayMode(
		KPilotLink::getConfig());
	int listIndex = 0;
	int currentEntry = 0;

#ifdef DEBUG
	if (debug_level & UI_MINOR)
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
	if (debug_level & UI_TEDIOUS)
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
				fListBox->insertItem(title, -1);
				fLookupTable[currentEntry++] = listIndex;
			}
		}
		listIndex++;
		fAddressList.next();
	}
	fTextWidget->clear();
#ifdef DEBUG
	if (debug_level & UI_TEDIOUS)
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
    if(item == -1)
      return;
    item = fLookupTable[item];
    if(fAddressList.at(item)->id() == 0x0)
      {
	KMessageBox::error(0L, 
		       i18n("Cannot edit new records until\n"
			    "Hot-Synced with Pilot."),
		       i18n("Hot-Sync Required"));
			 
	return;
      }
    AddressEditor* editor = new AddressEditor(fAddressList.at(item),this);
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

	int currentCatID = findSelectedCategory(fCatList,
		&(fAddressAppInfo.category),true);

  address->setCat(currentCatID);
  fAddressList.append(address);
  writeAddress(address);
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
  updateWidget();
  fListBox->setCurrentItem(currentRecord);

	emit(recordChanged(address));
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
      KMessageBox::error(this, 
			 i18n("Cannot delete new records until\n"
			      "Hot-Synced with pilot."), 
			 i18n("Hot-Sync Required"));
      return;
    }
  if(KMessageBox::questionYesNo(this, i18n("Delete currently selected record?"),
				i18n("Delete Record?")) == KMessageBox::No)
    return;
  PilotAddress* address = fAddressList.at(item);
  address->setAttrib(address->getAttrib() | dlpRecAttrDeleted);
  writeAddress(address);
	emit(recordChanged(address));
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

	int showWhichPhone = theAdd->getShownPhone();

    for(i = 0; i < 19; i++)
	{
	  if ((i > 2) && (i < 8))
	    {
	      strcat(text, fAddressAppInfo.phoneLabels[theAdd->getPhoneLabelIndex(i-3)]);
	      pad = PhoneNumberLength -
	      strlen(fAddressAppInfo.phoneLabels[theAdd->getPhoneLabelIndex(i-3)]);
		if (showWhichPhone == i-3)
		{
			strcat(text," []");
			pad -= 3;
		}
	    }
	  else
	    {
	      strcat(text, fAddressAppInfo.labels[i]);
	      pad = PhoneNumberLength - strlen(fAddressAppInfo.labels[i]);
	    }
 	strcat(text, ": ");
	if (i == 18)
	{
		strcat(text,"\n");
		pad=0;
	}
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
	if (debug_level & SYNC_MINOR)
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
	if (debug_level & SYNC_TEDIOUS)
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

#ifdef DEBUG
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
#endif
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
