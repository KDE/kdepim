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
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *memowidget_id =
	"$Id$";

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <time.h>
#include <iostream.h>
#include <pi-macros.h>
#include <pi-dlp.h>

#ifndef QDIR_H
#include <qdir.h>
#endif
#ifndef QLIST_H
#include <qlist.h>
#endif
#ifndef QLISTBOX_H
#include <qlistbox.h>
#endif
#ifndef QFILE_H
#include <qfile.h>
#endif
#ifndef QPUSHBUTTON_H
#include <qpushbutton.h>
#endif
#ifndef QLAYOUT_H
#include <qlayout.h>
#endif
#ifndef QDOM_H
#include <qdom.h>
#endif
#ifndef QTEXTSTREAM_H
#include <qtextstream.h>
#endif
#ifndef QWHATSTHIS_H
#include <qwhatsthis.h>
#endif
#ifndef QLABEL_H
#include <qlabel.h>
#endif
#ifndef _KAPP_H
#include <kapplication.h>
#endif
#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif
#ifndef _KFILEDIALOG_H
#include <kfiledialog.h>
#endif

#ifndef _KPILOT_KPILOT_H
#include "kpilot.h"
#endif
#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif
#ifndef _KPILOT_LISTITEMS_H 
#include "listItems.h"
#endif
#ifndef _KPILOT_PILOTLOCALDATABASE_H
#include "pilotLocalDatabase.h"
#endif


#include "memoWidget.moc"


// This constant (0xffff) appears all over the place (mostly in
// ::initialize(), but elsewhere as well. It seems to be inherited 
// from the pilot-link library.
//
// I've replaced instances of the constant with this #define
//
//
#define PILOT_BUFFER_SIZE	(0xffff)



