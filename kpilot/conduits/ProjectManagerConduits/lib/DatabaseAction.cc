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


// $Log$
// Revision 1.1  2002/04/07 12:09:42  kainhofe
// Initial checkin of the conduit. The gui works mostly, but syncing crashes KPilot...
//
// Revision 1.2  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.1  2002/04/07 01:03:52  reinhold
// the list of possible actions is now created dynamically
//
// Revision 1.7  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.6  2002/03/28 13:47:53  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.5  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.4  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//

