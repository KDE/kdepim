// memoWidget.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 



// REVISION HISTORY 
//
// 3.1b9	By Dan Pilone
// 3.1b10	By Adriaan de Groot: comments added all over the place,
//		added "all" category. Started code-layout war by imposing
//		my own layout style. Added many robustness checks and
//		errors (to stderr).
//
//		Remaining questions are marked with QADE.

static const char *id="$Id$";



#include <time.h>
#include <iostream.h>
#include <pi-macros.h>

#include <qdir.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpushbt.h>
#include <kapp.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "kpilot.h"
#include "options.h"
#include "memoWidget.moc"
#include "pi-dlp.h"

// QADE: Is this a Pilot limitation, or is it a KPilot limitation?
int MemoWidget::MAX_MEMO_LEN = 8192;

// This constant (0xffff) appears all over the place (mostly in
// ::initialize(), but elsewhere as well. It seems to be inherited 
// from the pilot-link library.
//
// I've replaced instances of the constant with this #define
//
//
#define PILOT_BUFFER_SIZE	(0xffff)



MemoWidget::MemoWidget(KPilotInstaller* installer, QWidget* parent)
  : PilotComponent(parent), fTextWidget(0L)
{
	FUNCTIONSETUP;

	setGeometry(0, 0, 
		parent->geometry().width(), parent->geometry().height());
	setupWidget();
	initialize();
	fMemoList.setAutoDelete(true);
	installer->addComponentPage(this, "Memos");
}

MemoWidget::~MemoWidget()
{
	FUNCTIONSETUP;

}

// void MemoWidget::initializeCategories(PilotDatabase *memoDB)
//
// Fill up the categories combobox with a list of all the
// categories available on the Pilot.
//
void MemoWidget::initializeCategories(PilotDatabase *memoDB)
{
	FUNCTIONSETUP;

	int i;
	fCatList->clear();
	// Get all the category names. The "All" category isn't
	// in the list of category names, so add it here by
	// hand.
	//
	//
	fCatList->insertItem(i18n("All"));

	unsigned char buffer[PILOT_BUFFER_SIZE];

	int appLen = memoDB->readAppBlock(buffer, sizeof(buffer));
	unpack_MemoAppInfo(&fMemoAppInfo, buffer, appLen);

	for(i = 0; i < 15; i++)
	{
		if(strlen(fMemoAppInfo.category.name[i]))
		{
			fCatList->insertItem(fMemoAppInfo.category.name[i]);
			if (debug_level & UI_MINOR)
			{
				cerr << fname << 
					": Added category " <<
					i << '=' <<
					fMemoAppInfo.category.name[i] <<
					'\n' ;
			}
		}
	}
}

// void MemoWidget::initializeMemos(PilotDatabase *memoDB)
//
// Reads all the memos from the local database and places them
// in the selection screen.
//
void
MemoWidget::initializeMemos(PilotDatabase *memoDB)
{
	FUNCTIONSETUP;


	// ShowSecrets tells us to also list memos with an attribute of "Secret"
	// or "Private"
	//
	KConfig* config = KGlobal::config();
	bool showSecrets=false;
	config->setGroup(QString());
	showSecrets = (bool) config->readNumEntry("ShowSecrets");

	fMemoList.clear();





	int currentRecord = 0;
	PilotRecord* pilotRec;
	PilotMemo* memo;

	while((pilotRec = memoDB->readRecordByIndex(currentRecord)) != NULL)
	{
		if(!pilotRec->isDeleted())
		{
			if((!pilotRec->isSecret()) || showSecrets)
			{
				memo = new PilotMemo(pilotRec);
				fMemoList.append(memo);
				if (debug_level & UI_TEDIOUS)
				{
					cerr << fname <<
						": Added memo "
						<< currentRecord
						<< endl ;
				}
			}
			else
			{
				if (debug_level&UI_TEDIOUS)
				{
					cerr << fname <<
						": Skipped secret record " <<
						currentRecord << endl ;
				}
			}
		}
		else
		{
			if (debug_level&UI_TEDIOUS)
			{
				cerr << fname << 
					": Skipped deleted record " <<
					currentRecord << endl ;
			}
		}

		delete pilotRec;
		currentRecord++;
	}
}


void
MemoWidget::initialize()
{
	FUNCTIONSETUP;


	// Get the local database - assume the call may fail and return 
	// NULL, or the database object may be returned but unopened.
	//
	//
	PilotDatabase* memoDB = 
		KPilotLink::getPilotLink()->openLocalDatabase("MemoDB");
	if (memoDB==NULL || !memoDB->isDBOpen())
	{
		cerr << fname << 
			": Can't open local database MemoDB\n" ;

		updateWidget();
		return;
	}

	initializeCategories(memoDB);
	initializeMemos(memoDB);

	KPilotLink::getPilotLink()->closeDatabase(memoDB);
	updateWidget();
}

