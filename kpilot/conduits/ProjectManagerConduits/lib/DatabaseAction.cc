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
/*#include <qtabwidget.h>*/
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qwidget.h>
#include <qhbox.h>

#include <kconfig.h>
/*#include <kinstance.h>
#include <kaboutdata.h>*/
#include <kfiledialog.h>

#include "DatabaseAction.h"
#include "DatabaseAction.moc"
//#include "DatabaseActionDialog.h"


DBSettings::DBSettings(QWidget *w, const char *n, DBSyncInfo*itm, SyncTypeList_t *tps,
	bool changeDBName, bool allowask) :
	KDialogBase(w, n, true, QString::null,
		KDialogBase::Ok | KDialogBase::Cancel,
		KDialogBase::Ok, true) {
	FUNCTIONSETUP;
	item=itm;
	synctypes=tps;
	
	fMainWidget = makeHBoxMainWidget();
	fConfigWidget = new DatabaseActionDlgPrivate(widget());

	SyncTypeIterator_t it(*synctypes);
	KPilotSyncType *st;
	while ( (st = it.current()) != 0 ) {
		++it;
		QRadioButton*btn=fConfigWidget->InsertRadioButton(st->LongName, st->ShortName.latin1());
		if (st->getFlag(SYNC_NEEDSFILE)) {
			QObject::connect(btn, SIGNAL(toggled(bool)), fConfigWidget->TextFileName, SLOT(setEnabled(bool)));
		} else {
			QObject::connect(btn, SIGNAL(toggled(bool)), fConfigWidget->TextFileName, SLOT(setDisabled(bool)));
		}
	}

	QObject::connect(fConfigWidget->PushBrowse,SIGNAL(clicked()), this,SLOT(slotBrowseFile()));
	
	fConfigWidget->EditDatabaseName->setText(item->dbname);
	fConfigWidget->DefaultSyncTypeGroup->setButton(SyncTypeToId(item->syncaction));
	fConfigWidget->TextFileName->setText(item->filename);
	fConfigWidget->EditDatabaseName->setEnabled(changeDBName);
	fConfigWidget->RadioAsk->setEnabled(allowask);
}

DBSettings::~DBSettings() {
	FUNCTIONSETUP;
}

void DBSettings::slotBrowseFile() {
	FUNCTIONSETUP;
	if (fConfigWidget->TextFileName->isEnabled()) {
		QString fileName = fConfigWidget->TextFileName->text();
		QString fn=KFileDialog::getOpenFileName(fileName, i18n("*.vcs *.ics|ICalendars\n*.pdb|Palm Database file (*.pdb)\n*.*|All files"), this);
		if(fn.isNull()) return;
		fConfigWidget->TextFileName->setText(fn);
	}
}

void DBSettings::slotOk() {
	commitChanges();
	KDialogBase::slotOk();
}

void DBSettings::slotApply() {
	FUNCTIONSETUP;
	commitChanges();
}

void DBSettings::commitChanges() {
	FUNCTIONSETUP;
	item->dbname=fConfigWidget->EditDatabaseName->text();
	item->filename=fConfigWidget->TextFileName->text();
	item->syncaction=IdToSyncType(fConfigWidget->DefaultSyncTypeGroup->id(fConfigWidget->DefaultSyncTypeGroup->selected()));
}

int DBSettings::IdToSyncType(int tp) {
	if (synctypes->at(tp)) return synctypes->at(tp)->id; else return st_ask;
};

int DBSettings::SyncTypeToId(int tt) {
	KPilotSyncType*st=synctypes->first();
	while (st) {
		if (st->id==tt) return synctypes->at();
		st=synctypes->next();
	}
	return -1;
}

