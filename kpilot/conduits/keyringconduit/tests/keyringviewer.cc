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

#include "keyringviewer.h"

#include "keyringhhrecord.h"

#include "keyringlistmodel.h"

KeyringViewer::KeyringViewer( QWidget *parent, KeyringHHDataProxy *proxy )
	: QDialog( parent )
{
	fModel = new KeyringListModel( proxy );
	
	fUi.setupUi(this);
	fUi.fNameEdit->setEnabled( false );
	fUi.fAccountEdit->setEnabled( false );
	fUi.fPasswordEdit->setEnabled( false );
	fUi.fCategoryEdit->setEnabled( false );
	fUi.fDateEdit->setEnabled( false );
	fUi.fNotesEdit->setEnabled( false );
	fUi.fAccountList->setModel( fModel );
	
	connect( fUi.fAccountList, SIGNAL( clicked( const QModelIndex& ) )
		, this, SLOT( selectionChanged( const QModelIndex& ) ) );
	
}

void KeyringViewer::selectionChanged( const QModelIndex &index )
{
	KeyringHHRecord *rec = static_cast<KeyringListModel*>( 
		fUi.fAccountList->model() )->record( index );

	fUi.fAccountEdit->setText( rec->account() );
}
