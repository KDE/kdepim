/*    
	imeditorwidget.cpp
	
	IM address editor widget for KAddressBook
	
	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlistview.h>
#include <qstringlist.h>

#include <kdialogbase.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

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
  return "IM Addresses";
}

QString IMEditorWidgetFactory::pageIdentifier() const
{
  return "instantmessaging";
}

/*===========================================================================*/

IMAddressLVI::IMAddressLVI( KListView *parent, IMProtocol protocol, QString address, IMContext context/*, bool inVCard */) : KListViewItem( parent )
{
	//mInVCard = inVCard;
	
	setProtocol( protocol );
	
	// set address
	setText( 2, address );
	
	// set context
	setContext( context );
	
	// set indicator that this field is saved in the vCard
	//if ( inVCard )
	//	setPixmap( 0, SmallIcon( QString::fromLatin1( "ok" ) ) );
}

/*
void IMAddressLVI::setInVCard( bool inVCard )
{
	if ( inVCard )
		setPixmap( 0, SmallIcon( QString::fromLatin1( "ok" ) ) );
	else
		setPixmap( 0, QPixmap() );
	mInVCard = inVCard;
}
*/

void IMAddressLVI::setAddress( QString address )
{
	setText( 2, address );
}

void IMAddressLVI::setContext( IMContext context )
{
	mContext = context;
	// set context
	switch ( context )
	{
	case Home:
		setText( 3, i18n( "Home" ) );
		break;
	case Work:
		setText( 3, i18n( "Work" ) );
		break;
	case Any:
		setText( 3, i18n( "Any" ) );
		break;
	}
}

void IMAddressLVI::setProtocol( IMProtocol protocol )
{
	mProtocol = protocol;
	switch ( protocol )
	{
	case AIM:
		setText( 1, QString::fromLatin1( "AIM" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("aim_protocol") ) );
		break;
	case GaduGadu:
		setText( 1, QString::fromLatin1( "Gadu-Gadu" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("gadu_protocol") ) );
		break;
	case Jabber:
		setText( 1, QString::fromLatin1( "Jabber" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("jabber_protocol") ) );
		break;
	case ICQ:
		setText( 1, QString::fromLatin1( "ICQ" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("icq_protocol") ) );
		break;
	case IRC:
		setText( 1, QString::fromLatin1( "IRC" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("irc_protocol") ) );
		break;
	case MSN:
		setText( 1, QString::fromLatin1( "MSN" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("msn_protocol") ) );
		break;
	case SMS:
		setText( 1, QString::fromLatin1( "SMS" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("sms_protocol") ) );
		break;
	case Yahoo:
		setText( 1, QString::fromLatin1( "Yahoo" ) );
		setPixmap( 1, SmallIcon(QString::fromLatin1("yahoo_protocol") ) );
		break;
	default:
		setText( 1, i18n( "Unknown" ) );
	}
}

/*
bool IMAddressLVI::inVCard()
{
	return mInVCard;
}
*/

IMProtocol IMAddressLVI::protocol()
{
	return mProtocol;
}

IMContext IMAddressLVI::context()
{
	return mContext;
}
 
QString IMAddressLVI::address()
{
	return text( 2 );
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
	
	mWidget->btnEdit->setEnabled( false );
	mWidget->btnDelete->setEnabled( false );
	// Disabled pending implementation
	mWidget->btnUp->setEnabled( false );
	mWidget->btnDown->setEnabled( false );
}

void IMEditorWidget::loadContact( KABC::Addressee *addr )
{
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
				// Get the protocol from the custom field
				// by chopping the 'messaging/' prefix from the custom field app name
				QString protocolName = app.right( app.length() - 10 );
				
				IMProtocol protocol = protocolFromString( protocolName );
				
				QStringList addresses = QStringList::split( QChar( 0xE000 ), value );
				QStringList::iterator end = addresses.end();
				for ( QStringList::iterator it = addresses.begin(); it != end; ++it )
				{
					new IMAddressLVI( mWidget->lvAddresses, protocol, *it, Any/*, false*/ );
				}
			}
		}
	}
}

