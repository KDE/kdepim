/* MultiDB-setup.cc                        KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the MultiDB-conduit plugin.
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

#include "options.h"

#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kfiledialog.h>
#include <klistview.h>

/*#include "MultiDB-conduitDialog.h"
#include "MultiDB-factory.h"*/
#include "MultiDB-conduit.h"
#include "MultiDB-setup.moc"
#include "DatabaseAction.h"
//#include "DatabaseActiondlgPrivate.h"


MultiDBWidgetSetup::MultiDBWidgetSetup(QWidget *w, const char *n,
	const QStringList & a, SyncTypeList_t *lst=0L, KAboutData *abt) : ConduitConfig(w,n,a) {
	FUNCTIONSETUP;

	fConfigWidget = new MultiDBWidgetPrivate(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false, abt);
	synctypes=lst;
	
	SyncTypeIterator_t it(*synctypes);
	KPilotSyncType *st;
	while ( (st = it.current()) != 0 ) {
		++it;
		QRadioButton*btn=fConfigWidget->InsertRadioButton(st->LongName, st->ShortName.latin1());
/*		if (st->getFlag(SYNC_NEEDSFILE)) {
			QObject::connect(btn, SIGNAL(toggled(bool)), fConfigWidget->TextFileName, SLOT(setEnabled(bool)));
		} else {
			QObject::connect(btn, SIGNAL(toggled(bool)), fConfigWidget->TextFileName, SLOT(setDisabled(bool)));
		}*/
	}
	
	QObject::connect(fConfigWidget->ListDatabases, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(edit(QListViewItem*)));
	QObject::connect(fConfigWidget->PushNew,SIGNAL(clicked()), this, SLOT(insert_db()));
	QObject::connect(fConfigWidget->PushEdit,SIGNAL(clicked()), this, SLOT(edit_db()));
	QObject::connect(fConfigWidget->PushDelete,SIGNAL(clicked()), this, SLOT(delete_db()));
	
	// Make the 1st and 3rd columns of the database list editable inline
	fConfigWidget->ListDatabases->setRenameable(0, true);
	fConfigWidget->ListDatabases->setRenameable(2, true);

}

MultiDBWidgetSetup::~MultiDBWidgetSetup() {
	FUNCTIONSETUP;
}

void MultiDBWidgetSetup::slotOk() {
	commitChanges();
	ConduitConfig::slotOk();
}

void MultiDBWidgetSetup::slotApply() {
	commitChanges();
	ConduitConfig::slotApply();
}


/* virtual */ void MultiDBWidgetSetup::commitChanges() {
	FUNCTIONSETUP;

	if (!fConfig) return;
	KConfigGroupSaver s(fConfig, getSettingsGroup());
	
	// walk through all items in the database list and commit the settings
	QListViewItem*item=fConfigWidget->ListDatabases->firstChild();
	QStringList strl;
	while (item) {
		strl << item->text(0);
		fConfig->writeEntry(item->text(0)+"_"+getSyncTypeEntry(), item->text(3));
		fConfig->writeEntry(item->text(0)+"_"+getSyncFileEntry(), item->text(2));
		item=item->nextSibling();
	}
	
	// now write out the default settings
	fConfig->writeEntry(settingsFileList(), strl);
	// TODO: This only works when the numbering starts from 0 and goes straight up to n.
	// This should do some translation from the id to the actual synctype!!!
	fConfig->writeEntry(getSettingsDefaultAct(),
		fConfigWidget->DefaultSyncTypeGroup->id(
			fConfigWidget->DefaultSyncTypeGroup->selected())
	);
}

/* virtual */ void MultiDBWidgetSetup::readSettings() {
	FUNCTIONSETUP;

	if (!fConfig) {
		DEBUGCONDUIT << fname << ": !fConfig..." << endl;
		return;
	}

	KConfigGroupSaver s(fConfig, getSettingsGroup());
	QStringList strl=fConfig->readListEntry(settingsFileList());
	
	for (QStringList::Iterator it = strl.begin(); it != strl.end(); ++it ) {
		int synctypeentry=fConfig->readNumEntry((*it)+"_"+getSyncTypeEntry());
		(void) new KListViewItem(fConfigWidget->ListDatabases, (*it),
					ActIdToName(synctypeentry),
					fConfig->readEntry((*it)+"_"+getSyncFileEntry()),
					fConfig->readEntry((*it)+"_"+getSyncTypeEntry()) );
	}

	// TODO: This only works when the numbering starts from 0 and goes straight up to n.
	// This should do some translation from the synctype to the actual id!!!
	fConfigWidget->DefaultSyncTypeGroup->setButton(fConfig->readNumEntry(getSettingsDefaultAct()));
}

void MultiDBWidgetSetup::insert_db() {
	FUNCTIONSETUP;

	KListViewItem*newitem=new KListViewItem(fConfigWidget->ListDatabases, i18n("New Item"), "ASK", "");
	fConfigWidget->ListDatabases->setSelected(newitem, true);
	edit(newitem);
}

void MultiDBWidgetSetup::edit(QListViewItem*listitem){
	FUNCTIONSETUP;
	if (listitem) {
		DBSyncInfo item(listitem);
		DBSettings*actiondlg=new DBSettings(this, "dbsettings", &item, synctypes);
		if (actiondlg->exec()==QDialog::Accepted ) {
			listitem->setText(0, item.dbname);
			QString act;
			listitem->setText(1, ActIdToName(item.syncaction));
			listitem->setText(2, item.filename);
			listitem->setText(3, act.arg(item.syncaction));
		}
		delete actiondlg;
	}
}

void MultiDBWidgetSetup::edit_db() {
	FUNCTIONSETUP;

	QListViewItem*listitem=fConfigWidget->ListDatabases->selectedItem();
	edit(listitem);
}

QString MultiDBWidgetSetup::ActIdToName(int act) {
	SyncTypeIterator_t it( *synctypes );
	KPilotSyncType *st;
	while ( (st = it.current()) != 0 ) {
		++it;
		if (st->id==act) return st->ShortName;
	}
	return "";
}
int MultiDBWidgetSetup::ActNameToId(QString act) {
	SyncTypeIterator_t it( *synctypes );
	KPilotSyncType *st;
	while ( (st = it.current()) != 0 ) {
		++it;
		if (st->ShortName==act) return st->id;
	}
	return st_ask;
}

void MultiDBWidgetSetup::delete_db() {
	FUNCTIONSETUP;

	QListViewItem*listitem=fConfigWidget->ListDatabases->selectedItem();
	fConfigWidget->ListDatabases->takeItem(listitem);
	delete listitem;
}