MemoWidget::MemoWidget(QWidget * parent,
	const QString & path) :
	PilotComponent(parent, "component_memo", path),
	fTextWidget(0L)
{
	FUNCTIONSETUP;

	setGeometry(0, 0,
		parent->geometry().width(), parent->geometry().height());
	setupWidget();
	initialize();
	fMemoList.setAutoDelete(true);
	slotUpdateButtons();

	connect(fTextWidget, SIGNAL(textChanged()),
		this, SLOT(slotTextChanged()));

	/* NOTREACHED */
	(void) memowidget_id;
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

void MemoWidget::initializeMemos(PilotDatabase * memoDB)
{
	FUNCTIONSETUP;


	// ShowSecrets tells us to also list memos with an attribute of "Secret"
	// or "Private"
	//
	bool showSecrets = KPilotConfig::getConfig().getShowSecrets();

	fMemoList.clear();





	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotMemo *memo;

	while ((pilotRec = memoDB->readRecordByIndex(currentRecord)) != NULL)
	{
		if (!pilotRec->isDeleted())
		{
			if ((!pilotRec->isSecret()) || showSecrets)
			{
				memo = new PilotMemo(pilotRec);
				fMemoList.append(memo);

#ifdef DEBUG
				DEBUGKPILOT << fname <<
					": Added memo "
					<< currentRecord << endl;
#endif
			}
			else
			{
#ifdef DEBUG
				DEBUGKPILOT << fname <<
					": Skipped secret record " <<
					currentRecord << endl;
#endif
			}
		}
		else
		{
#ifdef DEBUG
			DEBUGKPILOT << fname <<
				": Skipped deleted record " <<
				currentRecord << endl;
#endif
		}

		delete pilotRec;

		currentRecord++;
	}
}


void MemoWidget::initialize()
{
	FUNCTIONSETUP;


	// Get the local database - assume the call may fail and return 
	// NULL, or the database object may be returned but unopened.
	//
	//
	PilotLocalDatabase *memoDB =
		new PilotLocalDatabase(dbPath(), "MemoDB");
	if (memoDB == NULL || !memoDB->isDBOpen())
	{
		kdWarning() << k_funcinfo <<
			": Can't open local database MemoDB\n";

		populateCategories(fCatList, 0L);
		updateWidget();
		return;
	}

	// Normal case: there is a database so we can read it
	// and determine all the categories.
	//
	int appLen = memoDB->appInfoSize();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Got appInfoLen " << appLen << endl;
#endif

	unpack_MemoAppInfo(&fMemoAppInfo,
		(unsigned char *) memoDB->appInfo(), appLen);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Unpacked app info." << endl;

	for (int i = 0; i < 15; i++)
	{
		DEBUGKPILOT << fname
			<< ": Category #"
			<< i
			<< " has ID "
			<< (int) fMemoAppInfo.category.ID[i]
			<< " and name "
			<< (fMemoAppInfo.category.name[i][0] ? "*" : "-")
			<< fMemoAppInfo.category.name[i] << endl;
	}
#endif

	populateCategories(fCatList, &fMemoAppInfo.category);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Populated categories" << endl;
#endif

	initializeMemos(memoDB);

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Finished initializing" << endl;
#endif

	delete memoDB;

	updateWidget();
}

void MemoWidget::postHotSync()
{
	FUNCTIONSETUP;
	fMemoList.clear();
	initialize();
}


// void MemoWidget::setupWidget()
//
// Setup all the GUI components by allocating them. 
//
//
void MemoWidget::setupWidget()
{
	FUNCTIONSETUP;

	QLabel *label = NULL;
	QPushButton *button = NULL;
	QGridLayout *grid = new QGridLayout(this, 5, 4, SPACING);

	fCatList = new QComboBox(this);
	grid->addWidget(fCatList, 0, 1);
	connect(fCatList, SIGNAL(activated(int)),
		this, SLOT(slotSetCategory(int)));
	QWhatsThis::add(fCatList,
		i18n("Select the category of addresses\n"
			"to display here."));

	label = new QLabel(i18n("Memos:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label, 0, 0);

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox, 1, 1, 0, 1);
	fListBox->setSelectionMode( QListBox::Extended );
	connect(fListBox, SIGNAL(highlighted(int)),
		this, SLOT(slotShowMemo(int)));
	connect(fListBox, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateButtons()));
	QWhatsThis::add(fListBox,
		i18n("This list displays all the memos\n"
			"in the selected category. Click on\n"
			"one to display it to the right."));

	label = new QLabel(i18n("Memo text:"), this);
	grid->addWidget(label, 0, 2);

	fTextWidget = new QMultiLineEdit(this, "textArea");
	fTextWidget->setWordWrap(QMultiLineEdit::WidgetWidth);
	grid->addMultiCellWidget(fTextWidget, 1, 4, 2, 2);
	QWhatsThis::add(fTextWidget,
		i18n("The text of the selected memo appears here."));

	button = new QPushButton(i18n("Import Memo"), this);
	grid->addWidget(button, 2, 0);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportMemo()));
	QWhatsThis::add(button,
		i18n
		("Read a text file and add it to the Pilot's memo database."));

	fExportButton = new QPushButton(i18n("Export Memo"), this);
	grid->addWidget(fExportButton, 2, 1);
	connect(fExportButton, SIGNAL(clicked()), this,
		SLOT(slotExportMemo()));
	QWhatsThis::add(fExportButton,
		i18n("Write the selected memo to a file."));

	fDeleteButton = new QPushButton(i18n("Delete Memo"), this);
	grid->addWidget(fDeleteButton, 3, 0);
	connect(fDeleteButton, SIGNAL(clicked()), this,
		SLOT(slotDeleteMemo()));
	QWhatsThis::add(fDeleteButton, i18n("Delete the selected memo."));
}

void MemoWidget::slotUpdateButtons()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Selected item " << item << endl;
#endif

	if (fExportButton)
	{
		fExportButton->setEnabled(item != -1);
	}
	if (fDeleteButton)
	{
		fDeleteButton->setEnabled(item != -1);
	}
}

