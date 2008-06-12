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

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QtDebug>

#include "pilotLocalDatabase.h"

#include "keyringhhdataproxy.h"
#include "keyringhhrecord.h"

#include "keyringlistmodel.h"
#include "passworddialog.h"

KeyringViewer::KeyringViewer( QWidget *parent )
	: QMainWindow( parent ), fModel( 0L )
{
	fUi.setupUi(this);
	
	fUi.fCategoryFilter->setEnabled( false );
	fUi.fAccountList->setEnabled( false );
	fUi.fNameEdit->setEnabled( false );
	fUi.fAccountEdit->setEnabled( false );
	fUi.fPasswordEdit->setEnabled( false );
	fUi.fPasswordBox->setEnabled( false );
	fUi.fCategoryEdit->setEnabled( false );
	fUi.fDateEdit->setEnabled( false );
	fUi.fNotesEdit->setEnabled( false );
	
	connect( fUi.fAccountList, SIGNAL( clicked( const QModelIndex& ) )
		, this, SLOT( selectionChanged( const QModelIndex& ) ) );
	connect( fUi.fPasswordBox, SIGNAL( clicked() )
		, this, SLOT( togglePasswordVisibility() ) );
		
	// Connect the actions
	connect( fUi.actionNew, SIGNAL( triggered() ), this, SLOT( newDatabase() ) );
	connect( fUi.actionOpen, SIGNAL( triggered() ), this, SLOT( openDatabase() ) );
	connect( fUi.actionExit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );
}

KeyringViewer::~KeyringViewer()
{
	delete fModel;
}

void KeyringViewer::selectionChanged( const QModelIndex &index )
{
	fCurrentRecord = static_cast<KeyringListModel*>(
		fUi.fAccountList->model() )->record( index );

	fUi.fNameEdit->setText( fCurrentRecord->name() );
	fUi.fCategoryEdit->setText( fCurrentRecord->category() );
	fUi.fAccountEdit->setText( fCurrentRecord->account() );
	fUi.fPasswordEdit->setText( fCurrentRecord->password() );
	fUi.fNotesEdit->setText( fCurrentRecord->notes() );
	fUi.fDateEdit->setDateTime( fCurrentRecord->lastChangedDate() );
	
	// Don't show the password by default
	if( fUi.fPasswordEdit->echoMode() == QLineEdit::Normal )
	{
		togglePasswordVisibility();
	}
}

void KeyringViewer::togglePasswordVisibility()
{
	if( fUi.fPasswordEdit->echoMode() == QLineEdit::Password )
	{
		fUi.fPasswordEdit->setEchoMode( QLineEdit::Normal );
	}
	else
	{
		fUi.fPasswordEdit->setEchoMode( QLineEdit::Password );
		fUi.fPasswordBox->setCheckState( Qt::Unchecked );
	}
}

void KeyringViewer::newDatabase()
{
	QString fileName = QFileDialog::getSaveFileName( this,
		tr("New database"), QDir::home().absolutePath(), "*.pdb" );
	
	if( fileName.isEmpty() )
	{
		return;
	}
	
	if( !fileName.endsWith( ".pdb" ) )
	{
		fileName.append( ".pdb" );
	}
	
	QString pass = PasswordDialog::getPassword( this, PasswordDialog::New );
	
	if( pass.isEmpty() )
	{
		// Do not try to create a database with an empty password.
		return;
	}
	
	KeyringHHDataProxy* proxy = new KeyringHHDataProxy( fileName );
	proxy->openDatabase( pass );
	proxy->createDataStore();
	proxy->openDatabase( pass );
	
	fModel = new KeyringListModel( proxy, this );
	
	fUi.fAccountList->setModel( fModel );
	fUi.fAccountList->setEnabled( true );
	fUi.fCategoryFilter->setEnabled( true );
	fUi.fPasswordBox->setEnabled( true );
}

void KeyringViewer::openDatabase()
{
	QString fileName = QFileDialog::getOpenFileName( this,
		tr("New database"), QDir::home().absolutePath(), "*.pdb" );
	
	if( fileName.isEmpty() )
	{
		return;
	}
	
	QFile file( fileName );
	if( !file.exists() )
	{
		return;
	}
	
	QString pass = PasswordDialog::getPassword( this, PasswordDialog::Normal );
	
	KeyringHHDataProxy* proxy = new KeyringHHDataProxy( fileName );
	
	if( !proxy->openDatabase( pass ) )
	{
		delete proxy;
		
		QMessageBox::critical( this, "Error", "Invalid password, could not open database" );
		
		return;
	}
	
	delete fModel;
	
	fModel = new KeyringListModel( proxy, this );
	fUi.fAccountList->setModel( fModel );
	fUi.fAccountList->setEnabled( true );
	fUi.fCategoryFilter->setEnabled( true );
	fUi.fPasswordBox->setEnabled( true );
}
