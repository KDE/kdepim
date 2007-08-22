/* KPilot
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "config_dialog_dbselection.h"

#include <kmessagebox.h>

#include "options.h"

static inline void appendStringList(QStringList &items, const QStringList &add)
{
	QStringList::ConstIterator e = add.end();
	for ( QStringList::ConstIterator it = add.begin(); it != e; ++it )
	{
		if (items.contains(*it)==0)
		{
			items << (*it);
		}
	}
}

KPilotDBSelectionDialog::KPilotDBSelectionDialog(const QStringList &selectedDBs,
	const QStringList &deviceDBs, const QStringList &addedDBs, QWidget *w, const char *n) :
	KDialog(w),
	fSelectedDBs(selectedDBs),
	fAddedDBs(addedDBs),
	fDeviceDBs(deviceDBs)
{
	FUNCTIONSETUP;

	if (n)
	{
		setObjectName(n);
	}
	setButtons(Ok|Cancel);
	setDefaultButton(Ok);
	setModal(false);
	
	if( !w )
	{
		w = new QWidget( this );
	}
	
	fSelectionWidget.setupUi( w );
	setMainWidget( w );

	// Fill the encodings list
	QStringList items(deviceDBs);
	appendStringList(items, fAddedDBs);
	appendStringList(items, fSelectedDBs);
	items.sort();

	for ( QStringList::Iterator it = items.begin(); it != items.end(); ++it )
	{
		QListWidgetItem *checkitem = new QListWidgetItem(*it
			, fSelectionWidget.fDatabaseList);
		checkitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled 
			| Qt::ItemIsUserCheckable);
		checkitem->setCheckState(fSelectedDBs.contains(*it) ? Qt::Checked 
			: Qt::Unchecked);
	}

	fSelectionWidget.fAddButton->setEnabled(false);
	fSelectionWidget.fRemoveButton->setEnabled(false);

	connect(fSelectionWidget.fNameEdit, SIGNAL(textChanged( const QString & )),
		this, SLOT(textChanged( const QString &)));
	connect(fSelectionWidget.fAddButton, SIGNAL(clicked()),
		this, SLOT(addDB()));
	connect(fSelectionWidget.fRemoveButton, SIGNAL(clicked()),
		this, SLOT(removeDB()));
	connect(fSelectionWidget.fDatabaseList, SIGNAL(currentRowChanged(int)),
		this, SLOT(dbSelectionChanged(int)) );
}

KPilotDBSelectionDialog::~KPilotDBSelectionDialog()
{
	FUNCTIONSETUP;
}

void KPilotDBSelectionDialog::addDB()
{
	FUNCTIONSETUP;
	QString dbname(fSelectionWidget.fNameEdit->text());
	if (!dbname.isEmpty())
	{
		fSelectionWidget.fNameEdit->clear();
		QListWidgetItem *checkitem = new QListWidgetItem(dbname
			, fSelectionWidget.fDatabaseList);
		checkitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled
			| Qt::ItemIsUserCheckable);
		checkitem->setCheckState(Qt::Unchecked);
		fAddedDBs << dbname;
	}
}

void KPilotDBSelectionDialog::removeDB()
{
	FUNCTIONSETUP;
	QListWidgetItem *item(fSelectionWidget.fDatabaseList->currentItem());
	if (item)
	{
		QString dbname=item->text();
		if (fDeviceDBs.contains(dbname))
		{
			KMessageBox::error(this, i18n("This is a database that exists on the device. It was not added manually, so it can not removed from the list."), i18n("Database on Device"));
		}
		else
		{
			fSelectedDBs.removeAll(dbname);
			fAddedDBs.removeAll(dbname);
			KPILOT_DELETE(item);
		}
	}
	else
	{
		KMessageBox::information(this, i18n("You need to select a database to delete in the list."),i18n("No Database Selected"), CSL1("NoDBSelected"));
	}
}

QStringList KPilotDBSelectionDialog::getSelectedDBs()
{
	fSelectedDBs.clear();

	//  update the list of selected databases
	int c = fSelectionWidget.fDatabaseList->count();
	for (int i = 0; i<c; ++i)
	{
		QListWidgetItem *item = fSelectionWidget.fDatabaseList->item(i);

		if ( item && item->checkState() )
		{
			fSelectedDBs << item->text();
		}
	}

	return fSelectedDBs;
}

void KPilotDBSelectionDialog::textChanged( const QString& dbname)
{
	FUNCTIONSETUP;
	fSelectionWidget.fAddButton->setDisabled(dbname.isEmpty());
}

void KPilotDBSelectionDialog::dbSelectionChanged( int row )
{
	FUNCTIONSETUP;
	fSelectionWidget.fRemoveButton->setEnabled( row >= 0 );
}


