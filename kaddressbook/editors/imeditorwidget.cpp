/*
	imeditorwidget.cpp

	IM address editor widget for KAddressBook

	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlistview.h>
#include <qstringlist.h>

#include <kdialogbase.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>
#include <kpushbutton.h>
#include <ktrader.h>

#include "imaddresswidget.h"
#include "imeditorwidget.h"


extern "C" {
  void *init_libkaddrbk_instantmessaging()
  {
    return ( new IMEditorWidgetFactory );
  }
}

QString IMEditorWidgetFactory::pageTitle() const
{
  return i18n( "IM Addresses" );
}

QString IMEditorWidgetFactory::pageIdentifier() const
{
  return "instantmessaging";
}

/*===========================================================================*/

IMAddressLVI::IMAddressLVI( KListView *parent, KPluginInfo *protocol,
                            const QString &address, const IMContext &context )
  : KListViewItem( parent )
{
  setProtocol( protocol );
  setAddress( address );
  setContext( context );
}

void IMAddressLVI::setAddress( const QString &address )
{
	//irc uses 0xE120 to seperate the nick and the server group.

  	QString serverOrGroup = address.section(QChar(0xE120),1);
	if(serverOrGroup.isEmpty())
		setText( 1, address );
	else {
		QString nickname = address.section(QChar(0xE120),0,0);
		setText( 1, i18n("%1 on %2").arg(nickname).arg(serverOrGroup) );
	}
	mAddress = address;
}

void IMAddressLVI::setContext( const IMContext &context )
{
	mContext = context;
	// set context
/*	switch ( context )
	{
	case Home:
		setText( 2, i18n( "Home" ) );
		break;
	case Work:
		setText( 2, i18n( "Work" ) );
		break;
	case Any:
		setText( 2, i18n( "Any" ) );
		break;
	}
*/
}

void IMAddressLVI::setProtocol( KPluginInfo *protocol )
{
	mProtocol = protocol;
	setPixmap( 0,  SmallIcon( mProtocol->icon() ) );
	setText( 0, mProtocol->name() );
}

KPluginInfo * IMAddressLVI::protocol() const
{
	return mProtocol;
}

IMContext IMAddressLVI::context() const
{
	return mContext;
}

QString IMAddressLVI::address() const
{
	return mAddress;
}

void IMAddressLVI::activate()
{
	// show editor
}

/*===========================================================================*/

IMEditorWidget::IMEditorWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
: KAB::ContactEditorWidget( ab, parent, name ), mReadOnly( false )
{
	mWidget = new IMEditorBase( this );

	connect( mWidget->btnAdd, SIGNAL( clicked() ), SLOT( slotAdd() ) );
	connect( mWidget->btnEdit, SIGNAL( clicked() ), SLOT( slotEdit() ) );
	connect( mWidget->btnDelete, SIGNAL( clicked() ), SLOT( slotDelete() ) );
	connect( mWidget->lvAddresses, SIGNAL( selectionChanged() ), SLOT( slotUpdateButtons() ) );

	connect( mWidget->lvAddresses, SIGNAL( doubleClicked ( QListViewItem *, const QPoint &, int ) ),SLOT( slotEdit() ) );

	mWidget->btnEdit->setEnabled( false );
	mWidget->btnDelete->setEnabled( false );
	// Disabled pending implementation
	//mWidget->btnUp->setEnabled( false );
	//mWidget->btnDown->setEnabled( false );

	mProtocols = KPluginInfo::fromServices( KTrader::self()->query( QString::fromLatin1( "KABC/IMProtocol" ) ) );
	//kdDebug ( 5720 ) << " found " << mProtocols.count() << " protocols " << endl;
}

QValueList<KPluginInfo *> IMEditorWidget::availableProtocols() const
{
	return mProtocols;
}

