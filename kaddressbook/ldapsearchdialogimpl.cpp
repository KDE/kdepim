/* ldapsearchdialogimpl.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <qlistview.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <qvaluelist.h> 
#include <qapplication.h>

#include "ldapsearchdialogimpl.h"
#include "viewmanager.h"

static QString join( const KABC::LdapAttrValue& lst, const QString& sep )
{
  QString res;
  bool alredy = false;
  for ( KABC::LdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( alredy )
      res += sep;
    alredy = TRUE;
    res += QString::fromUtf8( *it );
  }
  return res;
}

static QMap<QString, QString>& adrbookattr2ldap()
{
  static QMap<QString, QString> keys;

  if ( keys.isEmpty() ) {
    keys[ i18n( "Title" ) ] = "title";
    keys[ i18n( "Full Name" ) ] = "cn";
    keys[ i18n( "Email" ) ] = "mail";
    keys[ i18n( "Phone Number" ) ] = "telephoneNumber";
    keys[ i18n( "Mobile Number" ) ] = "mobile";
    keys[ i18n( "Fax Number" ) ] = "facsimileTelephoneNumber";
    keys[ i18n( "Pager" ) ] = "pager";
    keys[ i18n( "Street") ] = "street";
    keys[ i18n( "State" ) ] = "st";
    keys[ i18n( "Country" ) ] = "co";
    keys[ i18n( "Locality" ) ] = "l";
    keys[ i18n( "Organization" ) ] = "o";
    keys[ i18n( "Company" ) ] = "Company";
    keys[ i18n( "Department" ) ] = "department";
    keys[ i18n( "Postal Code" ) ] = "postalCode";
    keys[ i18n( "Postal Address" ) ] = "postalAddress";
    keys[ i18n( "Description" ) ] = "description";
    keys[ i18n( "User ID" ) ] = "uid";
  }
  return keys;
}

class ContactListItem : public QListViewItem
{
  public:
    ContactListItem( QListView* parent, const KABC::LdapAttrMap& attrs )
      : QListViewItem( parent ), mAttrs( attrs )
    { }

    KABC::LdapAttrMap mAttrs;

    virtual QString text( int col ) const
    {
      // Look up a suitable attribute for column col
      QString colName = listView()->columnText( col );
      return join( mAttrs[ adrbookattr2ldap()[ colName ] ], ", " );
    }
};

LDAPSearchDialogImpl::LDAPSearchDialogImpl( KABC::AddressBook *ab, QWidget* parent, const char* name, bool modal, WFlags fl )
  : LDAPSearchDialog( parent, name, modal, fl ), mAddressBook( ab )
{
  mNumHosts = 0;
  mIsOK = false;

  filterCombo->insertItem( i18n( "Name" ) );
  filterCombo->insertItem( i18n( "Email" ) );
  filterCombo->insertItem( i18n( "Phone Number" ) );
 
  resultListView->setSelectionMode( QListView::Multi );
  resultListView->setAllColumnsShowFocus( true );
  resultListView->setShowSortIndicator( true );

  connect( recursiveCheckbox, SIGNAL( toggled( bool ) ),
	   this, SLOT( slotSetScope( bool ) ) );
  connect( addSelectedButton, SIGNAL( clicked() ),
	   this, SLOT( slotAddSelectedContacts() ) );
  connect( selectAllButton, SIGNAL( clicked() ),
	   this, SLOT( slotSelectAll() ) );
  connect( unselectAllButton, SIGNAL( clicked() ),
	   this, SLOT( slotUnSelectAll() ) );
  connect( mailToButton, SIGNAL( clicked() ),
	   this, SLOT( slotSendMail() ) );
  connect( searchButton, SIGNAL( clicked() ),
	   this, SLOT( slotStartSearch() ) );

  rereadConfig();
}

void LDAPSearchDialogImpl::rereadConfig()
{
  // Create one KABC::LdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at 
  // the beginning of the process
  mLdapClientList.setAutoDelete( true );
  mLdapClientList.clear();

  // then read the config file and register all selected 
  // server in the list
  KConfig* config = ViewManager::config();
  config->setGroup( "LDAP" );
  mNumHosts = config->readUnsignedNumEntry( "NumSelectedHosts" ); 
  if ( !mNumHosts ) {
    KMessageBox::error( this, i18n( "You must select a LDAP server before searching.\nYou can do this from the menu Settings/Configure KAddressBook." ) );
    mIsOK = false;
  } else {
    mIsOK = true;
    for ( int j = 0; j < mNumHosts; ++j ) {
      KABC::LdapClient* ldapClient = new KABC::LdapClient( this, "ldapclient" );
    
      QString host =  config->readEntry( QString( "SelectedHost%1" ).arg( j ), "" );
      if ( !host.isEmpty() )
        ldapClient->setHost( host );

      QString port = QString::number( config->readUnsignedNumEntry( QString( "SelectedPort%1" ).arg( j ) ) );
      if ( !port.isEmpty() )
        ldapClient->setPort( port );

      QString base = config->readEntry( QString( "SelectedBase%1" ).arg( j ), "" );
      if ( !base.isEmpty() )
        ldapClient->setBase( base );

      QStringList attrs;

      for ( QMap<QString,QString>::Iterator it = adrbookattr2ldap().begin(); it != adrbookattr2ldap().end(); ++it )
        attrs << *it;

      ldapClient->setAttrs( attrs );

      connect( ldapClient, SIGNAL( result( const KABC::LdapObject& ) ),
	       this, SLOT( slotAddResult( const KABC::LdapObject& ) ) );
      connect( ldapClient, SIGNAL( done() ),
	       this, SLOT( slotSearchDone() ) ); 
      connect( ldapClient, SIGNAL( error( const QString& ) ),
	       this, SLOT( slotError( const QString& ) ) );

      mLdapClientList.append( ldapClient );     
    }

/** CHECKIT*/
    while ( resultListView->header()->count() > 0 ) {
      resultListView->removeColumn(0);
    }

    resultListView->addColumn( i18n( "Full Name" ) );
    resultListView->addColumn( i18n( "Email" ) );
    resultListView->addColumn( i18n( "Phone Number" ) );
    resultListView->addColumn( i18n( "Mobile Number" ) );
    resultListView->addColumn( i18n( "Fax Number" ) );
    resultListView->addColumn( i18n( "Company" ) );
    resultListView->addColumn( i18n( "Organization" ) );
    resultListView->addColumn( i18n( "Street" ) );
    resultListView->addColumn( i18n( "State" ) );
    resultListView->addColumn( i18n( "Country" ) );
    resultListView->addColumn( i18n( "Postal Code" ) );
    resultListView->addColumn( i18n( "Postal Address" ) );
    resultListView->addColumn( i18n( "Locality" ) );
    resultListView->addColumn( i18n( "Department" ) );
    resultListView->addColumn( i18n( "Description" ) );
    resultListView->addColumn( i18n( "User ID" ) );
    resultListView->addColumn( i18n( "Title" ) );

    resultListView->clear();
  }
}