void
MemoWidget::preHotSync(char*)
{
	FUNCTIONSETUP;
}

void
MemoWidget::postHotSync()
{
	FUNCTIONSETUP;
	fMemoList.clear();
	initialize();
}

bool
MemoWidget::saveData()
{
	FUNCTIONSETUP;
	return true;
}


// void MemoWidget::setupWidget()
//
// Setup all the GUI components by allocating them. 
//
// QADE: I don't like all these new() calls without checks -- unless
// Qt provides a new() that throws exceptions when new() fails to allocate
// enough memory.
//
void
MemoWidget::setupWidget()
{
	FUNCTIONSETUP;

	QLabel *label=NULL;
	QPushButton *button=NULL;

	fCatList = new QComboBox(this);
	fCatList->move(110, 25);
	connect(fCatList, SIGNAL(activated(int)), 
		this, SLOT(slotSetCategory(int)));

	label = new QLabel(i18n("Memos:"), this);
	label->move(10, 30);

	fListBox = new QListBox(this);
	fListBox->setGeometry(10, 60, 200, 150);
	connect(fListBox, SIGNAL(highlighted(int)), 
		this, SLOT(slotShowMemo(int)));

	label = new QLabel(i18n("Memo Text:"), this);
	label->move(290, 0);

	fTextWidget = new QMultiLineEdit(this, "textArea");
	fTextWidget->setGeometry(230, 30, 260, 290);
	connect(fTextWidget, SIGNAL(textChanged()), 
		this, SLOT(slotTextChanged()));

	button = new QPushButton(i18n("Import Memo"), this);
	button->move(10, 220);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportMemo()));

	button = new QPushButton(i18n("Export Memo"), this);
	button->move(110, 220);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExportMemo()));

	button = new QPushButton(i18n("Delete Memo"), this);
	button->move(60, 250);
	connect(button, SIGNAL(clicked()), this, SLOT(slotDeleteMemo()));
}

void 
MemoWidget::slotSetCategory(int)
{
	FUNCTIONSETUP;
	updateWidget();
}

void
MemoWidget::slotDeleteMemo()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();

	if(item == -1)
	{
		cerr << fname <<
			": No current item selected\n" ;
		return;
	}

	item = fLookupTable[item];
	if(fMemoList.at(item)->id() == 0x0)
	{
		// QADE: Why is this? What prevents us from deleting
		// a "new" memo, ie. one we've imported, before *ever*
		// sending it to the Pilot?
		//
		//
		if (debug_level)
		{
			cerr << fname <<
				": Refusing to delete new memo.\n";
		}

		KMessageBox::error(this, i18n("Hot-Sync Required"), 
			i18n("Cannot delete new memo until \r\n" 
				"Hot-Synced with pilot."));
		return;
	}

	
	if(KMessageBox::questionYesNo(this,  
				      i18n("Delete currently selected memo?"),
				      i18n("Delete Memo?")) == KMessageBox::No)
	{
		if (debug_level)
		{
			cerr << fname <<
				": Used decided not to delete memo.\n" ;
		}
		return;
	}

	PilotMemo* memo = fMemoList.at(item);
	// QADE: Apparently a PilotMemo is not some kind of PilotRecord,
	// so the PilotRecord methods don't work on it.
	//
	//
	memo->makeDeleted();
	memo->setAttrib(memo->getAttrib() | dlpRecAttrDeleted);
	writeMemo(memo);
	initialize();
}