void IMEditorWidget::loadContact( KABC::Addressee *addr )
{
	if ( mWidget->lvAddresses ) 
		mWidget->lvAddresses->clear();

	// see README for details of how Evolution stores IM addresses (differently)
	QStringList customs = addr->customs();

	QStringList::ConstIterator it;
	for ( it = customs.begin(); it != customs.end(); ++it )
	{
		QString app, name, value;
		splitField( *it, app, name, value );

		if ( app.startsWith( QString::fromLatin1( "messaging/" ) ) )
		{
			if ( name == QString::fromLatin1( "All" ) )
			{
				KPluginInfo *protocol = protocolFromString( app );
				if ( protocol )
				{
					QStringList addresses = QStringList::split( QChar( 0xE000 ), value );
					QStringList::iterator end = addresses.end();
					for ( QStringList::iterator it = addresses.begin(); it != end; ++it )	
					{
						new IMAddressLVI( mWidget->lvAddresses, protocol, *it, Any/*, false*/ );
					}
				}
				else
					kdDebug( 5720 ) << k_funcinfo << " no protocol found for: " << app << endl;
			}
		}
	}
}

void IMEditorWidget::storeContact( KABC::Addressee *addr )
{
	// for each changed protocol, write a new custom field containing the current set of
	// addresses
	QValueList<KPluginInfo *>::iterator protocolIt;
	for ( protocolIt = mChangedProtocols.begin(); protocolIt != mChangedProtocols.end(); ++protocolIt )
	{
		QStringList lst;
		QListViewItemIterator addressIt( mWidget->lvAddresses );
		while ( addressIt.current() )
		{
			IMAddressLVI* currentAddress = static_cast<IMAddressLVI*>(*addressIt);
			if ( currentAddress->protocol() == *protocolIt )
				lst.append( currentAddress->address() );
			++addressIt;
		}

		//kdDebug( 0 ) << QString::fromLatin1("messaging/%1").arg( protocolToString( *protocolIt ) ) <<
		//						QString::fromLatin1("All") <<
		//					lst.join( QChar( 0xE000 ) ) << endl;
		
		QString addrBookField = ( *protocolIt )->property( "X-KDE-InstantMessagingKABCField" ).toString();
		if ( !lst.isEmpty() )
			addr->insertCustom( addrBookField, QString::fromLatin1( "All" ), lst.join( QChar( 0xE000 ) ) );
		else
			addr->removeCustom( addrBookField, QString::fromLatin1("All") );
	}
}

void IMEditorWidget::setReadOnly( bool readOnly )
{
	mReadOnly = readOnly;

	mWidget->btnAdd->setEnabled( !readOnly );
	mWidget->btnEdit->setEnabled( !readOnly && mWidget->lvAddresses->currentItem() );
	mWidget->btnDelete->setEnabled( !readOnly && mWidget->lvAddresses->currentItem() );
}

void IMEditorWidget::slotUpdateButtons()
{
	int num_selected = mWidget->lvAddresses->selectedItems().count(); 
	if ( !mReadOnly && num_selected == 1)
	{
		//mWidget->btnAdd->setEnabled( true );
		mWidget->btnEdit->setEnabled( true );
		mWidget->btnDelete->setEnabled( true );
	} else if(!mReadOnly && num_selected > 1) {	
		mWidget->btnEdit->setEnabled( false );
		mWidget->btnDelete->setEnabled( true );
	}
	else
	{
		//mWidget->btnAdd->setEnabled( false );
		mWidget->btnEdit->setEnabled( false );
		mWidget->btnDelete->setEnabled( false );
	}
}

