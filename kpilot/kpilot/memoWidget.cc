/* memoWidget.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the memo-viewing widget (internal conduit) used by KPilot.
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
static const char *memowidget_id="$Id$";

#include "options.h"

#include <time.h>
#include <iostream.h>
#include <pi-macros.h>
#include <pi-dlp.h>

#include <qdir.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpushbt.h>
#include <qlayout.h>
#include <kapp.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "kpilot.h"
#include "kpilotConfig.h"
#include "listItems.h"
#include "memoWidget.moc"

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



MemoWidget::MemoWidget( QWidget* parent, const QString& path) : 
	PilotComponent(parent,path), 
	fTextWidget(0L)
{
	FUNCTIONSETUP;

	setGeometry(0, 0, 
		parent->geometry().width(), parent->geometry().height());
	setupWidget();
	initialize();
	fMemoList.setAutoDelete(true);
	fTextWidget->setFont(KPilotConfig::fixed());
	slotUpdateButtons();
}

MemoWidget::~MemoWidget()
{
	FUNCTIONSETUP;

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
	KConfig& config = KPilotConfig::getConfig();
	bool showSecrets=false;
	showSecrets = config.readBoolEntry("ShowSecrets");

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
#ifdef DEBUG
				if (debug_level & UI_TEDIOUS)
				{
					kdDebug() << fname <<
						": Added memo "
						<< currentRecord
						<< endl ;
				}
#endif
			}
			else
			{
#ifdef DEBUG
				if (debug_level&UI_TEDIOUS)
				{
					kdDebug() << fname <<
						": Skipped secret record " <<
						currentRecord << endl ;
				}
#endif
			}
		}
		else
		{
#ifdef DEBUG
			if (debug_level&UI_TEDIOUS)
			{
				kdDebug() << fname << 
					": Skipped deleted record " <<
					currentRecord << endl ;
			}
#endif
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
		new PilotLocalDatabase(dbPath(),"MemoDB");
	if (memoDB==NULL || !memoDB->isDBOpen())
	{
		kdWarning() << __FUNCTION__ << 
			": Can't open local database MemoDB\n" ;

		populateCategories(fCatList,0L);
		updateWidget();
		return;
	}

	// Normal case: there is a database so we can read it
	// and determine all the categories.
	//
	unsigned char buffer[PILOT_BUFFER_SIZE];
	int appLen = memoDB->readAppBlock(buffer, sizeof(buffer));
	unpack_MemoAppInfo(&fMemoAppInfo, buffer, appLen);

	populateCategories(fCatList,&fMemoAppInfo.category);
	initializeMemos(memoDB);

	delete memoDB;
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
#ifdef DEBUG
	/* NOTREACHED */
	(void) memowidget_id;
#endif
}


// void MemoWidget::setupWidget()
//
// Setup all the GUI components by allocating them. 
//
//
void
MemoWidget::setupWidget()
{
	FUNCTIONSETUP;

	QLabel *label=NULL;
	QPushButton *button=NULL;
	QGridLayout *grid = new QGridLayout(this,5,4,SPACING);

	fCatList = new QComboBox(this);
	grid->addWidget(fCatList,0,1);
	connect(fCatList, SIGNAL(activated(int)), 
		this, SLOT(slotSetCategory(int)));

	label = new QLabel(i18n("Memos:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label,0,0);

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox,1,1,0,1);
	connect(fListBox, SIGNAL(highlighted(int)), 
		this, SLOT(slotShowMemo(int)));
	connect(fListBox, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateButtons()));

	label = new QLabel(i18n("Memo Text:"), this);
	grid->addWidget(label,0,2);

	fTextWidget = new QMultiLineEdit(this, "textArea");
	grid->addMultiCellWidget(fTextWidget,1,4,2,2);
	connect(fTextWidget, SIGNAL(textChanged()), 
		this, SLOT(slotTextChanged()));

	button = new QPushButton(i18n("Import Memo"), this);
	grid->addWidget(button,2,0);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportMemo()));

	fExportButton = new QPushButton(i18n("Export Memo"), this);
	grid->addWidget(fExportButton,2,1);
	connect(fExportButton, SIGNAL(clicked()), this, SLOT(slotExportMemo()));

	fDeleteButton = new QPushButton(i18n("Delete Memo"), this);
	grid->addWidget(fDeleteButton,3,0);
	connect(fDeleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteMemo()));
}

void
MemoWidget::slotUpdateButtons()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();

	if (fExportButton)
	{
		fExportButton -> setEnabled(item != -1);
	}
	if (fDeleteButton)
	{
		fDeleteButton -> setEnabled(item != -1);
	}
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
#ifdef DEBUG
		kdDebug() << fname <<
			": No current item selected\n" ;
