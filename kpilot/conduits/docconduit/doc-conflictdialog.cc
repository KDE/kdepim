// doc-conflictdialog.cc
//
// Copyright (C) 2003 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$
//

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "doc-conflictdialog.moc"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <kmessagebox.h>
#include <qtimer.h>
#include <qtable.h>



ResolutionDialog::ResolutionDialog( QWidget* parent, const QString& caption, syncInfoList*sinfo, KPilotDeviceLink*lnk )
    : KDialogBase( parent, "resolutionDialog", true, caption, KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), tickleTimer(0L), fHandle(lnk) {
	FUNCTIONSETUP;
	syncInfo=sinfo;
	hasConflicts=false;
	
	QWidget *page = new QWidget( this );
	setMainWidget(page);
	QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
	
	// First, insert the texts on top:
	textLabel1 = new QLabel(i18n("Here is a list of all text files and DOC databases the conduit found. The conduit tried to determine the correct sync direction, but for databases in bold red letters a conflict occurred (i.e. the text was changed both on the desktop and on the handheld). For these databases please specify which version is the current one."), page);
	textLabel1->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );
	topLayout->addWidget(textLabel1);

	textLabel2 = new QLabel(i18n("You can also change the sync direction for databases without a conflict." ),  page );
	textLabel2->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );
	topLayout->addWidget(textLabel2);
	
#ifdef SCROLL_TABLE
	if (syncInfo) {
		resolutionTable = new QTable(syncInfo->size(), 3, page);
		resolutionTable->setSelectionMode(QTable::NoSelection);
		resolutionTable->setRowMovingEnabled(false);
		resolutionTable->setColumnMovingEnabled(false);
		resolutionTable->setDragEnabled(false);
		resolutionTable->setColumnReadOnly(0, true);
		for (int i=0; i<3; i++) {
			resolutionTable->adjustColumn(i);
//			resolutionTable->setColumnStretchable(i, true);
		}
		QStringList headers;
		headers<<i18n("Database name")<<i18n("Resolution")<<"";
		resolutionTable->setColumnLabels(headers);
		resolutionTable->setSorting(false);
		
		
		QButtonGroup *bgroup = new QButtonGroup(page);
		bgroup->hide();
		QObject::connect(bgroup, SIGNAL(clicked(int)), this, SLOT(slotInfo(int)));
		
		
		DEBUGCONDUIT<<"Adding resolution options for the databases "<<endl;
		syncInfoList::Iterator it;
		int nr=0;
		DEBUGCONDUIT<<"We're having "<<(*syncInfo).size()<<" entries in the database list"<<endl;
		for (it=syncInfo->begin(); it!=syncInfo->end(); it++) {
			docSyncInfo si=(*it);
			conflictEntry cE;
			cE.index=nr;
			cE.conflict=(si.direction==eSyncConflict);
			DEBUGCONDUIT<<"Adding "<<si.handheldDB<<" to the conflict resolution dialog"<<endl;
			
			QString text=si.handheldDB;
			if  (cE.conflict) {
				text="<qt><b><font color=red>"+text+"</font></b></qt>";
				DEBUGCONDUIT<<"We have a conflict for database "<<si.handheldDB<<endl;
				hasConflicts=true;
			}
			cE.dbname=text;
			resolutionTable->setText(cE.index, 0, text);
			
			QStringList entries;
			entries<< i18n( "No Sync" ) << i18n( "Sync handheld to PC" ) << i18n( "Sync PC to handheld" ) << i18n( "Delete both databases" );
			cE.resolution=new QComboTableItem(resolutionTable, entries);
			cE.resolution->setCurrentItem((int)si.direction);
			resolutionTable->setItem( cE.index, 1, cE.resolution);
			
			cE.info = new QPushButton( i18n("More information..."), resolutionTable );
			resolutionTable->setCellWidget(cE.index, 2, cE.info);
			bgroup->insert(cE.info);
			
			conflictEntries.append(cE);
			nr++;
		}

	} else {
		kdWarning()<<"The list of text files is not available to the resolution dialog. Something must have gone wrong ..."<<endl;
		new QLabel(i18n("<qt><b>Error</b>: Could not get list of DOC databases!</qt>"), page );
	}
	topLayout->addWidget(resolutionTable);