void IMEditorWidget::slotAdd()
{
	KDialogBase *addDialog = new KDialogBase( this, "addaddress", true, i18n("Add Address"), KDialogBase::Ok|KDialogBase::Cancel );
	IMAddressWidget *addressWid = new IMAddressWidget( addDialog, mProtocols );
	addDialog->setMainWidget( addressWid );
	if ( addDialog->exec() == QDialog::Accepted )
	{
		// add the new item
		new IMAddressLVI( mWidget->lvAddresses, addressWid->protocol(), addressWid->address() /*, addressWid->context() */ );
		if ( mChangedProtocols.find( addressWid->protocol() ) == mChangedProtocols.end() )
			mChangedProtocols.append( addressWid->protocol() );
		mWidget->lvAddresses->sort();

		setModified( true );
	}
	delete addDialog;
}

void IMEditorWidget::slotEdit()
{
	QListViewItemIterator it( mWidget->lvAddresses, QListViewItemIterator::Selected );
	if ( IMAddressLVI *current = static_cast<IMAddressLVI*>(it.current() ) ) //just edit the first one selected.
	{
		KDialogBase *editDialog = new KDialogBase( this, "editaddress", true, i18n("Edit Address"), KDialogBase::Ok|KDialogBase::Cancel );
		IMAddressWidget *addressWid = new IMAddressWidget( editDialog, mProtocols, current->protocol(), current->address(), current->context() ) ;

		editDialog->setMainWidget( addressWid );

		if ( editDialog->exec() == QDialog::Accepted )
		{
			bool modified = false;
			if(addressWid->address() != current->address()) {
				modified = true;
				current->setAddress( addressWid->address() );
			}
			if(addressWid->context() != current->context()) {
				modified = true;
				current->setContext( addressWid->context() );
			}

			// the entry for the protocol of the current address has changed
			if ( mChangedProtocols.find( current->protocol() ) == mChangedProtocols.end() ) {
				mChangedProtocols.append( current->protocol() );
			}
			// update protocol - has another protocol gained an address?
			if ( current->protocol() != addressWid->protocol() )
			{
				modified = true;
				// this proto is losing an entry
				current->setProtocol( addressWid->protocol() );
				if ( mChangedProtocols.find( current->protocol() ) == mChangedProtocols.end() )
					mChangedProtocols.append( current->protocol() );
			}

			if(modified) setModified(true);
		}
                delete editDialog;
	}
}

void IMEditorWidget::slotDelete()
{
	int num_selected = 0;
	{
		QListViewItemIterator it( mWidget->lvAddresses, QListViewItemIterator::Selected );
		while ( it.current() ) {
			num_selected++;
			++it;
	        }
	}
	if(num_selected == 0)
		return;
	if(KMessageBox::warningContinueCancel( this, i18n("Do you really want to remove the selected address?", "Do you really want to remove the %n selected addresses?", num_selected), i18n("Confirm Remove"), KGuiItem(i18n("&Remove"),"editdelete") ) != KMessageBox::Continue )
		return;
	
	QListViewItemIterator it( mWidget->lvAddresses);
        while(it.current())
	{
		if(it.current()->isSelected()) {
			IMAddressLVI * current = static_cast<IMAddressLVI*>( *it );
			if ( mChangedProtocols.find( current->protocol() ) == mChangedProtocols.end() )
			{
				mChangedProtocols.append( current->protocol() );
				//kdDebug ( 0 ) << " changed protocols:  " << mProtocols.count() << endl;
			}
			delete current;
		} else
			++it;
	}
	setModified( true );
}

KPluginInfo * IMEditorWidget::protocolFromString( QString fieldValue )
{
	QValueList<KPluginInfo *>::ConstIterator it;
	KPluginInfo * protocol = 0;
	for ( it = mProtocols.begin(); it != mProtocols.end(); ++it )
	{
		if ( ( (*it)->property( "X-KDE-InstantMessagingKABCField" ).toString() == fieldValue ) )
		{
			protocol = *it;
			break;
		}
	}
	return protocol;
}

void IMEditorWidget::splitField( const QString &str, QString &app, QString &name, QString &value )
{
  int colon = str.find( ':' );
  if ( colon != -1 ) {
    QString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    int dash = tmp.find( '-' );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}

#include "imeditorwidget.moc"
