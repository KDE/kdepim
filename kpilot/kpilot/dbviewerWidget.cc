/* dbViewerWidget.cc		KPilot
**
** Copyright (C) 2003 by Dan Pilone.
**	Written 2003 by Reinhold Kainhofer and Adriaan de Groot
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"

#include <unistd.h>
#include <stdio.h>

#include <pi-file.h>
#include <pi-dlp.h>

#include <qlayout.h>
#include <qdir.h>
#include <qregexp.h>
#include <qlistview.h>

#include <klistbox.h>
#include <ktextedit.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kmessagebox.h>

#include "listCat.h"
#include "listItems.h"

#include "dbviewerWidget.h"
#include "pilotLocalDatabase.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"
#include "dbFlagsEditor.h"
#include "dbRecordEditor.h"
#include "dbAppInfoEditor.h"
#include "kpilotConfig.h"


#include "dbviewerWidget.moc"


GenericDBWidget::GenericDBWidget(QWidget *parent, const QString &dbpath) :
	PilotComponent(parent,"component_generic",dbpath), fDB(0L)
{
	FUNCTIONSETUP;
	setupWidget();
	fRecList.setAutoDelete(true);
}


void GenericDBWidget::setupWidget()
{
	QGridLayout *g = new QGridLayout( this, 1, 1, SPACING);

	fDBList = new KListBox( this );
	g->addWidget( fDBList, 0, 0 );
	fDBType = new KComboBox( FALSE, this );
	g->addWidget( fDBType, 1, 0 );
	fDBType->insertItem( i18n( "All Databases" ) );
	fDBType->insertItem( i18n( "Only Applications (*.prc)" ) );
	fDBType->insertItem( i18n( "Only Databases (*.pdb)" ) );

	QGridLayout *g1 = new QGridLayout( 0, 1, 1);
	fDBInfo = new KTextEdit( this );
	fDBInfo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, fDBInfo->sizePolicy().hasHeightForWidth() ) );
	fDBInfo->setReadOnly( TRUE );
	g1->addWidget( fDBInfo, 0, 0 );
	fDBInfoButton = new KPushButton( i18n( "General Database &Information" ), this );
	g1->addWidget( fDBInfoButton, 1, 0 );
	fAppInfoButton = new KPushButton( i18n( "&Application Info Block (Categories etc.)" ), this );
	g1->addWidget( fAppInfoButton, 2, 0 );

	QGridLayout *g2 = new QGridLayout( 0, 1, 1);
	fRecordList = new KListView( this );
	g2->addMultiCellWidget( fRecordList, 0, 0, 0, 2 );
	fRecordList->addColumn(i18n("Rec. Nr."));
	fRecordList->addColumn(i18n("Length"));
	fRecordList->addColumn(i18n("Record ID"));
	fRecordList->setAllColumnsShowFocus(true);
	fRecordList->setResizeMode( KListView::LastColumn );
	fRecordList->setFullWidth( TRUE );
	fRecordList->setItemsMovable( FALSE );

	fAddRecord = new KPushButton( i18n("&Add..."), this );
	g2->addWidget( fAddRecord, 1, 0 );
	fEditRecord = new KPushButton( i18n("&Edit..."), this );
	g2->addWidget( fEditRecord, 1, 1 );
	fDeleteRecord = new KPushButton( i18n("&Delete"), this );
	g2->addWidget( fDeleteRecord, 1, 2 );

	g1->addLayout( g2, 3, 0 );


	g->addMultiCellLayout( g1, 0, 1, 1, 1 );
	resize( QSize(682, 661).expandedTo(minimumSizeHint()) );

	connect(fDBList, SIGNAL(highlighted(const QString &)),
		this, SLOT(slotSelected(const QString &)));
	connect(fDBType, SIGNAL(activated(int)),
		this, SLOT(slotDBType(int)));
	connect(fDBInfoButton,  SIGNAL(clicked()),
		this, SLOT(slotShowDBInfo()));
	connect(fAppInfoButton,  SIGNAL(clicked()),
		this, SLOT(slotShowAppInfo()));
	connect(fAddRecord,  SIGNAL(clicked()),
		this, SLOT(slotAddRecord()));
	connect(fEditRecord,  SIGNAL(clicked()),
		this, SLOT(slotEditRecord()));
	connect(fDeleteRecord,  SIGNAL(clicked()),
		this, SLOT(slotDeleteRecord()));
	connect(fRecordList, SIGNAL(executed(QListViewItem*)),
		this, SLOT(slotEditRecord(QListViewItem*)));

}

GenericDBWidget::~GenericDBWidget()
{
	FUNCTIONSETUP;
	if (fDB) KPILOT_DELETE(fDB);
}


void GenericDBWidget::showComponent()
{
	FUNCTIONSETUP;
	fDBInfo->setText(QString::null);
	slotDBType(0);

	fDBList->show();
	fDBInfo->show();
}

void GenericDBWidget::hideComponent()
{
	reset();
}

void GenericDBWidget::slotSelected(const QString &dbname)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Selected DB " << dbname << endl;
#endif
	struct DBInfo dbinfo;
	QString display("");
	fRecList.clear();
	fRecordList->clear();

	if (fDB) KPILOT_DELETE(fDB);
	currentDB=dbname;

	if (!shown) return;

	if (dbname.endsWith(".pdb") || dbname.endsWith(".PDB"))
	{
		// We are dealing with a database
		currentDBtype=eDatabase;

		currentDB.remove( QRegExp(".(pdb|PDB)$") );

		fDB=new PilotLocalDatabase(dbPath(), currentDB, false);
		if (!fDB || !fDB->isDBOpen())
		{
			fDBInfo->setText(i18n("<B>Warning:</B> Cannot read "
				"database file %1.").arg(currentDB));
			return;
		}
		dbinfo=fDB->getDBInfo();
		display.append(i18n("<B>Database:</B> %1, %2 records<BR>").arg(dbinfo.name).arg(fDB->recordCount()));
		char buff[5];
		set_long(buff, dbinfo.type);
		buff[4]='\0';
		QString tp(buff);
		set_long(buff, dbinfo.creator);
		buff[4]='\0';
		QString cr(buff);
		display.append(i18n("<B>Type:</B> %1, <B>Creator:</B> %2<br><br>").arg(tp).arg(cr));

		int currentRecord = 0;
		PilotRecord *pilotRec;

#ifdef DEBUG
		DEBUGKPILOT << fname << ": Reading database "<<dbname<<"..." << endl;
#endif

		while ((pilotRec = fDB->readRecordByIndex(currentRecord)) != 0L)
		{
//			if (!(pilotRec->isDeleted()) )
			{
				PilotListViewItem*item=new PilotListViewItem(fRecordList,
					QString::number(currentRecord), QString::number(pilotRec->getLen()),  QString::number(pilotRec->id()), QString::null,
					pilotRec->id(), pilotRec);
				item->setNumericCol(0, true);
				item->setNumericCol(1, true);
				item->setNumericCol(2, true);
			}
			fRecList.append(pilotRec);

			currentRecord++;
		}

#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Total " << currentRecord << " records" << endl;
#endif

	}
	else
	{
		// we are dealing with an application
		currentDBtype=eApplication;

		QCString filename = QFile::encodeName(dbPath() + CSL1("/") + dbname);
		const char *s = filename;
		struct pi_file *pf = pi_file_open(const_cast<char *>(s));
		if (!pf)
		{
			fDBInfo->setText(i18n("<B>Warning:</B> Cannot read "
				"application file %1.").arg(dbname));
			return;
		}
		if (pi_file_get_info(pf,&dbinfo))
		{
			fDBInfo->setText(i18n("<B>Warning:</B> Cannot read "
				"application file %1.").arg(dbname));
			return;
		}
		display.append(i18n("<B>Application:</B> %1<BR><BR>").arg(dbname));
	}
	enableWidgets(currentDBtype==eDatabase);

	QDateTime ttime;

	ttime.setTime_t(dbinfo.createDate);
	display.append(i18n("Created: %1<BR>").arg(ttime.toString()));

	ttime.setTime_t(dbinfo.modifyDate);
	display.append(i18n("Modified: %1<BR>").arg(ttime.toString()));

	ttime.setTime_t(dbinfo.backupDate);
	display.append(i18n("Backed up: %1<BR>").arg(ttime.toString()));

	fDBInfo->setText(display);
}

void GenericDBWidget::slotDBType(int mode)
{
	FUNCTIONSETUP;
	if (!shown) return;

	reset();

	QDir dir(dbPath());
	switch (mode)
	{
		case 1:
			dir.setNameFilter(CSL1("*.prc")); break;
		case 2:
			dir.setNameFilter(CSL1("*.pdb")); break;
		case 0:
		default:
			dir.setNameFilter(CSL1("*.pdb;*.prc")); break;
	}
	QStringList l = dir.entryList();
	fDBList->insertStringList(l);
}

void GenericDBWidget::reset()
{
	FUNCTIONSETUP;
	fDBList->clear();
	fDBInfo->clear();
	fRecordList->clear();
	if (fDB)  KPILOT_DELETE(fDB);
	currentDB=QString::null;
}

void GenericDBWidget::slotAddRecord()
{
	FUNCTIONSETUP;
	PilotRecord*rec=new PilotRecord(0L, 0, 0, 0, 0);
	PilotListViewItem*item=new PilotListViewItem(fRecordList,
		QString::number(-1), QString::number(rec->getLen()),
		QString::number(rec->id()), QString::null,
		rec->id(), rec);
	if (slotEditRecord(item))
	{
		fRecList.append(rec);
	}
	else
	{
		KPILOT_DELETE(item);
		KPILOT_DELETE(rec);
	}
}

bool GenericDBWidget::slotEditRecord(QListViewItem*item)
{
	FUNCTIONSETUP;
	PilotListViewItem*currRecItem=dynamic_cast<PilotListViewItem*>(item);
	if (currRecItem)
	{
		PilotRecord*rec=(PilotRecord*)currRecItem->rec();
		int nr=currRecItem->text(0).toInt();
		DBRecordEditor*dlg=new DBRecordEditor(rec, nr, this);
		if (dlg->exec())
		{
			currRecItem->setText(1, QString::number(rec->getLen()));
			currRecItem->setText(2, QString::number(rec->id()));
			fRecordList->triggerUpdate();
			writeRecord(rec);
			KPILOT_DELETE(dlg);
			return true;
		}
		KPILOT_DELETE(dlg);
	}
	else
	{
		// Either nothing selected, or some error occurred...
		KMessageBox::information(this, i18n("You must select a record for editing."), i18n("No Record Selected"), "norecordselected");
	}
	return false;
}
void GenericDBWidget::slotEditRecord()
{
	slotEditRecord(fRecordList->selectedItem());
}

void GenericDBWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;
	PilotListViewItem*currRecItem=dynamic_cast<PilotListViewItem*>(fRecordList->selectedItem());
	if (currRecItem && (KMessageBox::questionYesNo(this, i18n("<qt>Do you really want to delete the selected record? This cannot be undone.<br><br>Delete record?<qt>"), i18n("Deleting Record"))==KMessageBox::Yes) )
	{
		PilotRecord*rec=(PilotRecord*)currRecItem->rec();
		rec->setDeleted();
		writeRecord(rec);
		// fRecordList->triggerUpdate();
		KPILOT_DELETE(currRecItem);
	}
}

void GenericDBWidget::slotShowAppInfo()
{
	FUNCTIONSETUP;
	if (!fDB) return;
	char*appBlock=new char[0xFFFF];
	int len=fDB->readAppBlock((unsigned char*)appBlock, 0xFFFF);
	DBAppInfoEditor*dlg=new DBAppInfoEditor(appBlock, len, this);
	if (dlg->exec())
	{
		fDB->writeAppBlock( (unsigned char*)(dlg->appInfo), dlg->len );

		KPilotConfig::addAppBlockChangedDatabase(getCurrentDB());
		KPilotSettings::writeConfig();
	}
	KPILOT_DELETE(dlg);
	delete[] appBlock;
}

void GenericDBWidget::slotShowDBInfo()
{
	FUNCTIONSETUP;
	if ( !fDB || !shown ) return;
	DBInfo db=fDB->getDBInfo();
	DBFlagsEditor*dlg=new DBFlagsEditor(&db, this);
	if (dlg->exec())
	{
#ifdef DEBUG
		DEBUGKPILOT<<"OK pressed, assiging DBInfo, flags="<<
			db.flags<<",  miscFlag="<<db.miscFlags<<endl;
#endif
		fDB->setDBInfo(db);

		KPilotConfig::addFlagsChangedDatabase(getCurrentDB());
		KPilotSettings::writeConfig();

		slotSelected(fDBList->currentText());
	}
	KPILOT_DELETE(dlg);
}

void GenericDBWidget::enableWidgets(bool enable)
{
	//FUNCTIONSETUP;
	fDBInfoButton->setEnabled(enable);
	fAppInfoButton->setEnabled(enable);
	fRecordList->setEnabled(enable);
	fAddRecord->setEnabled(enable);
	fEditRecord->setEnabled(enable);
	fDeleteRecord->setEnabled(enable);
}

void GenericDBWidget::writeRecord(PilotRecord*r)
{
	// FUNCTIONSETUP;
	if (fDB && r)
	{
		fDB->writeRecord(r);
		markDBDirty(getCurrentDB());
	}
}