void MemoWidget::slotSetCategory(int)
{
	FUNCTIONSETUP;
	updateWidget();
}

void MemoWidget::slotDeleteMemo()
{
	FUNCTIONSETUP;

	int item = fListBox->currentItem();

	if (item == -1)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": No current item selected\n";
#endif
		return;
	}

	PilotListItem *p = (PilotListItem *) fListBox->item(item);
	PilotMemo *selectedMemo = (PilotMemo *) p->rec();

	if (selectedMemo->id() == 0x0)
	{
		// QADE: Why is this? What prevents us from deleting
		// a "new" memo, ie. one we've imported, before *ever*
		// sending it to the Pilot?
		//
		//
		kdWarning() << k_funcinfo <<
			": Refusing to delete new memo.\n";

		KMessageBox::error(this,
			i18n("Cannot delete new memo until "
				"Hot-Synced with pilot."),
			i18n("Hot-Sync Required"));
		return;
	}


	if (KMessageBox::questionYesNo(this,
			i18n("Delete currently selected memo?"),
			i18n("Delete Memo?")) == KMessageBox::No)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname <<
			": User decided not to delete memo.\n";
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


void MemoWidget::updateWidget()
{
	FUNCTIONSETUP;

	if (fCatList->currentItem() == -1)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": No category selected.\n";
#endif
		return;
	}

	int listIndex = 0;
	int currentCatID = findSelectedCategory(fCatList,
		&(fMemoAppInfo.category), false);


	fListBox->clear();
	fMemoList.first();


	// Iterate through all the memos and insert each memo
	// only if the category of the memo matches the selected category
	// (using -1 to mean "All")
	//
	//
	while (fMemoList.current())
	{
		if ((fMemoList.current()->getCat() == currentCatID) ||
			(currentCatID == -1))
		{
			PilotListItem *p =
				new PilotListItem(fMemoList.current()->
				shortTitle(),
				listIndex,
				fMemoList.current());

			// List will delete the title of the memo,
			// so there's no memory leak here.
			//
			//
			fListBox->insertItem(p);

#ifdef DEBUG
			DEBUGKPILOT << fname << ": Added memo "
				<< fMemoList.current()->getTitle() << endl;
#endif
		}
		else
		{
#ifdef DEBUG
			DEBUGKPILOT << fname << ": Skipped memo "
				<< fMemoList.current()->getTitle() << endl;
#endif
		}

		listIndex++;
		fMemoList.next();
	}

	fTextWidget->clear();

	slotUpdateButtons();
}

void MemoWidget::slotShowMemo(int which)
{
	FUNCTIONSETUP;

	disconnect(fTextWidget, SIGNAL(textChanged()),
		this, SLOT(slotTextChanged()));
	fTextWidget->deselect();
	PilotListItem *p = (PilotListItem *) fListBox->item(which);
	PilotMemo *theMemo = (PilotMemo *) p->rec();

	fTextWidget->setText(theMemo->text());
	connect(fTextWidget, SIGNAL(textChanged()),
		this, SLOT(slotTextChanged()));
}

void MemoWidget::writeMemo(PilotMemo * which)
{
	FUNCTIONSETUP;

	PilotDatabase *memoDB = new PilotLocalDatabase(dbPath(), "MemoDB");
	PilotRecord *pilotRec = which->pack();

	memoDB->writeRecord(pilotRec);
	delete pilotRec;
	delete memoDB;
}

