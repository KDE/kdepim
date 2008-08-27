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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <q3buttongroup.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3ValueList>
#include <Q3VBoxLayout>
#include <kmessagebox.h>
#include <qtimer.h>
#include <q3table.h>
#include <q3scrollview.h>

#include <KComboBox>


ResolutionDialog::ResolutionDialog( QWidget* parent, const QString& caption, syncInfoList*sinfo, KPilotLink*lnk )
    : KDialog( parent)
	, tickleTimer(0L), fHandle(lnk) {
	setModal(true);
	setButtons(Ok|Cancel);
	setDefaultButton(Ok);
	setCaption(caption);
	FUNCTIONSETUP;
	syncInfo=sinfo;
	hasConflicts=false;

	QWidget *page = new QWidget( this );
	setMainWidget(page);
	Q3VBoxLayout *topLayout = new Q3VBoxLayout( page, 0, spacingHint() );

	// First, insert the texts on top:
	textLabel1 = new QLabel(i18n("Here is a list of all text files and DOC databases the conduit found. The conduit tried to determine the correct sync direction, but for databases in bold red letters a conflict occurred (i.e. the text was changed both on the desktop and on the handheld). For these databases please specify which version is the current one."), page);
	textLabel1->setAlignment( int( Qt::TextWordWrap | Qt::AlignVCenter ) );
	topLayout->addWidget(textLabel1);

	textLabel2 = new QLabel(i18n("You can also change the sync direction for databases without a conflict." ),  page );
	textLabel2->setAlignment( int( Qt::TextWordWrap | Qt::AlignVCenter ) );
	topLayout->addWidget(textLabel2);

	resolutionGroupBox = new Q3GroupBox(i18n("DOC Databases"), page );
	Q3VBoxLayout*playout = new Q3VBoxLayout(resolutionGroupBox);
	Q3ScrollView* sv = new Q3ScrollView(resolutionGroupBox);
	playout->addWidget(sv);
	sv->setResizePolicy(Q3ScrollView::AutoOneFit);
	sv->setHScrollBarMode(Q3ScrollView::AlwaysOff);
	sv->setMargin(5);
	Q3Frame* big_box = new Q3Frame(sv->viewport());
	sv->addChild(big_box);


	resolutionGroupBoxLayout = new Q3GridLayout( big_box, syncInfo->size(), 3 );
	resolutionGroupBoxLayout->setAlignment( Qt::AlignTop );

	// Invisible button group for the information buttons to use the same slot for all of them (see Dallheimer's book, page 309f)
	Q3ButtonGroup *bgroup = new Q3ButtonGroup( this );
	bgroup->hide();
	QObject::connect(bgroup, SIGNAL(clicked(int)), this, SLOT(slotInfo(int)));

	if (syncInfo) {
		DEBUGKPILOT<<"Adding resolution options for the databases";
		syncInfoList::Iterator it;
		int nr=0;
		DEBUGKPILOT<<"We have "<<(*syncInfo).size()<<" entries in the database list";
		for (it=syncInfo->begin(); it!=syncInfo->end(); ++it ) {
			docSyncInfo si=(*it);
			conflictEntry cE;
			cE.index=nr;
			cE.conflict=(si.direction==eSyncConflict);
			DEBUGKPILOT<<"Adding"<<si.handheldDB<<" to the conflict resolution dialog";

			QString text=si.handheldDB;
			if  (cE.conflict) {
				text=CSL1("<qt><b><font color=red>")+text+CSL1("</font></b></qt>");
				DEBUGKPILOT<<"We have a conflict for database"<<si.handheldDB;
				hasConflicts=true;
			}
			cE.dbname=new QLabel(text, big_box);
			resolutionGroupBoxLayout->addWidget( cE.dbname, cE.index, 0 );

			cE.resolution=new KComboBox( false, big_box);
			cE.resolution->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7,
				(QSizePolicy::SizeType)0, 0, 0,
				cE.resolution->sizePolicy().hasHeightForWidth() ) );
			cE.resolution->clear();
			cE.resolution->insertItem( i18n( "No Sync" ) );
			cE.resolution->insertItem( i18n( "Sync Handheld to PC" ) );
			cE.resolution->insertItem( i18n( "Sync PC to Handheld" ) );
			cE.resolution->insertItem( i18n( "Delete Both Databases" ) );
			cE.resolution->setCurrentItem((int)si.direction);
			resolutionGroupBoxLayout->addWidget( cE.resolution, cE.index, 1);

			cE.info = new QPushButton( i18n("More Info..."), big_box );
			resolutionGroupBoxLayout->addWidget(cE.info, cE.index, 2);
			bgroup->insert(cE.info);

			conflictEntries.append(cE);
			++nr;
		}
	} else {
		WARNINGKPILOT <<"The list of text files is not available to the resolution dialog.";
	}


	topLayout->addWidget( resolutionGroupBox );
	resize( QSize(600, 480).expandedTo(minimumSizeHint()) );

	if (fHandle) tickleTimer=new QTimer(this, "TickleTimer");
	if (tickleTimer) {
		connect( tickleTimer, SIGNAL(timeout()), this, SLOT(_tickle()) );
		tickleTimer->start( 10000 ); // tickle the palm every 10 seconds to prevent a timeout until the sync is really finished.
	}
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
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
	Q3ValueList<conflictEntry>::Iterator ceIt;
	for (ceIt=conflictEntries.begin(); ceIt!=conflictEntries.end(); ++ceIt) {
		(*syncInfo)[(*ceIt).index].direction=(eSyncDirectionEnum)((*ceIt).resolution->currentItem());
	}
	accept();
}

QString eTextStatusToString(eTextStatus stat) {
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
	QString text=i18n("Status of the database %1:\n\n",si.handheldDB);
	text+=i18n("Handheld: %1\n",eTextStatusToString(si.fPalmStatus));
	text+=i18n("Desktop: %1\n",eTextStatusToString(si.fPCStatus));

	KMessageBox::information(this, text, i18n("Database information"));
}


void ResolutionDialog::_tickle() {
	FUNCTIONSETUP;
	if (fHandle) fHandle->tickle();
}
