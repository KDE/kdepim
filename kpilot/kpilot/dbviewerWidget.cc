/* dbViewerWidget.cc		KPilot
**
** Copyright (C) 2003 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003 by Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2003,2007 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
**
** This is the generic DB viewer widget.
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

#include "dbviewerWidget.h"

#include "options.h"

#include <pi-dlp.h>
#include <pi-file.h>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QHeaderView>

#include <kmessagebox.h>

#include "pilotDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotRecord.h"
#include "kpilotConfig.h"
#include "dbFlagsEditor.h"
#include "dbAppInfoEditor.h"
#include "dbRecordEditor.h"

#include "ui_viewer_genericdb.h"

class GenericDBWidget::Private
{
public:
	Private() : fWidgetsInitialized( false ) {}

	bool fWidgetsInitialized;
	Ui::GenericDBViewer fWidgetUi;
	QString fCurrentDB;
	eDBType fCurrentDBtype;
};

GenericDBWidget::GenericDBWidget( QWidget *parent, const QString &dbpath )
	: ViewerPageBase( parent, dbpath, QString(), i18n( "Generic DB Viewer" ) )
		, fP( new Private )
{
	FUNCTIONSETUP;
}

GenericDBWidget::~GenericDBWidget()
{
	FUNCTIONSETUP;
	KPILOT_DELETE( fP );
}

void GenericDBWidget::showPage()
{
	FUNCTIONSETUP;
	
	if( !fP->fWidgetsInitialized )
	{
		fP->fWidgetUi.setupUi( this );
		
		// COnfigure the tablewidget.
		fP->fWidgetUi.fRecordList->verticalHeader()->hide();
		fP->fWidgetUi.fRecordList->horizontalHeader()->setStretchLastSection( true );
		fP->fWidgetUi.fRecordList->horizontalHeader()->setSortIndicator ( 0
			, Qt::AscendingOrder );
		
		QVariant data = fP->fWidgetUi.fRecordList->model()->headerData( 0, Qt::Horizontal);
		qDebug() << data;
		
		fP->fWidgetsInitialized = true;
		
		connect( fP->fWidgetUi.fDBList, SIGNAL( itemSelectionChanged() )
			, this, SLOT( slotSelectionChanged() ) );
		connect( fP->fWidgetUi.fDBType, SIGNAL( activated( int ) )
			, this, SLOT( slotDBType( int ) ) );
		connect( fP->fWidgetUi.fDBInfoButton,  SIGNAL( clicked() )
			, this, SLOT( slotShowDBInfo() ) );
		connect( fP->fWidgetUi.fAppInfoButton, SIGNAL( clicked() )
			, this, SLOT( slotShowAppInfo() ) );
		connect( fP->fWidgetUi.fRecordList, SIGNAL( itemSelectionChanged() )
			, this, SLOT( slotShowRecord() ) );
	}
	
	slotDBType( 0 );
}

void GenericDBWidget::hidePage()
{
	FUNCTIONSETUP;
	
	// Clear the ui
	fP->fWidgetUi.fDBList->clear();
	fP->fWidgetUi.fRecordList->clearContents();
	
}

void GenericDBWidget::slotSelectionChanged()
{
	FUNCTIONSETUP;
	
	if( fP->fWidgetUi.fDBList->selectedItems().size() == 0 )
	{
		return;
	}
	
	QListWidgetItem *item = fP->fWidgetUi.fDBList->selectedItems().first();
	QString dbname = item->text();
	
	DEBUGKPILOT << "Selected DB [" << dbname << ']';

	fP->fWidgetUi.fRecordList->clearContents();	
	fP->fCurrentDB = dbname;

	if( !isVisible() )
	{
		return;
	}

	QString display;
	struct DBInfo dbinfo;

	if( fP->fCurrentDB.endsWith( CSL1( ".pdb" ) )
		|| fP->fCurrentDB.endsWith( CSL1( ".PDB" ) ) )
	{
		// We are dealing with a database
		fP->fCurrentDBtype = eDatabase;
		fP->fCurrentDB.remove( QRegExp( CSL1( ".(pdb|PDB)$" ) ) );

		PilotDatabase *db = new PilotLocalDatabase( dbPath(), fP->fCurrentDB, false );
		if( !db || !db->isOpen() )
		{
			fP->fWidgetUi.fDBInfo->setText( i18n( "<b>Warning:</b> Cannot read "
				"database file %1.", fP->fCurrentDB ) );
			return;
		}
		
		// Deletes the previous loaded db (if one is loaded) and sets db as current.
		setDatabase( db );
		
		dbinfo = db->getDBInfo();
		
		char buff[5];
		set_long( buff, dbinfo.type );
		buff[4]='\0';
		QString tp = QString::fromLatin1( buff );
		
		set_long( buff, dbinfo.creator );
		buff[4]='\0';
		QString cr = QString::fromLatin1( buff );
		
		display.append( i18n( "<p><b>Database:</b> %1, %2 records<br/>"
			, QString::fromLatin1( dbinfo.name ), db->recordCount() ) );
		display.append( i18n( "<b>Type:</b> %1<br/><b>Creator:</b> %2</p>", tp
			, cr ) );

		int currentRecord = 0;
		PilotRecord *pilotRec;
		QTableWidgetItem *recordNr;
		QTableWidgetItem *recordLength;
		QTableWidgetItem *id;

		fP->fWidgetUi.fRecordList->setRowCount( db->recordCount() );

		while( ( pilotRec = db->readRecordByIndex( currentRecord ) ) != 0L )
		{
			if( !( pilotRec->isDeleted() ) )
			{
				recordNr = new QTableWidgetItem();
				recordNr->setText( QString::number( currentRecord ) );
				recordNr->setData( Qt::UserRole, (qulonglong) pilotRec->id() );
				recordLength = new QTableWidgetItem( QString::number( pilotRec->size() ) );
				recordLength->setData( Qt::UserRole, (qulonglong) pilotRec->id() );
				id = new QTableWidgetItem( QString::number( pilotRec->id() ) );
				id->setData( Qt::UserRole, (qulonglong) pilotRec->id() );
				
				fP->fWidgetUi.fRecordList->setItem( currentRecord, 0, recordNr );
				fP->fWidgetUi.fRecordList->setItem( currentRecord, 1, recordLength );
				fP->fWidgetUi.fRecordList->setItem( currentRecord, 2, id );
			}
			currentRecord++;
		}

		DEBUGKPILOT << "Total " << currentRecord << " records.";
	}
	else
	{
		// we are dealing with an application
		fP->fCurrentDBtype = eApplication;

		QByteArray filename = QFile::encodeName( dbPath() + '/' + dbname );
		const char *s = filename;
		struct pi_file *pf = pi_file_open( const_cast<char *>( s ) );
		
		if( !pf )
		{
			fP->fWidgetUi.fDBInfo->setText( i18n( "<b>Warning:</b> Cannot read "
				"application file %1.", dbname ) );
			return;
		}
		
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
		if( pi_file_get_info (pf, &dbinfo ) )
		{
			fP->fWidgetUi.fDBInfo->setText( i18n("<b>Warning:</b> Cannot read "
				"application file %1.", dbname ) );
			return;
		}
#else
		pi_file_get_info( pf, &dbinfo );
#endif

		display.append( i18n("<p><b>Application:</b> %1</p>", dbname ) );
	}
	
	enableWidgets( fP->fCurrentDBtype == eDatabase );

	QDateTime ttime;

	ttime.setTime_t( dbinfo.createDate );
	display.append( i18n( "<p><b>Created:</b> %1<br/>", ttime.toString() ) );

	ttime.setTime_t( dbinfo.modifyDate );
	display.append( i18n( "<b>Modified:</b> %1", ttime.toString() ) );

	ttime.setTime_t( dbinfo.backupDate );
	display.append( i18n( "<br/><b>Backed up:</b> %1</p>", ttime.toString() ) );

	fP->fWidgetUi.fDBInfo->setText( display );
}

void GenericDBWidget::enableWidgets( bool enable )
{
	fP->fWidgetUi.fDBInfoButton->setEnabled( enable );
	fP->fWidgetUi.fAppInfoButton->setEnabled( enable );
	fP->fWidgetUi.fRecordList->setEnabled( enable );
}

void GenericDBWidget::slotDBType( int mode )
{
	FUNCTIONSETUP;
	
	if( !isVisible() )
	{
		return;
	}

	reset();

	QDir dir( dbPath() );
	switch( mode )
	{
		case 1:
			dir.setNameFilters( QStringList() << CSL1( "*.prc" ) );
			break;
		case 2:
			dir.setNameFilters( QStringList() << CSL1( "*.pdb" ) );
			break;
		case 0:
		default:
			dir.setNameFilters( QStringList() << CSL1( "*.pdb" ) << CSL1( "*.prc" ) );
			break;
	}
	
	QStringList l = dir.entryList();
	fP->fWidgetUi.fDBList->addItems( l );
}

void GenericDBWidget::reset()
{
	FUNCTIONSETUP;
	fP->fWidgetUi.fDBList->clear();
	fP->fWidgetUi.fDBInfo->clear();
	fP->fWidgetUi.fRecordList->clearContents();

	fP->fCurrentDB = QString();
}

void GenericDBWidget::slotShowDBInfo()
{
	FUNCTIONSETUP;
	
	PilotLocalDatabase *db = static_cast<PilotLocalDatabase*>( database() );
	
	if( !db || !isVisible() )
	{
		 return;
	}
	
	DBInfo info = db->getDBInfo();
	DBFlagsEditor *dlg = new DBFlagsEditor( &info, this );
	
	if( dlg->exec() )
	{
		DEBUGKPILOT<< "OK pressed, assiging DBInfo, flags="
			<< info.flags << ",  miscFlag=" << info.miscFlags;
		db->setDBInfo( info );

//		KPilotConfig::addFlagsChangedDatabase( dbPath() );
		KPilotSettings::self()->writeConfig();

		slotSelectionChanged();
	}
	
	KPILOT_DELETE( dlg );
}

void GenericDBWidget::slotShowAppInfo()
{
	FUNCTIONSETUP;
	
	PilotLocalDatabase *db = static_cast<PilotLocalDatabase*>( database() );
	
	if( !db || !isVisible() )
	{
		 return;
	}
	
	char *appBlock = new char[0xFFFF];
	int len = db->readAppBlock( ( unsigned char* ) appBlock, 0xFFFF );
	DBAppInfoEditor *dlg = new DBAppInfoEditor( appBlock, len, this );
	
	if( dlg->exec() )
	{
		db->writeAppBlock( (unsigned char*)(dlg->appInfo), dlg->len );

//		KPilotConfig::addAppBlockChangedDatabase( dbPath() );
//		KPilotSettings::self()->writeConfig();
	}
	
	KPILOT_DELETE( dlg );
	
	delete[] appBlock;
}

void GenericDBWidget::slotShowRecord()
{
	FUNCTIONSETUP;
	
	QList<QTableWidgetItem*> rowItems = fP->fWidgetUi.fRecordList->selectedItems();
	
	if( !rowItems.isEmpty() )
	{
		recordid_t id = rowItems.first()->data( Qt::UserRole ).value<id_t>();
		PilotRecord *rec = database()->readRecordById( id );
		
		DBRecordEditor *dlg = new DBRecordEditor( rec, rowItems.first()->row(), this );
		
		if( dlg->exec() )
		{
			rowItems.at( 1 )->setText( QString::number( rec->size() ) );
			rowItems.at( 2 )->setText( QString::number(rec->id() )  );
			
			database()->writeRecord( rec );
			
			KPILOT_DELETE( dlg );
		}
		KPILOT_DELETE( dlg );
	}
	else
	{
		// Either nothing selected, or some error occurred...
		KMessageBox::information( this
			, i18n( "You must select a record for editing." )
			, i18n( "No Record Selected" ), CSL1( "norecordselected" ) );
	}
}