void MemoWidget::slotTextChanged()
{
	FUNCTIONSETUP;

	if (fListBox->currentItem() == -1)
	{
		kdWarning() << k_funcinfo
			<< ": slotTextChanged with no memo selected!" << endl;
		return;
	}

	PilotListItem *p =
		(PilotListItem *) fListBox->item(fListBox->currentItem());
	PilotMemo *currentMemo = (PilotMemo *) p->rec();

	if (fListBox->currentItem() >= 0)
	{
		if (currentMemo->id() == 0x0)
		{
			KMessageBox::error(0L,
				i18n
				("Cannot edit new memo until \n Hot-Synced with pilot."),
				i18n("Hot-Sync Required"));
			slotShowMemo(fListBox->currentItem());
			return;
		}
		currentMemo->setText(fTextWidget->text().latin1());
		writeMemo(currentMemo);
	}
}

void MemoWidget::slotImportMemo()
{
	FUNCTIONSETUP;

	int i = 0;
	int nextChar;
	int currentCatID = findSelectedCategory(fCatList,
		&(fMemoAppInfo.category), true);

	QString fileName = KFileDialog::getOpenFileName();

	if (fileName != NULL)
	{
		QFile importFile(fileName);

		if (importFile.open(IO_ReadOnly) == FALSE)
		{
			// show error!
			return;
		}
		char *text = new char[(int) MemoWidget::MAX_MEMO_LEN];

		for (i = 0;
			(i < (MemoWidget::MAX_MEMO_LEN - 1))
			&& ((nextChar = importFile.getch()) != -1); i++)
			text[i] = nextChar;
		text[i] = 0;
		PilotMemo *aMemo = new PilotMemo(text, 0, 0, currentCatID);

		fMemoList.append(aMemo);
		writeMemo(aMemo);
		updateWidget();
		delete[]text;
	}
}

void MemoWidget::slotExportMemo()
{
	FUNCTIONSETUP;
	
	int index = fListBox->numRows();
	if (index == 0)
		return;

	QString data;

	const QString filter = "*|Plain text output\n*.xml|XML output";
	QString fileName;
	
	KFileDialog kfile( QString::null , filter, fExportButton , "memoSave" , true );
	kfile.setOperationMode( KFileDialog::Saving );
	
	if ( kfile.exec() == QDialog::Accepted ) {
		fileName = kfile.selectedFile();
	}
	
	if (fileName.isEmpty())
		return;

	QList<PilotListItem> menu_items;

	for (int x = 0; x < index; x++){
		if (fListBox->item(x)->selected()){
			menu_items.append((PilotListItem *) fListBox->item(x));
		}
	}

	if (kfile.currentFilter() == "*.xml" )
	{
		MemoWidget::saveAsXML( fileName , menu_items );
	}
	else
	{
		MemoWidget::saveAsText( fileName , menu_items );
	}


	return;
}

bool MemoWidget::saveAsText(const QString &fileName,const QList<PilotListItem> &memo_list)
{
	QFile f( fileName );
	QTextStream stream(&f);
		
	if ( QFile::exists( fileName ) )
	{
		if( !f.open(IO_ReadWrite | IO_Append) )
		{
			return false;
		}
	}
	else
	{
		if( !f.open(IO_WriteOnly) )
		{
			return false;
		}
	}

	QListIterator<PilotListItem> it(memo_list);
	for ( ; it.current(); ++it )
	{
		PilotListItem *p = it.current();
		PilotMemo *theMemo = (PilotMemo *) p->rec();
		stream << theMemo->text() << endl;
	}

	
	return true;
}	