#endif
		return;
	}

	PilotListItem * p = (PilotListItem *)fListBox->item(item);
	PilotMemo * selectedMemo = (PilotMemo *)p->rec();
	if(selectedMemo->id() == 0x0)
	{
		// QADE: Why is this? What prevents us from deleting
		// a "new" memo, ie. one we've imported, before *ever*
		// sending it to the Pilot?
		//
		//
		kdWarning() << __FUNCTION__ <<
			": Refusing to delete new memo.\n";

		KMessageBox::error(this, i18n("Hot-Sync Required"), 
			i18n("Cannot delete new memo until \n" 
				"Hot-Synced with pilot."));
		return;
	}

	
	if(KMessageBox::questionYesNo(this,  
				      i18n("Delete currently selected memo?"),
				      i18n("Delete Memo?")) == KMessageBox::No)
	{
#ifdef DEBUG
		if (debug_level)
		{
			kdDebug() << fname <<
				": Used decided not to delete memo.\n" ;
		}
#endif
		return;
	}

	// QADE: Apparently a PilotMemo is not some kind of PilotRecord,
	// so the PilotRecord methods don't work on it.
	//
	//
	selectedMemo->makeDeleted();
	writeMemo(selectedMemo);
	initialize();
}


void
MemoWidget::updateWidget()
{
	FUNCTIONSETUP;

	if (fCatList->currentItem()==-1)
	{
		DEBUGKPILOT << fname <<
			": No category selected.\n";
		return ;
	}

	int listIndex = 0;
	int currentEntry = 0;
	int currentCatID = findSelectedCategory(fCatList,
		&(fMemoAppInfo.category),false);


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
			PilotListItem * p = new PilotListItem(
				fMemoList.current()->shortTitle(),
				listIndex,
				fMemoList.current());
			// List will delete the title of the memo,
			// so there's no memory leak here.
			//
			//
			fListBox->insertItem(p);

#ifdef DEBUG
			if (debug_level & UI_TEDIOUS)
			{
				kdDebug() << fname << ": Added memo "
					<< fMemoList.current()->getTitle()
					<< endl;
			}
#endif
		}
		else
		{
#ifdef DEBUG
			if (debug_level & UI_TEDIOUS)
			{
				kdDebug() << fname << ": Skipped memo "
					<< fMemoList.current()->getTitle()
					<< endl;
			}
#endif
		}

		listIndex++;
		fMemoList.next();
	}

	fTextWidget->clear();

	slotUpdateButtons();
}

void
MemoWidget::slotShowMemo(int which)
{      
	disconnect(fTextWidget, SIGNAL(textChanged()), 
		this, SLOT(slotTextChanged()));
	fTextWidget->deselect();
	PilotListItem *p = (PilotListItem *)fListBox->item(which);
	PilotMemo * theMemo = (PilotMemo *)p->rec();
	fTextWidget->setText(theMemo->text());
	connect(fTextWidget, SIGNAL(textChanged()), 
		this, SLOT(slotTextChanged()));
}

void
MemoWidget::writeMemo(PilotMemo* which)
{
	FUNCTIONSETUP;
	PilotDatabase* memoDB = 
		new PilotLocalDatabase(dbPath(),"MemoDB");
	PilotRecord* pilotRec = which->pack();
	memoDB->writeRecord(pilotRec);
	delete pilotRec;
	delete memoDB;
}
    
void
MemoWidget::slotTextChanged()
{
  FUNCTIONSETUP;
	if (fListBox->currentItem() == -1)
	{
		kdWarning() << __FUNCTION__
			<< ": slotTextChanged with no memo selected!"
			<< endl;
		return;
	}

  PilotListItem *p = (PilotListItem *)fListBox->item(fListBox->currentItem());
  PilotMemo* currentMemo = (PilotMemo *)p->rec();
  if(fListBox->currentItem() >= 0)
    {
      if(currentMemo->id() == 0x0)
	{
	  KMessageBox::error(0L,
			     i18n("Cannot edit new memo until \n Hot-Synced with pilot."), 
			     i18n("Hot-Sync Required"));
	  slotShowMemo(fListBox->currentItem());
	  return;
	}
      currentMemo->setText(fTextWidget->text().latin1());
      writeMemo(currentMemo);
    }
}

void
MemoWidget::slotImportMemo()
    {
    FUNCTIONSETUP;
    int i = 0;
    int nextChar;
	int currentCatID = findSelectedCategory(fCatList,
		&(fMemoAppInfo.category),true);
  
    QString fileName = KFileDialog::getOpenFileName();
    if(fileName != NULL)
	{
	QFile importFile(fileName);
	if(importFile.open(IO_ReadOnly) == FALSE)
	    {
	    // show error!
	    return;
	    }
	char *text = new char[MemoWidget::MAX_MEMO_LEN];
	for(i = 0; (i < (MemoWidget::MAX_MEMO_LEN - 1)) && ((nextChar = importFile.getch()) != -1); i++)
	    text[i] = nextChar;
	text[i] = 0;
	PilotMemo* aMemo = new PilotMemo(text, 0, 0, currentCatID);
	fMemoList.append(aMemo);
	writeMemo(aMemo);
	updateWidget();
	delete[] text;
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

	PilotListItem *p = (PilotListItem *)fListBox->item(item);
	PilotMemo* theMemo = (PilotMemo *)p->rec();

	QString fileName = KFileDialog::getSaveFileName();
	if(fileName.isEmpty()) return;

	data = theMemo->text();

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

// $Log$
// Revision 1.24  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.23  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.22  2001/02/07 14:21:43  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.21  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
