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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *memowidget_id =
	"$Id$";

#include "options.h"

#include <time.h>

#include <pi-macros.h>
#include <pi-dlp.h>

#include <qdir.h>
#include <qptrlist.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qwhatsthis.h>
#include <qlabel.h>
#include <qtextcodec.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "kpilot.h"
#include "kpilotConfig.h"
#include "listItems.h"
#include "pilotLocalDatabase.h"
#include "pilotMemo.h"

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
	fTextWidget(0L),
	lastSelectedMemo(-1)
{
	FUNCTIONSETUP;

	setGeometry(0, 0,
		parent->geometry().width(), parent->geometry().height());
	setupWidget();
	fMemoList.setAutoDelete(true);
	slotUpdateButtons();

#if 0
	connect(fTextWidget, SIGNAL(textChanged()),
		this, SLOT(slotTextChanged()));
#endif

	/* NOTREACHED */
	(void) memowidget_id;
}

MemoWidget::~MemoWidget()
{
	FUNCTIONSETUP;
	saveChangedMemo();
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
		new PilotLocalDatabase(dbPath(), CSL1("MemoDB"));
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

	(void) i18n("Memos:");
	label = new QLabel(i18n("Category:"), this);
	label->setBuddy(fCatList);
	grid->addWidget(label, 0, 0);

	fListBox = new QListBox(this);
	grid->addMultiCellWidget(fListBox, 1, 1, 0, 1);
	connect(fListBox, SIGNAL(highlighted(int)),
		this, SLOT(slotShowMemo(int)));
	connect(fListBox, SIGNAL(selectionChanged()),
		this,SLOT(slotUpdateButtons()));
	QWhatsThis::add(fListBox,
		i18n("This list displays all the memos\n"
			"in the selected category. Click on\n"
			"one to display it to the right."));

	label = new QLabel(i18n("Memo text:"), this);
	grid->addWidget(label, 0, 2);

	fTextWidget = new KTextEdit(this, "textArea");
	fTextWidget->setWordWrap(KTextEdit::WidgetWidth);
	fTextWidget->setTextFormat(Qt::PlainText);
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

	bool highlight = false;

	if ((fListBox->currentItem() != -1) &&
		(fListBox->isSelected(fListBox->currentItem())))
			highlight=true;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Selected items " << highlight << endl;
#endif

	if (fExportButton)
	{
		fExportButton->setEnabled(highlight);
	}
	if (fDeleteButton)
	{
		fDeleteButton->setEnabled(highlight);
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
			i18n("New memo cannot be deleted until "
				"HotSynced with pilot."),
			i18n("HotSync Required"));
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

	selectedMemo->makeDeleted();
	writeMemo(selectedMemo);
	fMemoList.remove(selectedMemo);
	delete p;
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

	lastSelectedMemo=-1;
}

void MemoWidget::slotShowMemo(int which)
{
	FUNCTIONSETUP;
	if ( which == -1 )
		return;

	slotUpdateButtons();
	if ( !fListBox->isSelected(which) )
	{
		// Handle unselecting a memo. This is easy.
		fTextWidget->blockSignals(true);
		fTextWidget->clear();
		fTextWidget->blockSignals(false);
		return;
	}


#ifdef DEBUG
	DEBUGKPILOT << fname << ": Displaying memo " << which << endl;
#endif
	fTextWidget->blockSignals(true);
	PilotListItem *p = (PilotListItem *) fListBox->item(which);
	PilotMemo *theMemo = (PilotMemo *) p->rec();
	fTextWidget->setText(theMemo->text());
	fTextWidget->blockSignals(false);
}

void MemoWidget::writeMemo(PilotMemo * which)
{
	FUNCTIONSETUP;

	PilotDatabase *memoDB = new PilotLocalDatabase(dbPath(), CSL1("MemoDB"));
	PilotRecord *pilotRec = which->pack();

	memoDB->writeRecord(pilotRec);
	markDBDirty("MemoDB");
	delete pilotRec;
	delete memoDB;
}

void MemoWidget::saveChangedMemo()
{
	FUNCTIONSETUP;

	if (-1 == lastSelectedMemo) return;
	if (!fTextWidget->isModified()) return;

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Saving changed memo " << lastSelectedMemo << endl;
#endif

	PilotListItem *p =
		(PilotListItem *) fListBox->item(lastSelectedMemo);
	PilotMemo *currentMemo = (PilotMemo *) p->rec();

	currentMemo->setText(PilotAppCategory::codec()->
		fromUnicode(fTextWidget->text()));
	writeMemo(currentMemo);
}

/* virtual */ bool MemoWidget::preHotSync(QString &)
{
	FUNCTIONSETUP;
	saveChangedMemo();
	return true;
}

void MemoWidget::slotImportMemo()
{
	FUNCTIONSETUP;

	int i = 0;
	int nextChar;
	int currentCatID = findSelectedCategory(fCatList,
		&(fMemoAppInfo.category), true);

	QString fileName = KFileDialog::getOpenFileName();

	if (!fileName.isEmpty())
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

	const QString filter = CSL1("*|Plain text output\n*.xml|XML output");
	QString fileName;

	KFileDialog kfile( QString::null , filter, fExportButton , "memoSave" , true );
	kfile.setOperationMode( KFileDialog::Saving );

	if ( kfile.exec() == QDialog::Accepted ) {
		fileName = kfile.selectedFile();
	}

	if (fileName.isEmpty())
		return;

	QPtrList<PilotListItem> menu_items;

	for (int x = 0; x < index; x++){
		if (fListBox->item(x)->isSelected()){
			menu_items.append((PilotListItem *) fListBox->item(x));
		}
	}

	if (kfile.currentFilter() == CSL1("*.xml") )
	{
		MemoWidget::saveAsXML( fileName , menu_items );
	}
	else
	{
		MemoWidget::saveAsText( fileName , menu_items );
	}


	return;
}

bool MemoWidget::saveAsText(const QString &fileName,const QPtrList<PilotListItem> &memo_list)
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

	QPtrListIterator<PilotListItem> it(memo_list);
	for ( ; it.current(); ++it )
	{
		PilotListItem *p = it.current();
		PilotMemo *theMemo = (PilotMemo *) p->rec();
		stream << theMemo->text() << endl;
	}


	return true;
}

bool MemoWidget::saveAsXML(const QString &fileName,const QPtrList<PilotListItem> &memo_list)
{
	QDomDocument doc( CSL1("kpilotmemos") );
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
			if ( memos.tagName()!= CSL1("memos") )
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
		memos = doc.createElement( CSL1("memos") );
		doc.appendChild ( memos );
	}

	QPtrListIterator<PilotListItem> it(memo_list);
	for ( ; it.current(); ++it )
	{
		PilotListItem *p = it.current();
		PilotMemo *theMemo = (PilotMemo *) p->rec();


    	QDomElement memo = doc.createElement( CSL1("memo") );
	    memo.setAttribute ( CSL1("pilotid") , mpilotid );
	    memos.appendChild ( memo );

	    //QDomElement category = doc.createElement( "category" );
	    //head.appendChild ( category );
		//
	 	//QDomText categorytext = doc.createTextNode( memo->category() );
		//category.appendChild ( categorytext );
		//FIXME

		QDomElement title = doc.createElement(CSL1("title" ));
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