LDAPSearchDialogImpl::~LDAPSearchDialogImpl()
{
}

void LDAPSearchDialogImpl::cancelQuery()
{
  for ( KABC::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    client->cancelQuery();
  }
}

void LDAPSearchDialogImpl::slotAddResult( const KABC::LdapObject& obj )
{
  new ContactListItem( resultListView, obj.attrs );
}

void LDAPSearchDialogImpl::slotSetScope( bool rec )
{
  for ( KABC::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    if ( rec )
      client->setScope( "sub" );
    else
      client->setScope( "one" );  
  }
}

QString LDAPSearchDialogImpl::makeFilter( const QString& query, const QString& attr )
{
  QString result;

  if ( query.isEmpty() )
    result = "%1=*%2";
  else
    result = "%1=*%2*";

  if ( attr == i18n( "Name" ) ) {
    result = result.arg( "cn" ).arg( query );
  } else if ( attr == i18n( "Email" ) ) {
    result = result.arg( "mail" ).arg( query );
  } else if ( attr == i18n( "Phone Number" ) ) {
    result = result.arg( "telephoneNumber" ).arg( query );
  } else {
    // Error?
    result = QString::null;
  }
  return result;
}

void LDAPSearchDialogImpl::slotStartSearch()
{
  cancelQuery();

  QApplication::setOverrideCursor( Qt::waitCursor );
  searchButton->setText( i18n( "Stop" ) );

  disconnect( searchButton, SIGNAL( clicked() ),
              this, SLOT( slotStartSearch() ) );
  connect( searchButton, SIGNAL( clicked() ),
           this, SLOT( slotStopSearch() ) );

  QString filter = makeFilter( searchEdit->text().stripWhiteSpace(), filterCombo->currentText() );

   // loop in the list and run the KABC::LdapClients 
  resultListView->clear();
  for( KABC::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    client->startQuery( filter );
  }
}

void LDAPSearchDialogImpl::slotStopSearch()
{
  cancelQuery();
  slotSearchDone();
}

void LDAPSearchDialogImpl::slotSearchDone()
{
  // If there are no more active clients, we are done.
  for ( KABC::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    if ( client->isActive() )
      return;
  }

  disconnect( searchButton, SIGNAL( clicked() ),
              this, SLOT( slotStopSearch() ) );
  connect( searchButton, SIGNAL( clicked() ),
           this, SLOT( slotStartSearch() ) );

  searchButton->setText( i18n( "Search" ) );
  QApplication::restoreOverrideCursor();
}

