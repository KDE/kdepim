/* dbViewerWidget.cc		KPilot
**
** Copyright (C) 2003 by Dan Pilone.
**	Authored by Adriaan de Groot
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


#include "config.h"
#include "options.h"

#include <pi-file.h>

#include <qlayout.h>
#include <qdir.h>
#include <qregexp.h>

#include <klistbox.h>
#include <ktextedit.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kmessagebox.h>

#include "listCat.h"

#include "dbviewerWidget.h"
#include "pilotLocalDatabase.h"
#include "pilotDatabase.h"

//#include "hexviewwidget.h"

GenericDBWidget::GenericDBWidget(QWidget *parent, const QString &dbpath) :
	PilotComponent(parent,"component_generic",dbpath), fDB(0L)
{
	FUNCTIONSETUP;

	QGridLayout *g = new QGridLayout( this, 1, 1, SPACING);

	fDBList = new KListBox( this );
	g->addWidget( fDBList, 0, 0 );
	fDBType = new KComboBox( FALSE, this );
	g->addWidget( fDBType, 1, 0 );
	fDBType->insertItem( i18n( "All databases" ) );
	fDBType->insertItem( i18n( "Only Applications (*.prc)" ) );
	fDBType->insertItem( i18n( "Only Resources (*.pdb)" ) );

	QGridLayout *g1 = new QGridLayout( 0, 1, 1);
	fDBInfo = new KTextEdit( this );
	fDBInfo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, fDBInfo->sizePolicy().hasHeightForWidth() ) );
	fDBInfo->setReadOnly( TRUE );
	g1->addWidget( fDBInfo, 0, 0 );
	fDBInfoButton = new KPushButton( i18n( "General Database &information..." ), this );
	g1->addWidget( fDBInfoButton, 1, 0 );
	fAppInfoButton = new KPushButton( i18n( "&Application Info Block (Categories etc.)" ), this );
	g1->addWidget( fAppInfoButton, 2, 0 );

	QGridLayout *g2 = new QGridLayout( 0, 1, 1);
	fRecordList = new KListBox( this );
	g2->addMultiCellWidget( fRecordList, 0, 0, 0, 2 );
/*	CHexBuffer*fHexBuff=new CHexBuffer();
	fHexEdit=new CHexViewWidget( this, "HexEdit", fHexBuff );
	fHexEdit->newFile("/local/home/reinhold/libgtkhtml1.1-dev_1.1.10-0.ximian.3_i386.deb");
	fHexEdit->initFile();
	g2->addMultiCellWidget(fHexEdit, 0,0,0,2);*/

	fAddRecord = new KPushButton( i18n("&Add"), this );
	g2->addWidget( fAddRecord, 1, 0 );
	fEditRecord = new KPushButton( i18n("&Edit"), this );
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

}

GenericDBWidget::~GenericDBWidget()
{
	FUNCTIONSETUP;
	if (fDB) KPILOT_DELETE(fDB);
}


void GenericDBWidget::initialize()
{
	FUNCTIONSETUP;
	fDBInfo->setText(QString::null);
	slotDBType(0);

	fDBList->show();
	fDBInfo->show();
}

void GenericDBWidget::slotSelected(const QString &dbname)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Selected DB " << dbname << endl;
#endif
	struct DBInfo dbinfo;
	QString display("");

	if (fDB) KPILOT_DELETE(fDB);
	currentDB=dbname;
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
		display.append(i18n("<B>Database:</B> %1, %2 records<BR>").arg(dbname).arg(fDB->recordCount()));
		char buff[5];
		set_long(buff, dbinfo.type);
		buff[4]='\0';
		QString tp(buff);
		set_long(buff, dbinfo.creator);
		buff[4]='\0';
		QString cr(buff);
		display.append(i18n("<B>Type:</B> %1, <B>Creator:</B> %2<br><br>").arg(tp).arg(cr));

/*		unsigned char*appBlock=new unsigned char[0xFFFF];
		int len=fDB->readAppBlock(appBlock, 0xFFFF);
		//CHexBuffer*fBuff=new CHexBuffer();
		CHexBuffer*fBuff=fHexEdit->hexBuffer();
		fBuff->assign((char*)appBlock, len);
		fHexEdit->setBuffer(fBuff);
		DEBUGKPILOT<<"fBuff.size="<<fBuff->size()<<endl;
		DEBUGKPILOT<<"fBuff.count="<<fBuff->count()<<endl;
		DEBUGKPILOT<<"len of appBlock="<<len<<endl;*/

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
//	pi_file_close(pf);
// TODO

}

void GenericDBWidget::slotDBType(int mode)
{
	FUNCTIONSETUP;

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
// TODO:	if (editDlg->exec()) markDBDirty(getCurrentDB());
	KMessageBox::information(this, i18n("slotAddRecord on DB %1").arg(getCurrentDB()));
}

void GenericDBWidget::slotEditRecord()
{
	FUNCTIONSETUP;
// TODO:	if (editDlg->exec()) markDBDirty(getCurrentDB());
	KMessageBox::information(this, i18n("slotEditRecord on DB %1").arg(getCurrentDB()));
}

void GenericDBWidget::slotDeleteRecord()
{
	FUNCTIONSETUP;
// TODO:	if (ReallyDelete) markDBDirty(getCurrentDB());
	KMessageBox::information(this, i18n("slotDeleteRecord on DB %1").arg(getCurrentDB()));
}

void GenericDBWidget::slotShowAppInfo()
{
	FUNCTIONSETUP;
	if (!fDB) return;

// TODO:	if (editDlg->exec()) markDBDirty(getCurrentDB());
	KMessageBox::information(this, i18n("slotShowApppInfo on DB %1").arg(getCurrentDB()));
}

void GenericDBWidget::slotShowDBInfo()
{
	FUNCTIONSETUP;
// TODO:	if (editDlg->exec()) markDBDirty(getCurrentDB());
	KMessageBox::information(this, i18n("slotShowDBInfo on DB %1").arg(getCurrentDB()));
}

void GenericDBWidget::enableWidgets(bool enable)
{
	FUNCTIONSETUP;
	fDBInfoButton->setEnabled(enable);
	fAppInfoButton->setEnabled(enable);
	fRecordList->setEnabled(enable);
	fAddRecord->setEnabled(enable);
	fEditRecord->setEnabled(enable);
	fDeleteRecord->setEnabled(enable);
}

#include "dbviewerWidget.moc"
