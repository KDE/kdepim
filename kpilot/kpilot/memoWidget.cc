/* memoWidget.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2001 by David Bishop (XML stuff)
** Copyright (C) 2004 by Adriaan de Groot
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
#include <qdatetime.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdeversion.h>
#include <ktextedit.h>

#include "kpilot.h"
#include "kpilotConfig.h"
#include "listItems.h"
#include "pilotLocalDatabase.h"
#include "pilotMemo.h"

#include "memoWidget.moc"





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
	bool showSecrets = KPilotSettings::showSecrets();

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


void MemoWidget::showComponent()
{
	FUNCTIONSETUP;
	if (!shown) return;

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

	KPILOT_DELETE( memoDB );

	updateWidget();
}

void MemoWidget::hideComponent()
{
	FUNCTIONSETUP;
	saveChangedMemo();
	fCatList->clear();
	fTextWidget->clear();
	fMemoList.clear();
	fListBox->clear();
	lastSelectedMemo = -1;
}

void MemoWidget::postHotSync()
{
	FUNCTIONSETUP;
	fMemoList.clear();
	showComponent();
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
	QString wt;

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
	fTextWidget->setReadOnly(!KPilotSettings::internalEditors());

	button = new QPushButton(i18n("Import Memo..."), this);
	grid->addWidget(button, 2, 0);
	connect(button, SIGNAL(clicked()), this, SLOT(slotImportMemo()));
	wt = KPilotSettings::internalEditors() ?
		i18n	("Read a text file and add it to the Pilot's memo database.") :
		i18n("<qt><i>Import is disabled by the 'internal editors' setting.</i></qt>");
	QWhatsThis::add(button,wt);

	fExportButton = new QPushButton(i18n("Export Memo..."), this);
	grid->addWidget(fExportButton, 2, 1);
	connect(fExportButton, SIGNAL(clicked()), this,
		SLOT(slotExportMemo()));
	QWhatsThis::add(fExportButton,
		i18n("Write the selected memo to a file."));

	fDeleteButton = new QPushButton(i18n("Delete Memo"), this);
	grid->addWidget(fDeleteButton, 3, 1);
	connect(fDeleteButton, SIGNAL(clicked()), this,
		SLOT(slotDeleteMemo()));
	wt = KPilotSettings::internalEditors() ?
		i18n("Delete the selected memo.") :
		i18n("<qt><i>Deleting is disabled by the 'internal editors' setting.</i></qt>") ;
	QWhatsThis::add(fDeleteButton, wt);

	button = new QPushButton(i18n("Add Memo"), this);
	grid->addWidget(button, 3, 0);
	connect(button, SIGNAL(clicked()), this, SLOT(slotAddMemo()));
	QWhatsThis::add(button,i18n("Add a new memo to the database."));
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

	//  The remaining buttons are relevant only if the
	// internal editors are editable.
	highlight &= KPilotSettings::internalEditors() ;
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
	if (!shown) return;

	int item = fListBox->currentItem();

	if (item == -1)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": No current item selected\n";
#endif
		return;
	}
	if (KMessageBox::questionYesNo(this,
			i18n("Delete currently selected memo?"),
			i18n("Delete Memo?")) != KMessageBox::Yes)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname <<
			": User decided not to delete memo.\n";
#endif
		return;
	}

	PilotListItem *p = (PilotListItem *) fListBox->item(item);
	PilotMemo *selectedMemo = (PilotMemo *) p->rec();

	if (selectedMemo->id() == 0x0)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Searching for record to delete (it's fresh)" << endl;
#endif
		PilotLocalDatabase *memoDB = new PilotLocalDatabase(dbPath(), CSL1("MemoDB"));
		if (!memoDB || (!memoDB->isDBOpen()))
		{
			// Err.. peculiar.
			kdWarning() << k_funcinfo << ": Can't open MemoDB" << endl;
			KMessageBox::sorry(this,
				i18n("Cannot open MemoDB to delete record."),
				i18n("Cannot Delete Memo"));
			return;
		}
		memoDB->resetDBIndex();
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Searching for new record." << endl;
#endif
		const PilotRecord *r = 0L;
		while ((r = memoDB->findNextNewRecord()))
		{
#ifdef DEBUG
			DEBUGKPILOT << fname << ": got record " << (void *) r << endl;
#endif
			PilotMemo m(r);
			if (m.text() == selectedMemo->text())
			{
				DEBUGKPILOT << fname << ": I think I found the memo." << endl;
				(const_cast<PilotRecord *>(r))->makeDeleted();
				break;
			}
		}
		delete memoDB;
	}
	else
	{
		selectedMemo->makeDeleted();
		writeMemo(selectedMemo);
	}
	fMemoList.remove(selectedMemo);
	delete p;
}


void MemoWidget::updateWidget()
{
	FUNCTIONSETUP;
	if (!shown) return;

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
		PilotMemo *curr = fMemoList.current();
		if ((curr->getCat() == currentCatID) ||
			(currentCatID == -1))
		{
			PilotListItem *p =
				new PilotListItem(curr->shortTitle(),
				listIndex,
				curr);

			// List will delete the title of the memo,
			// so there's no memory leak here.
			//
			//
			fListBox->insertItem(p);

#ifdef DEBUG
			DEBUGKPILOT << fname << ": Added memo "
				<< curr->getTitle() << endl;
#endif
		}
		else
		{
#ifdef DEBUG
			DEBUGKPILOT << fname << ": Skipped memo "
				<< curr->getTitle() << endl;
#endif
		}

		listIndex++;
		fMemoList.next();
	}

	fTextWidget->clear();

	slotUpdateButtons();

	lastSelectedMemo=-1;
}

void MemoWidget::showMemo(const PilotMemo *m)
{
	FUNCTIONSETUP;

	int index = fListBox->count();
	for (int x = 0; x < index; x++)
	{
		PilotMemo *p = (PilotMemo *) ((PilotListItem *)fListBox->item(x))->rec();
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Memo @" << (void *) p <<endl;
		DEBUGKPILOT << fname << "       :" << fListBox->item(x)->text() << endl;
#endif
		if (m==p)
		{
			fListBox->setSelected(x,true);
			slotShowMemo(x);
			break;
		}
	}

}

void MemoWidget::slotShowMemo(int which)
{
	FUNCTIONSETUP;
	if ( which == -1 ) return;
	if (!shown) return;

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
	if (!shown) return;
	PilotRecord *pilotRec = which->pack();
	PilotDatabase *memoDB = new PilotLocalDatabase(dbPath(), CSL1("MemoDB"));
	memoDB->writeRecord(pilotRec);
	markDBDirty("MemoDB");
	KPILOT_DELETE( memoDB );
	KPILOT_DELETE( pilotRec );
}

void MemoWidget::saveChangedMemo()
{
	FUNCTIONSETUP;
	if (!shown) return;

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

bool MemoWidget::addMemo(const QString &s, int category)
{
	FUNCTIONSETUP;

	if (s.length() >= MemoWidget::MAX_MEMO_LEN)
	{
		return false;
	}
	if ((category<0) || (category>15)) category=0;

	char *text = new char[s.length() + 2];
	if (s.isEmpty())
	{
		text[0]=0;
	}
	else
	{
		strlcpy(text,PilotAppCategory::codec()->fromUnicode(s),s.length()+2);
	}
	PilotMemo *aMemo = new PilotMemo(text, 0, 0, category);
	fMemoList.append(aMemo);
	writeMemo(aMemo);
	delete[]text;
	updateWidget();
#ifdef DEBUG
	DEBUGKPILOT << fname << ": New memo @" << (void *)aMemo << endl;
#endif
	showMemo(aMemo);
	return true;
}

void MemoWidget::slotAddMemo()
{
	FUNCTIONSETUP;
	int currentCatID = findSelectedCategory(fCatList,
		&(fMemoAppInfo.category), true);
	addMemo(QDateTime::currentDateTime().toString(), currentCatID);
}

void MemoWidget::slotImportMemo()
{
	FUNCTIONSETUP;
	if (!shown) return;

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

		if (importFile.size() > MemoWidget::MAX_MEMO_LEN)
		{
			// Perhaps read first 64k?
			return;
		}

		QTextStream stream(&importFile);
		QString memoText = stream.read();
		addMemo(memoText, currentCatID);
	}
}

void MemoWidget::slotExportMemo()
{
	FUNCTIONSETUP;
	if (!shown) return;

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