void LDAPSearchDialogImpl::slotError( const QString& error )
{
  QApplication::restoreOverrideCursor();
  KMessageBox::error( this, error );
}

void LDAPSearchDialogImpl::closeEvent( QCloseEvent* e )
{
  slotStopSearch();
  e->accept();
}

void LDAPSearchDialogImpl::slotAddSelectedContacts()
{
  ContactListItem* cli = static_cast<ContactListItem*>( resultListView->firstChild() );
  while ( cli ) {
    if ( cli->isSelected() ) {
      KABC::Addressee addr;

      // name
      addr.setNameFromString( QString::fromUtf8( cli->mAttrs["cn"].first() ) );

      // email
      KABC::LdapAttrValue lst = cli->mAttrs["mail"];
      KABC::LdapAttrValue::ConstIterator it = lst.begin();
      bool pref = true;
      if ( it != lst.end() ) {
        addr.insertEmail( QString::fromUtf8( *it ), pref );
        pref = false;
        ++it;
      }

      addr.setOrganization(QString::fromUtf8( cli->mAttrs[ "o" ].first() ) );
      if (addr.organization().isEmpty())
         addr.setOrganization(QString::fromUtf8( cli->mAttrs[ "Company" ].first() ) );

      addr.insertCustom("KADDRESSBOOK", "X-Department", QString::fromUtf8( cli->mAttrs[ "department" ].first() ) );

      // Address
      KABC::Address workAddr(KABC::Address::Work);

      workAddr.setStreet(QString::fromUtf8( cli->mAttrs[ "street" ].first()) );
      workAddr.setLocality(QString::fromUtf8( cli->mAttrs[ "l" ].first()) );
    workAddr.setRegion(QString::fromUtf8(  cli->mAttrs[ "address" ].first()));
      workAddr.setPostalCode(QString::fromUtf8( cli->mAttrs[ "postalCode" ].first()) );
      workAddr.setCountry(QString::fromUtf8( cli->mAttrs[ "co" ].first()) );

      addr.insertAddress( workAddr );

      // phone

      KABC::PhoneNumber telNr = QString::fromUtf8( cli->mAttrs[  "telephoneNumber" ].first() );
      telNr.setType(KABC::PhoneNumber::Work);
      addr.insertPhoneNumber(telNr);

      KABC::PhoneNumber faxNr = QString::fromUtf8( cli->mAttrs[  "facsimileTelephoneNumber" ].first() );
      faxNr.setType(KABC::PhoneNumber::Fax);
      addr.insertPhoneNumber(faxNr);

      KABC::PhoneNumber cellNr = QString::fromUtf8( cli->mAttrs[  "mobile" ].first() );
      cellNr.setType(KABC::PhoneNumber::Cell);
      addr.insertPhoneNumber(cellNr);

      KABC::PhoneNumber pagerNr = QString::fromUtf8( cli->mAttrs[  "pager" ].first() );
      pagerNr.setType(KABC::PhoneNumber::Pager);
      addr.insertPhoneNumber(pagerNr);

      if ( mAddressBook )
        mAddressBook->insertAddressee( addr );
    }

    cli = static_cast<ContactListItem*>( cli->nextSibling() );
  }

  emit addresseesAdded();
}

/*!
 * Returns a ", " separated list of email addresses that were
 * checked by the user
 */
QString LDAPSearchDialogImpl::selectedEMails() const
{
  QStringList result;
  ContactListItem* cli = static_cast<ContactListItem*>( resultListView->firstChild() );
  while ( cli ) {
    if ( cli->isSelected() ) {
      QString email = QString::fromUtf8( cli->mAttrs[ "mail" ].first() ).stripWhiteSpace();
      if ( !email.isEmpty() ) {
        QString name = QString::fromUtf8( cli->mAttrs[ "cn" ].first() ).stripWhiteSpace();
        if ( name.isEmpty() ) {
          result << email;
        } else {
          result << name + " <" + email + ">";
        }
      }
    }
    cli = static_cast<ContactListItem*>( cli->nextSibling() );
  }

  return result.join( ", " );
}

void LDAPSearchDialogImpl::slotSendMail()
{
  kapp->invokeMailer( selectedEMails(), "" );
}

void LDAPSearchDialogImpl::slotSelectAll()
{
  resultListView->selectAll( true );
}

void LDAPSearchDialogImpl::slotUnSelectAll()
{
  resultListView->selectAll( false );
}

#include "ldapsearchdialogimpl.moc"
