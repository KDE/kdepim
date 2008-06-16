#ifndef KEYRINGEDITOR_H
#define KEYRINGEDITOR_H
/* keyringeditor.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "ui_viewer.h"

class LocalKeyringProxy;
class KeyringListModel;
class KeyringHHRecord;

class KeyringViewer : public QMainWindow
{
	Q_OBJECT

public:
	KeyringViewer( QWidget *parent = 0 );
	
	~KeyringViewer();

private slots:
	void selectionChanged( const QModelIndex &index );
	void togglePasswordVisibility();
	void newDatabase();
	void openDatabase();
	
	// Slots to deal with changes in the input fields.
	void nameEditCheck();
	void accountEditCheck();
	void passEditCheck();
	void categoryEditCheck( const QString& newCategory );

	// Slots to deal with addition and deletion of records.
	void deleteRecord();
	void newRecord();
	
private:
	Ui::MainWindow fUi;
	LocalKeyringProxy* fProxy;
	KeyringHHRecord* fCurrentRecord;
	KeyringListModel* fModel;
};

#endif
