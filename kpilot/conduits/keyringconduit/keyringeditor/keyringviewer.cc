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

#include <QtCore/QDir>
#include <QtCore/QtDebug>

#include <kfiledialog.h>
#include <kmessagebox.h>

#include "pilotLocalDatabase.h"
#include "pilotRecord.h"

#include "localkeyringproxy.h"
#include "keyringhhrecord.h"

#include "keyringlistmodel.h"
#include "passworddialog.h"

KeyringViewer::KeyringViewer( QWidget *parent )
	: QMainWindow( parent ), fProxy( 0L ), fCurrentRecord( 0L ), fModel( 0L )
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
	fUi.fDeleteButton->setEnabled( false );
	fUi.fNewButton->setEnabled( false );
	
	connect( fUi.fAccountList, SIGNAL( clicked( const QModelIndex& ) )
		, this, SLOT( selectionChanged( const QModelIndex& ) ) );
	connect( fUi.fPasswordBox, SIGNAL( clicked() )
		, this, SLOT( togglePasswordVisibility() ) );
	connect( fUi.fNameEdit, SIGNAL( editingFinished () )
		, this, SLOT( nameEditCheck() ) );
	connect( fUi.fAccountEdit, SIGNAL( editingFinished () )
		, this, SLOT( accountEditCheck() ) );
	connect( fUi.fPasswordEdit, SIGNAL( editingFinished () )
		, this, SLOT( passEditCheck() ) );
	connect( fUi.fCategoryEdit, SIGNAL( currentIndexChanged( const QString& ) )
		, this, SLOT( categoryEditCheck( const QString& ) ) );
	connect( fUi.fDeleteButton, SIGNAL( clicked() )
		, this, SLOT( deleteRecord() ) );
	connect( fUi.fNewButton, SIGNAL( clicked() )
		, this, SLOT( newRecord() ) );
		
	// Connect the actions
	connect( fUi.actionNew, SIGNAL( triggered() ), this, SLOT( newDatabase() ) );
	connect( fUi.actionOpen, SIGNAL( triggered() ), this, SLOT( openDatabase() ) );
	connect( fUi.actionExit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );
}

KeyringViewer::~KeyringViewer()
{
	delete fProxy;
	delete fModel;
}

void KeyringViewer::selectionChanged( const QModelIndex &index )
{
	// Do this here because the QPlainTextEdit does not have a signal like
	// editingFinished, but only a text changed. We don't want to unpack and pack
	// the record for each character we type, do we?
	if( fCurrentRecord && fCurrentRecord->notes() != fUi.fNotesEdit->toPlainText() )
	{
		QDateTime now = QDateTime::currentDateTime();
		
		fCurrentRecord->setNotes( fUi.fNotesEdit->toPlainText() );
		fCurrentRecord->setLastChangedDate( now );
		fCurrentRecord->setModified();
		fProxy->saveRecord( fCurrentRecord );
	}
	
	fUi.fDeleteButton->setEnabled( true );
	
	fCurrentRecord = static_cast<KeyringListModel*>(
		fUi.fAccountList->model() )->record( index );

	fUi.fNameEdit->setText( fCurrentRecord->name() );
	int i = fProxy->categories().indexOf( fCurrentRecord->category() );
	fUi.fCategoryEdit->setCurrentIndex( i );
	fUi.fAccountEdit->setText( fCurrentRecord->account() );
	fUi.fPasswordEdit->setText( fCurrentRecord->password() );
	fUi.fNotesEdit->setPlainText( fCurrentRecord->notes() );
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
	QString fileName = KFileDialog::getSaveFileName( KUrl(), "*.pdb", this,
		tr("New database") );
	
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
	
	// Clean up eventually open database.
	delete fProxy;
	delete fModel;
	
	fProxy = new LocalKeyringProxy( fileName );
	fProxy->openDatabase( pass );
	fProxy->createDataStore();
	fProxy->openDatabase( pass );
	
	fModel = new KeyringListModel( fProxy, this );
	
	fUi.fAccountList->setModel( fModel );
	fUi.fAccountList->setEnabled( true );
	fUi.fCategoryFilter->addItems( fProxy->categories() );
	fUi.fCategoryFilter->setEnabled( true );
	fUi.fNameEdit->setEnabled( true );
	fUi.fAccountEdit->setEnabled( true );
	fUi.fPasswordEdit->setEnabled( true );
	fUi.fPasswordBox->setEnabled( true );
	fUi.fCategoryEdit->addItems( fProxy->categories() );
	fUi.fCategoryEdit->setEnabled( true );
	fUi.fNotesEdit->setEnabled( true );
	fUi.fNewButton->setEnabled( true );
}