void IMEditorWidget::storeContact( KABC::Addressee *addr )
{
	kdDebug( 0 ) << k_funcinfo << endl;
	// Write all protocols, or track changed ones and just write them?
	kdDebug( 0 ) << mChangedProtocols.count() << " Changed protocols are\n";
	QValueList<IMProtocol>::iterator protocolIt;
    for ( protocolIt = mChangedProtocols.begin(); protocolIt != mChangedProtocols.end(); ++protocolIt )
		kdDebug( 0 ) << *protocolIt << ", \n" ;
		
	kdDebug( 0 ) << endl;
	
	// for each changed protocol, write a new custom field containing the current set of 
	// addresses
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
		
		kdDebug( 0 ) << QString::fromLatin1("messaging/%1").arg( protocolToString( *protocolIt ) ) <<
							QString::fromLatin1("All") <<
							lst.join( QChar( 0xE000 ) ) << endl;
		if ( lst.count() > 0 )
			addr->insertCustom( QString::fromLatin1("messaging/%1").arg( protocolToString( *protocolIt ) ),
							QString::fromLatin1("All"), 
							lst.join( QChar( 0xE000 ) ) );
		else
			addr->removeCustom( QString::fromLatin1("messaging/%1").arg( protocolToString( *protocolIt ) ),
								QString::fromLatin1("All") );
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
	
	if ( !mReadOnly && mWidget->lvAddresses->selectedItem() )
	{
		//mWidget->btnAdd->setEnabled( true );
		mWidget->btnEdit->setEnabled( true );
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
	kdDebug( 0 ) << k_funcinfo << endl;
	KDialogBase *addDialog = new KDialogBase( this, "addaddress", true, i18n("Add Address"), KDialogBase::Ok|KDialogBase::Cancel );
	IMAddressWidget *addressWid = new IMAddressWidget( addDialog );
	addDialog->setMainWidget( addressWid );
	if ( addDialog->exec() == QDialog::Accepted )
	{
		/*
		// disable vcard flag on each other item
		if ( addressWid->inVCard() )
		{
			QListViewItemIterator it( mWidget->lvAddresses );
			while ( it.current() )
			{
				(static_cast<IMAddressLVI*>(*it))->setInVCard( false );
				++it;
			}
		}
		*/
		
		// add the new item
		new IMAddressLVI( mWidget->lvAddresses, addressWid->protocol(), addressWid->address(), addressWid->context()/*, addressWid->inVCard()*/ );
		if ( mChangedProtocols.find( addressWid->protocol() ) == mChangedProtocols.end() )
			mChangedProtocols.append( addressWid->protocol() );
		mWidget->lvAddresses->sort();
		
		setModified( true );
	}
}

void IMEditorWidget::slotEdit()
{
	kdDebug( 0 ) << k_funcinfo << endl;
	if ( IMAddressLVI *current = static_cast<IMAddressLVI*>(mWidget->lvAddresses->selectedItem() ) )
	{
		KDialogBase *editDialog = new KDialogBase( this, "editaddress", true, i18n("Edit Address"), KDialogBase::Ok|KDialogBase::Cancel );
		IMAddressWidget *addressWid = new IMAddressWidget( editDialog, current->protocol(), current->address(), current->context()/*, current->inVCard()*/ ) ;
		
		editDialog->setMainWidget( addressWid );
		
		if ( editDialog->exec() == QDialog::Accepted )
		{
			/*
			if ( addressWid->inVCard() )
			{
				QListViewItemIterator it( mWidget->lvAddresses );
				while ( it.current() )
				{
					(static_cast<IMAddressLVI*>(*it))->setInVCard( false );
					++it;
				}
			}
			*/
			current->setAddress( addressWid->address() );
			//current->setInVCard( addressWid->inVCard() );
			current->setContext( addressWid->context() );
			
			// the entry for the protocol of the current address has changed
			if ( mChangedProtocols.find( current->protocol() ) == mChangedProtocols.end() )
				mChangedProtocols.append( current->protocol() );
			// update protocol - has another protocol gained an address?
			if ( current->protocol() != addressWid->protocol() )
			{
				// this proto is losing an entry
				current->setProtocol( addressWid->protocol() );
				if ( mChangedProtocols.find( current->protocol() ) == mChangedProtocols.end() )
					mChangedProtocols.append( current->protocol() );
			}
			
			setModified( true );
		}
	}
}

void IMEditorWidget::slotDelete()
{
	kdDebug( 0 ) << k_funcinfo << endl;
	if ( mWidget->lvAddresses->selectedItem() && KMessageBox::questionYesNo( this, i18n("Do you really want to delete the selected address?"), i18n("Confirm delete") ) == KMessageBox::Yes  )
	{
		IMAddressLVI * current = static_cast<IMAddressLVI*>( mWidget->lvAddresses->selectedItem() );
		if ( mChangedProtocols.find( current->protocol() ) == mChangedProtocols.end() )
			mChangedProtocols.append( current->protocol() );
		delete current;
		
		setModified( true );
	}
}

IMProtocol IMEditorWidget::protocolFromString( QString protocolName )
{
	IMProtocol protocol = Unknown;
	
	if ( protocolName == QString::fromLatin1( "aim") )
		protocol = AIM;
	else if ( protocolName == QString::fromLatin1( "gadu") )
		protocol = GaduGadu;
	else if ( protocolName == QString::fromLatin1( "xmpp") )
		protocol = Jabber;
	else if ( protocolName == QString::fromLatin1( "icq") )
		protocol = ICQ;
	else if ( protocolName == QString::fromLatin1( "irc") )
		protocol = IRC;
	else if ( protocolName == QString::fromLatin1( "msn") )
		protocol = MSN;
	else if ( protocolName == QString::fromLatin1( "sms") )
		protocol = SMS;
	else if ( protocolName == QString::fromLatin1( "yahoo") )
		protocol = Yahoo;
	
	return protocol;
}

QString IMEditorWidget::protocolToString( IMProtocol protocol )
{
	QString protocolName;
	switch ( protocol )
	{
	case AIM:
		protocolName = QString::fromLatin1( "aim" );
		break;
	case GaduGadu:
		protocolName = QString::fromLatin1( "gadu" );
		break;
	case Jabber:
		protocolName = QString::fromLatin1( "xmpp" );
		break;
	case ICQ:
		protocolName = QString::fromLatin1( "icq" );
		break;
	case IRC:
		protocolName = QString::fromLatin1( "irc" );
		break;
	case MSN:
		protocolName = QString::fromLatin1( "msn" );
		break;
	case SMS:
		protocolName = QString::fromLatin1( "sms" );
		break;
	case Yahoo:
		protocolName = QString::fromLatin1( "yahoo" );
		break;
	case Unknown:
		break;
	}
	return protocolName;
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