#else
	
	resolutionGroupBox = new QGroupBox(i18n("DOC databases"), page );
	QScrollView* sv = new QScrollView(resolutionGroupBox);
//	QVBox* big_box = new QVBox(sv->viewport());
//        sv->addChild(big_box);
    
// You can go on to add arbitrary child widgets to the single child in the scrollview as you would with any widget: 
//        QLabel* child1 = new QLabel("CHILD", big_box);
//        QLabel* child2 = new QLabel("CHILD", big_box);
//        QLabel* child3 = new QLabel	resolutionGroupBox->setColumnLayout(0, Qt::Vertical );
	
	resolutionGroupBoxLayout = new QGridLayout( sv->viewport() );
	resolutionGroupBoxLayout->setAlignment( Qt::AlignTop );
	
	// Invisible button group for the information buttons to use the same slot for all of them (see Dallheimer's book, page 309f)
	QButtonGroup *bgroup = new QButtonGroup(page);
	bgroup->hide();
	QObject::connect(bgroup, SIGNAL(clicked(int)), this, SLOT(slotInfo(int)));

	if (syncInfo) {
		DEBUGCONDUIT<<"Adding resolution options for the databases "<<endl;
		syncInfoList::Iterator it;
		int nr=0;
		DEBUGCONDUIT<<"We're having "<<(*syncInfo).size()<<" entries in the database list"<<endl;
		for (it=syncInfo->begin(); it!=syncInfo->end(); it++) {
			docSyncInfo si=(*it);
			conflictEntry cE;
			cE.index=nr;
			cE.conflict=(si.direction==eSyncConflict);
			DEBUGCONDUIT<<"Adding "<<si.handheldDB<<" to the conflict resolution dialog"<<endl;
			
			QString text=si.handheldDB;
			if  (cE.conflict) {
				text="<qt><b><font color=red>"+text+"</font></b></qt>";
				DEBUGCONDUIT<<"We have a conflict for database "<<si.handheldDB<<endl;
				hasConflicts=true;
			}
			cE.dbname=new QLabel(text, sv->viewport());
			resolutionGroupBoxLayout->addWidget( cE.dbname, cE.index, 0 );
			
			cE.resolution=new QComboBox( FALSE, sv->viewport());
			cE.resolution->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, cE.resolution->sizePolicy().hasHeightForWidth() ) );
			cE.resolution->clear();
			cE.resolution->insertItem( i18n( "No Sync" ) );
			cE.resolution->insertItem( i18n( "Sync handheld to PC" ) );
			cE.resolution->insertItem( i18n( "Sync PC to handheld" ) );
			cE.resolution->insertItem( i18n( "Delete both databases" ) );
			cE.resolution->setCurrentItem((int)si.direction);
			resolutionGroupBoxLayout->addWidget( cE.resolution, cE.index, 1);
			
			cE.info = new QPushButton( i18n("More information..."), sv->viewport() );
			resolutionGroupBoxLayout->addWidget(cE.info, cE.index, 2);
			bgroup->insert(cE.info);
			
			conflictEntries.append(cE);
			nr++;
		}
	} else {
		kdWarning()<<"The list of text files is not available to the resolution dialog. Something must have gone wrong ..."<<endl;
	}


	topLayout->addWidget( resolutionGroupBox );
	
#endif
	
/*	topLayout->addStretch(10);*/


	if (fHandle) tickleTimer=new QTimer(this, "TickleTimer");
	if (tickleTimer) {
		connect( tickleTimer, SIGNAL(timeout()), this, SLOT(_tickle()) );
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
	QValueList<conflictEntry>::Iterator ceIt;
	for (ceIt=conflictEntries.begin(); ceIt!=conflictEntries.end(); ceIt++) {
		(*syncInfo)[(*ceIt).index].direction=(eSyncDirectionEnum)((*ceIt).resolution->currentItem());
	}
	KDialogBase::slotOk();
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
	QString text=i18n("Status of the database %1:\n\n").arg(si.handheldDB);
	text+=i18n("Handheld: %1\n").arg(eTextStatusToString(si.fPalmStatus));
	text+=i18n("Desktop: %1\n").arg(eTextStatusToString(si.fPCStatus));
	
	KMessageBox::information(this, text, i18n("Database information"));
}


void ResolutionDialog::_tickle() {
	FUNCTIONSETUP;
	if (fHandle) fHandle->tickle();
}
