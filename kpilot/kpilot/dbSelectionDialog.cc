/* dbSelectionDialog.cc                KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines a dialog box that lets the
** user select a set of databases (e.g. which databases
** should be ignored  when doing a backup)
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

#include <qlistview.h>
#include <qpushbutton.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <klineedit.h>

#include "dbSelection_base.h"
#include "dbSelectionDialog.moc"


KPilotDBSelectionDialog::KPilotDBSelectionDialog(QStringList &selectedDBs, QStringList &deviceDBs,
		QStringList &addedDBs, QWidget *w, const char *n) :
	KDialogBase(w, n, true, QString::null, KDialogBase::Ok | KDialogBase::Cancel,
		KDialogBase::Ok, false),
	fSelectedDBs(selectedDBs),
	fAddedDBs(addedDBs),
	fDeviceDBs(deviceDBs)
{
	FUNCTIONSETUP;

	fSelectionWidget = new KPilotDBSelectionWidget(this);
	setMainWidget(fSelectionWidget);

	// Fill the encodings list
	QStringList items(deviceDBs);
	for ( QStringList::Iterator it = fAddedDBs.begin(); it != fAddedDBs.end(); ++it ) {
		if (items.contains(*it)==0) items << (*it);
	}
	for ( QStringList::Iterator it = fSelectedDBs.begin(); it != fSelectedDBs.end(); ++it ) {
		if (items.contains(*it)==0) items << (*it);
	}
	items.sort();

	for ( QStringList::Iterator it = items.begin(); it != items.end(); ++it ) {
		QCheckListItem*checkitem=new QCheckListItem(fSelectionWidget->fDatabaseList,
			*it, QCheckListItem::CheckBox);
		if (fSelectedDBs.contains(*it)) checkitem->setOn(true);
	}

	connect(fSelectionWidget->fNameEdit, SIGNAL(textChanged( const QString & )),
		this, SLOT(slotTextChanged( const QString &)));
	connect(fSelectionWidget->fAddButton, SIGNAL(clicked()),
		this, SLOT(addDB()));
	connect(fSelectionWidget->fRemoveButton, SIGNAL(clicked()),
		this, SLOT(removeDB()));
}

KPilotDBSelectionDialog::~KPilotDBSelectionDialog()
{
	FUNCTIONSETUP;
}

void KPilotDBSelectionDialog::addDB()
{
	FUNCTIONSETUP;
	QString dbname(fSelectionWidget->fNameEdit->text());
	if (!dbname.isEmpty())
	{
		fSelectionWidget->fNameEdit->clear();
		new QCheckListItem(fSelectionWidget->fDatabaseList, dbname,
			QCheckListItem::CheckBox);
		fAddedDBs << dbname;
	}
}

void KPilotDBSelectionDialog::removeDB()
{
	FUNCTIONSETUP;
	QListViewItem*item(fSelectionWidget->fDatabaseList->selectedItem());
	if (item)
	{
		QString dbname=item->text(0);
		if (fDeviceDBs.contains(dbname))
		{
			KMessageBox::error(this, i18n("This is a database that exists on the device. It was not added manually, so it can not removed from the list."), i18n("Database on Device"));
		}
		else
		{
			fSelectedDBs.remove(dbname);
			fAddedDBs.remove(dbname);
			KPILOT_DELETE(item);
		}
	}
	else
	{
		KMessageBox::information(this, i18n("You need to select a database to delete in the list."),i18n("No Database Selected"), "NoDBSelected");
	}
}

QStringList KPilotDBSelectionDialog::getSelectedDBs()
{
	fSelectedDBs.clear();

	//  update the list of selected databases
	QListViewItemIterator it( fSelectionWidget->fDatabaseList );
	while ( it.current() ) {
		QCheckListItem *item = dynamic_cast<QCheckListItem*>(it.current());
		++it;

		if ( item && item->isOn() )
			fSelectedDBs << item->text();
	}

	return fSelectedDBs;
}

void KPilotDBSelectionDialog::slotTextChanged( const QString& dbname)
{
	FUNCTIONSETUP;
	fSelectionWidget->fAddButton->setDisabled(dbname.isEmpty());
}