void KeyringViewer::openDatabase()
{
	QString fileName = KFileDialog::getOpenFileName( KUrl(), "*.pdb", this,
		tr("Open database") );
	
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
	
	LocalKeyringProxy* proxy = new LocalKeyringProxy( fileName );
	
	if( !proxy->openDatabase( pass ) )
	{
		delete proxy;
		
		KMessageBox::error( this, "Error", "Invalid password, could not open database" );
		
		return;
	}
	
	delete fProxy;
	delete fModel;
	
	fProxy = proxy;
	fModel = new KeyringListModel( fProxy, this );
	
	fUi.fAccountList->setModel( fModel );
	fUi.fAccountList->setEnabled( true );
	fUi.fCategoryFilter->addItems( fProxy->categories() );
	fUi.fCategoryFilter->setEnabled( true );
	fUi.fNameEdit->setEnabled( true );
	fUi.fAccountEdit->setEnabled( true );
	fUi.fPasswordEdit->setEnabled( true );
	fUi.fPasswordBox->setEnabled( true );
	fUi.fCategoryEdit->addItems( fProxy->categories() );
	fUi.fCategoryEdit->setEnabled( true );
	fUi.fNotesEdit->setEnabled( true );
	fUi.fNewButton->setEnabled( true );
}

void KeyringViewer::nameEditCheck()
{
	if( !fCurrentRecord ) return;
	
	if( fUi.fNameEdit->text() != fCurrentRecord->name() )
	{
		QDateTime now = QDateTime::currentDateTime();
		
		fCurrentRecord->setName( fUi.fNameEdit->text() );
		fCurrentRecord->setLastChangedDate( now );
		fCurrentRecord->setModified();
		fProxy->saveRecord( fCurrentRecord );
	}
}

void KeyringViewer::accountEditCheck()
{
	if( !fCurrentRecord ) return;
	
	if( fUi.fAccountEdit->text() != fCurrentRecord->account() )
	{
		QDateTime now = QDateTime::currentDateTime();
		
		fCurrentRecord->setAccount( fUi.fAccountEdit->text() );
		fCurrentRecord->setLastChangedDate( now );
		fCurrentRecord->setModified();
		fProxy->saveRecord( fCurrentRecord );
	}
}

void KeyringViewer::passEditCheck()
{
	if( !fCurrentRecord ) return;
	
	if( fUi.fPasswordEdit->text() != fCurrentRecord->password() )
	{
		QDateTime now = QDateTime::currentDateTime();
		
		fCurrentRecord->setPassword( fUi.fPasswordEdit->text() );
		fCurrentRecord->setLastChangedDate( now );
		fCurrentRecord->setModified();
		fProxy->saveRecord( fCurrentRecord );
	}
}

void KeyringViewer::categoryEditCheck( const QString& newCategory )
{
	if( !fCurrentRecord ) return;
	
	if( newCategory != fCurrentRecord->category() )
	{
		QDateTime now = QDateTime::currentDateTime();
		
		fProxy->setCategory( fCurrentRecord, newCategory );
		
		fCurrentRecord->setLastChangedDate( now );
		fCurrentRecord->setModified();
		fProxy->saveRecord( fCurrentRecord );
	}
}

void KeyringViewer::deleteRecord()
{
	qDebug() << fCurrentRecord;
	if( !fCurrentRecord ) return;
	
	int row = fUi.fAccountList->currentIndex().row();
	QModelIndex parent =fUi.fAccountList->currentIndex().parent();
	
	if( fUi.fAccountList->model()->removeRow( row, parent ) )
	{
		fProxy->deleteRecord( fCurrentRecord );
		fCurrentRecord = 0L;
		
		if( fModel->rowCount() > 0 )
		{
			selectionChanged( fModel->index( fModel->rowCount() - 1 ) );
		}
		else
		{
			fUi.fDeleteButton->setEnabled( false );
		}
		
		// Clear the fields.
		fUi.fNameEdit->setText( QString() );
		fUi.fCategoryEdit->setCurrentIndex( 0 );
		fUi.fAccountEdit->setText( QString() );
		fUi.fPasswordEdit->setText( QString() );
		fUi.fNotesEdit->setPlainText( QString() );
		fUi.fDateEdit->setDateTime( QDateTime() );
	}
}

void KeyringViewer::newRecord()
{
	KeyringHHRecord* rec = fProxy->createRecord();
	fProxy->addRecord( rec );
	fModel->addRecord( rec );
}