void
MemoWidget::updateWidget()
{
	FUNCTIONSETUP;

	if (fCatList->currentItem()==-1)
	{
		cerr << fname <<
			": No category selected.\n";
		return ;
	}

	int listIndex = 0;
	int currentEntry = 0;

	// Semantics of currentCatID are: 
	//
	// >=0 is a specific category based on the text -> category number 
	// 	mapping defined by the Pilot, 
	// ==-1 means "All" category selected.
	//
	//
	int currentCatID = 0;

	// If a category is deleted after others have been added, none of the
	// category numbers are changed.  So we need to find the category number
	// for this category (this category is represented by the selected
	// *text*).
	//
	//
	// The top entry in the list is "All", so if the top item is
	// selected we can indicate that we are using the "All" category.
	//
	//
	if (fCatList->currentItem()==0)
	{
		currentCatID=-1;
		if (debug_level&UI_MINOR)
		{
			cerr << fname <<
				": Category 'All' selected.\n" ;
		}
	}
	else
	{
		QString selectedCategory=fCatList->text(fCatList->currentItem());
		if (debug_level&UI_MINOR)
		{
			cerr << fname << 
				": List item " << fCatList->currentItem() <<
				" selected, text=" <<
				selectedCategory << '\n' ;
		}

		currentCatID=0;
		while(strcmp(fMemoAppInfo.category.name[currentCatID], 
		       selectedCategory) && 
			(currentCatID < fCatList->count()))
		{
			if (debug_level&UI_TEDIOUS)
			{
				cerr << fname <<
					": Didn't match category " <<
					currentCatID << '=' <<
					fMemoAppInfo.category.name[currentCatID]
					<< '\n' ;
			}

			currentCatID++;
		}

		if (currentCatID < fCatList->count())
		{
			if (debug_level&UI_MINOR)
			{
				cerr << fname << 
					": Matched category " <<
					currentCatID << '=' <<
					fMemoAppInfo.category.name[currentCatID]
					<< '\n' ;
			}
		}
		else
		{
			cerr << fname << ": Selected category didn't match "
				"any name!\n" ;
			currentCatID=-1;
		}
	}

	fListBox->clear();
	fMemoList.first();


	// Iterate through all the memos and insert each memo
	// only if the category of the memo matches the selected category
	// (using -1 to mean "All")
	//
	//
	while(fMemoList.current())
	{
		if((fMemoList.current()->getCat() == currentCatID) ||
			(currentCatID==-1))
		{
			// List will delete it.
			fListBox->insertItem(fMemoList.current()->getTitle());
			fLookupTable[currentEntry++] = listIndex;
			if (debug_level & UI_TEDIOUS)
			{
				cerr << fname << ": Added memo "
					<< fMemoList.current()->getTitle()
					<< endl;
			}
		}
		else
		{
			if (debug_level & UI_TEDIOUS)
			{
				cerr << fname << ": Skipped memo "
					<< fMemoList.current()->getTitle()
					<< endl;
			}
		}

		listIndex++;
		fMemoList.next();
	}

	fTextWidget->clear();
}

void
MemoWidget::slotShowMemo(int which)
{      
  disconnect(fTextWidget, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
  fTextWidget->deselect();
  fTextWidget->setText(fMemoList.at(fLookupTable[which])->text());
  connect(fTextWidget, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
}

void
MemoWidget::writeMemo(PilotMemo* which)
{
	FUNCTIONSETUP;
	PilotDatabase* memoDB = 
		KPilotLink::getPilotLink()->openLocalDatabase("MemoDB");
	PilotRecord* pilotRec = which->pack();
	memoDB->writeRecord(pilotRec);
	delete pilotRec;
	KPilotLink::getPilotLink()->closeDatabase(memoDB);
}
    
void
MemoWidget::slotTextChanged()
{
  FUNCTIONSETUP;
  PilotMemo* currentMemo;

  if(fListBox->currentItem() >= 0)
    {
      currentMemo = fMemoList.at(fLookupTable[fListBox->currentItem()]);
      if(currentMemo->id() == 0x0)
	{
	  KMessageBox::error(0L,
			     i18n("Cannot edit new memo until \r\n Hot-Synced with pilot."), 
			     i18n("Hot-Sync Required"));
	  slotShowMemo(fListBox->currentItem());
	  return;
	}
      currentMemo->setText(fTextWidget->text().data());
      writeMemo(currentMemo);
    }
}

void
MemoWidget::slotImportMemo()
    {
    FUNCTIONSETUP;
    int i = 0;
    int nextChar;
    int currentCatID = -1;
  
    // If a category is deleted after others have been added, none of the
    // category numbers are changed.  So we need to find the category number
    // for this category.
    char notFound = 1;
    while (notFound)
      {
	currentCatID++;
	if (fMemoAppInfo.category.name[currentCatID])
	  notFound = strcmp(fMemoAppInfo.category.name[currentCatID],
			    fCatList->text(fCatList->currentItem()));
      }

    QString fileName = KFileDialog::getOpenFileName();
    if(fileName != NULL)
	{
	QFile importFile(fileName);
	if(importFile.open(IO_ReadOnly) == FALSE)
	    {
	    // show error!
	    return;
	    }
	char text[MemoWidget::MAX_MEMO_LEN];
	for(i = 0; (i < (MemoWidget::MAX_MEMO_LEN - 1)) && ((nextChar = importFile.getch()) != -1); i++)
	    text[i] = nextChar;
	text[i] = 0;
	PilotMemo* aMemo = new PilotMemo(text, 0, 0, currentCatID);
	fMemoList.append(aMemo);
	writeMemo(aMemo);
	updateWidget();
	}
    }

void
MemoWidget::slotExportMemo()
    {
    FUNCTIONSETUP;
    int item = fListBox->currentItem();
    const char* data;

    if(item == -1)
	return;
    item = fLookupTable[item];
    QString fileName = KFileDialog::getSaveFileName();
    if(fileName == 0L)
	return;

    data = fMemoList.at(item)->text();

    QFile outFile(fileName);
    if(outFile.open(IO_WriteOnly | IO_Truncate) == FALSE)
	{
	// show error!
	return;
	}
    QDataStream outStream(&outFile);
    outStream.writeRawBytes(data, strlen(data) + 1);
    outFile.close();
    }