bool MemoWidget::saveAsXML(const QString &fileName,const QList<PilotListItem> &memo_list)
{
	QDomDocument doc( "kpilotmemos" );
	QFile f( fileName );
	QTextStream stream( &f );
	QDomElement memos;
	int append = 0;
	

	if ( f.exists() )
	{
		if ( !f.open(IO_ReadOnly ) ) return false;

		if ( doc.setContent( &f ) )
		{
		//
		//
		//Only if QDom can read the .xml file and set the doc object to be populated with it's contents
			memos = doc.documentElement();
			if ( memos.tagName()!="memos" ) 
			{ 
				return false; 
			} 
				//
				//			
				//This is an XML Document but it isn't a valid KPilot-Memo xml document
			else 
			{
				append = 1; 
			}
				//
				//
				//This is a valid KPilot memo, and we will append the current memo to the xml
		}
		else
		{
		//
		//
		//We *couldn't* understand the xml.  Return false!
			return false;
		}
	}
	else
	{
		if ( !f.open(IO_ReadWrite ) ) return false;
		//
		//
		//If there's no such file, we are not appending, just opening the file to read/write.
	}

	f.close();
    // These are temporary, and should be retrieved from the pilot stuff
    QString mpilotid;
    mpilotid = "1";
    //  End of temp variables

	if (append == 1)
	{
		memos = doc.documentElement();
	}
	else
	{
		memos = doc.createElement( "memos" );
		doc.appendChild ( memos );
	}

	QListIterator<PilotListItem> it(memo_list);
	for ( ; it.current(); ++it )
	{
		PilotListItem *p = it.current();
		PilotMemo *theMemo = (PilotMemo *) p->rec();
	

    	QDomElement memo = doc.createElement( "memo" );
	    memo.setAttribute ( "pilotid" , mpilotid );
	    memos.appendChild ( memo );

	    //QDomElement category = doc.createElement( "category" );
	    //head.appendChild ( category );
		//
	 	//QDomText categorytext = doc.createTextNode( memo->category() );
		//category.appendChild ( categorytext );
		//FIXME

		QDomElement title = doc.createElement( "title" );
		memo.appendChild ( title );

		QDomText titletext = doc.createTextNode( theMemo->shortTitle() );
		title.appendChild ( titletext );

		QDomText body = doc.createTextNode( theMemo->text() );
		memo.appendChild ( body );
	}
	if ( !f.open(IO_WriteOnly ) ) return false;
	stream << doc.toString();
	return true;
}

// $Log$
// Revision 1.46  2002/05/15 17:15:33  gioele
// kapp.h -> kapplication.h
// I have removed KDE_VERSION checks because all that files included "options.h"
// which #includes <kapplication.h> (which is present also in KDE_2).
// BTW you can't have KDE_VERSION defined if you do not include
// - <kapplication.h>: KDE3 + KDE2 compatible
// - <kdeversion.h>: KDE3 only compatible
//
// Revision 1.45  2002/04/16 18:14:18  adridg
// David Bishop's XML export patches
//
// Revision 1.44  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.43  2002/01/20 06:46:23  waba
// Messagebox changes.
//
// Revision 1.42  2001/10/19 14:03:04  adridg
// Qt3 include fixes
//
// Revision 1.41  2001/10/10 22:22:39  adridg
// Removed really weird debugging
//
// Revision 1.40  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.39  2001/09/30 16:59:22  adridg
// Cleaned up preHotSync
//
// Revision 1.38  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.37  2001/09/23 18:30:15  adridg
// Adjusted widget for new config
//
// Revision 1.36  2001/09/06 22:33:43  adridg
// Cruft cleanup
//
// Revision 1.35  2001/08/19 19:12:55  adridg
// Fixed up some kdWarnings that were generated because connect() was called too soon
//
// Revision 1.34  2001/06/13 22:51:38  cschumac
// Minor fixes reviewed on the mailing list.
//
// Revision 1.33  2001/06/13 21:26:54  adridg
// Add cast to avoid comile warning
//
// Revision 1.32  2001/06/11 07:35:19  adridg
// Cleanup before the freeze
//
// Revision 1.31  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.30  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.29  2001/04/14 15:21:35  adridg
// XML GUI and ToolTips
//
// Revision 1.28  2001/04/01 17:32:52  adridg
// I really don't remember
//
// Revision 1.27  2001/03/24 15:59:22  adridg
// Some populateCategories changes for bug #22112
//
// Revision 1.26  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.25  2001/03/04 13:11:49  adridg
// More response to bug 21392
//
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
