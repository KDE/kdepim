/* KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "doc-conflictdialog.moc"

#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqbuttongroup.h>
#include <kmessagebox.h>
#include <tqtimer.h>
#include <tqtable.h>
#include <tqcombobox.h>
#include <tqscrollview.h>


ResolutionDialog::ResolutionDialog( TQWidget* parent, const TQString& caption, syncInfoList*sinfo, KPilotLink*lnk )
    : KDialogBase( parent, "resolutionDialog", true, caption, KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), tickleTimer(0L), fHandle(lnk) {
	FUNCTIONSETUP;
	syncInfo=sinfo;
	hasConflicts=false;

	TQWidget *page = new TQWidget( this );
	setMainWidget(page);
	TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );

	// First, insert the texts on top:
	textLabel1 = new TQLabel(i18n("Here is a list of all text files and DOC databases the conduit found. The conduit tried to determine the correct sync direction, but for databases in bold red letters a conflict occurred (i.e. the text was changed both on the desktop and on the handheld). For these databases please specify which version is the current one."), page);
	textLabel1->setAlignment( int( TQLabel::WordBreak | TQLabel::AlignVCenter ) );
	topLayout->addWidget(textLabel1);

	textLabel2 = new TQLabel(i18n("You can also change the sync direction for databases without a conflict." ),  page );
	textLabel2->setAlignment( int( TQLabel::WordBreak | TQLabel::AlignVCenter ) );
	topLayout->addWidget(textLabel2);

	resolutionGroupBox = new TQGroupBox(i18n("DOC Databases"), page );
	TQVBoxLayout*playout = new TQVBoxLayout(resolutionGroupBox);
	TQScrollView* sv = new TQScrollView(resolutionGroupBox);
	playout->addWidget(sv);
	sv->setResizePolicy(TQScrollView::AutoOneFit);
	sv->setHScrollBarMode(TQScrollView::AlwaysOff);
	sv->setMargin(5);
	TQFrame* big_box = new TQFrame(sv->viewport());
	sv->addChild(big_box);


	resolutionGroupBoxLayout = new TQGridLayout( big_box, syncInfo->size(), 3 );
	resolutionGroupBoxLayout->setAlignment( Qt::AlignTop );

	// Invisible button group for the information buttons to use the same slot for all of them (see Dallheimer's book, page 309f)
	TQButtonGroup *bgroup = new TQButtonGroup( this );
	bgroup->hide();
	TQObject::connect(bgroup, TQT_SIGNAL(clicked(int)), this, TQT_SLOT(slotInfo(int)));

	if (syncInfo) {
		DEBUGKPILOT<<"Adding resolution options for the databases "<<endl;
		syncInfoList::Iterator it;
		int nr=0;
		DEBUGKPILOT<<"We're having "<<(*syncInfo).size()<<" entries in the database list"<<endl;
		for (it=syncInfo->begin(); it!=syncInfo->end(); ++it ) {
			docSyncInfo si=(*it);
			conflictEntry cE;
			cE.index=nr;
			cE.conflict=(si.direction==eSyncConflict);
			DEBUGKPILOT<<"Adding "<<si.handheldDB<<" to the conflict resolution dialog"<<endl;

			TQString text=si.handheldDB;
			if  (cE.conflict) {
				text=CSL1("<qt><b><font color=red>")+text+CSL1("</font></b></qt>");
				DEBUGKPILOT<<"We have a conflict for database "<<si.handheldDB<<endl;
				hasConflicts=true;
			}
			cE.dbname=new TQLabel(text, big_box);
			resolutionGroupBoxLayout->addWidget( cE.dbname, cE.index, 0 );

			cE.resolution=new TQComboBox( FALSE, big_box);
			cE.resolution->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)7,
				(TQSizePolicy::SizeType)0, 0, 0,
				cE.resolution->sizePolicy().hasHeightForWidth() ) );
			cE.resolution->clear();
			cE.resolution->insertItem( i18n( "No Sync" ) );
			cE.resolution->insertItem( i18n( "Sync Handheld to PC" ) );
			cE.resolution->insertItem( i18n( "Sync PC to Handheld" ) );
			cE.resolution->insertItem( i18n( "Delete Both Databases" ) );
			cE.resolution->setCurrentItem((int)si.direction);
			resolutionGroupBoxLayout->addWidget( cE.resolution, cE.index, 1);

			cE.info = new TQPushButton( i18n("More Info..."), big_box );
			resolutionGroupBoxLayout->addWidget(cE.info, cE.index, 2);
			bgroup->insert(cE.info);

			conflictEntries.append(cE);
			++nr;
		}
	} else {
		WARNINGKPILOT << "The list of text files is not available to the resolution dialog." << endl;
	}


	topLayout->addWidget( resolutionGroupBox );
	resize( TQSize(600, 480).expandedTo(minimumSizeHint()) );

	if (fHandle) tickleTimer=new TQTimer(this, "TickleTimer");
	if (tickleTimer) {
		connect( tickleTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(_tickle()) );
		tickleTimer->start( 10000 ); // tickle the palm every 10 seconds to prevent a timeout until the sync is really finished.
	}

}

/*
 *  Destroys the object and frees any allocated resources
 */
ResolutionDialog::~ResolutionDialog()
{
	// no need to delete child widgets, Qt does it all for us
}

/* virtual slot */ void ResolutionDialog::slotOk() {
	FUNCTIONSETUP;
	TQValueList<conflictEntry>::Iterator ceIt;
	for (ceIt=conflictEntries.begin(); ceIt!=conflictEntries.end(); ++ceIt) {
		(*syncInfo)[(*ceIt).index].direction=(eSyncDirectionEnum)((*ceIt).resolution->currentItem());
	}
	KDialogBase::slotOk();
}

TQString eTextStatusToString(eTextStatus stat) {
	switch(stat) {
		case eStatNone: return i18n("unchanged");
		case eStatNew: return i18n("new");
		case eStatChanged: return i18n("changed");
		case eStatBookmarksChanged: return i18n("only bookmarks changed");
		case eStatDeleted: return i18n("deleted");
		case eStatDoesntExist: return i18n("does not exist");
		default: return i18n("unknown");
	}
}

void ResolutionDialog::slotInfo(int index) {
	FUNCTIONSETUP;
	conflictEntry cE=conflictEntries[index];
	int ix=cE.index;
	if (!syncInfo) return;
	docSyncInfo si=(*syncInfo)[ix];
	TQString text=i18n("Status of the database %1:\n\n").arg(si.handheldDB);
	text+=i18n("Handheld: %1\n").arg(eTextStatusToString(si.fPalmStatus));
	text+=i18n("Desktop: %1\n").arg(eTextStatusToString(si.fPCStatus));

	KMessageBox::information(this, text, i18n("Database information"));
}


void ResolutionDialog::_tickle() {
	FUNCTIONSETUP;
	if (fHandle) fHandle->tickle();
}
