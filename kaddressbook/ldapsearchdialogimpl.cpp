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

static QString join( const KLdapAttrValue& lst, const QString& sep )
{
  QString res;
  bool alredy = FALSE;
  for ( KLdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( alredy )
      res += sep;
    alredy = TRUE;
    res += QString::fromUtf8(*it);
  }
  return res;
}

static QMap<QString,QString>& adrbookattr2ldap() {
  static QMap<QString,QString> keys;
  if( keys.isEmpty() ) {
    keys[i18n("Full Name")] = "cn";
    keys[i18n("File As")] = "cn";
    keys[i18n("Email")] = "mail";
    keys[i18n("Home Phone")] = "telephoneNumber";
    keys[i18n("Business Phone")] = "telephoneNumber";
    keys[i18n("State")] = "st";
    keys[i18n("Country")] = "c";
  }  
  return keys;
}

class ContactListItem : public QListViewItem {
public:
  ContactListItem( QListView* parent, const KLdapAttrMap& attrs )
    : QListViewItem( parent), _attrs(attrs) {}
  KLdapAttrMap _attrs;

  virtual QString text( int col ) const {
    // Look up a suitable attribute for column col
    QString colName = listView()->columnText(col);
    return join( _attrs[adrbookattr2ldap()[colName]], ", " );
  }
};

/* 
 *  Constructs a LDAPSearchDialogImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
LDAPSearchDialogImpl::LDAPSearchDialogImpl( KABC::AddressBook *ab, QWidget* parent, const char* name, bool modal, WFlags fl )
  : LDAPSearchDialog( parent, name, modal, fl ), mAddressBook( ab )
{
  numHosts = 0;
  bOK = false;

  filterCombo->insertItem(i18n("Name"));
  filterCombo->insertItem(i18n("Email"));
  filterCombo->insertItem(i18n("Phone Number"));
 
  resultListView->setSelectionMode( QListView::Multi );
  resultListView->setAllColumnsShowFocus( true );
  resultListView->setShowSortIndicator( true );

  connect( recursiveCheckbox, SIGNAL( toggled( bool ) ),
	   this, SLOT( slotSetScope( bool ) ) );
  connect( addSelectedButton, SIGNAL( clicked() ),
	   this, SLOT( slotAddSelectedContacts() ) );
  connect( mailToButton, SIGNAL( clicked() ),
	   this, SLOT( slotSendMail() ) );
  connect( searchButton, SIGNAL( clicked() ),
	   this, SLOT( slotStartSearch() ) );

  rereadConfig();
}

void LDAPSearchDialogImpl::rereadConfig()
{
    // Create one KLdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at 
  // the beginning of the process
  ldapclientlist.setAutoDelete(TRUE);
  ldapclientlist.clear();

  // then read the config file and register all selected 
  // server in the list
  KConfig* config = kapp->config();
  config->setGroup("LDAP");
  numHosts = config->readUnsignedNumEntry( "NumSelectedHosts"); 
  if (!numHosts) {
    KMessageBox::error( this, i18n( "You must select an address before searching.\nYou can do this from the menu Settings/Configure KAddressBook." ) );
    bOK = false;
  } else {
    bOK = true;
    for ( int j = 0; j < numHosts; j++ ) {
      KLdapClient* ldapClient = new KLdapClient( this, "ldapclient" );
    
      QString host =  config->readEntry( QString( "SelectedHost%1" ).arg(j), "" ).stripWhiteSpace();
      if ( host != "" ){
        ldapClient->setHost( host );
      }
      QString port = QString::number(config->readUnsignedNumEntry(QString( "SelectedPort%1" ).arg(j)));
      if(!port.isEmpty() ) {
        ldapClient->setPort( port );
      }
      QString base = config->readEntry(QString( "SelectedBase%1" ).arg(j), "" ).stripWhiteSpace();
      if ( base != "" ){
        ldapClient->setBase( base );
      }

      QStringList attrs;
      for( QMap<QString,QString>::Iterator it = adrbookattr2ldap().begin(); it != adrbookattr2ldap().end(); ++it) {
        attrs << *it;
      }
      ldapClient->setAttrs( attrs );

      connect( ldapClient, SIGNAL( result( const KLdapObject& ) ),
	       this, SLOT( slotAddResult( const KLdapObject& ) ) );
      connect( ldapClient, SIGNAL( done() ),
	       this, SLOT( slotSearchDone() ) ); 
      connect( ldapClient, SIGNAL( error( const QString& ) ),
	       this, SLOT( slotError( const QString& ) ) );

      //_ldapClient->setHost("sphinx500.bsi.bund.de");
      //_ldapClient->setHost( "ldap.bigfoot.com" );
      //_ldapClient->setBase( "dc=klaralvdalens-datakonsult,dc=se" );
      //_ldapClient->setBase(""); 
      ldapclientlist.append( ldapClient );     
    }

    while( resultListView->header()->count() > 0 ) {
      resultListView->removeColumn(0);
    }

// Disabled because of restructuring of views.
#if 0
    QStringList* field = _adrBook->fields();
    for ( uint i = 0; i < field->count(); i++ )
      resultListView->addColumn( Attributes::instance()->fieldToName( (*field)[i] ) );
#endif
    
    resultListView->addColumn( i18n("Full Name") );
    resultListView->addColumn( i18n("Email") );

    resultListView->clear();
  }
}

/*  
 *  Destroys the object and frees any allocated resources
 */
LDAPSearchDialogImpl::~LDAPSearchDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

void LDAPSearchDialogImpl::cancelQuery()
{
  for( KLdapClient* client = ldapclientlist.first(); client; client = ldapclientlist.next() ) {
    client->cancelQuery();
  }
}

void LDAPSearchDialogImpl::slotAddResult( const KLdapObject& obj )
{
  new ContactListItem( resultListView, obj.attrs );
}

void LDAPSearchDialogImpl::slotSetScope( bool rec )
{
  for( KLdapClient* client = ldapclientlist.first(); client; client = ldapclientlist.next() ) {
    if( rec ) client->setScope( "sub" );
    else client->setScope( "one" );  
  }
}

QString LDAPSearchDialogImpl::makeFilter( const QString& query, const QString& attr )
{
  QString result;
  if( query.isEmpty() ) result = "%1=*%2";
  else result = "%1=*%2*";
  if( attr == i18n("Name") ) {
    result = result.arg( "cn" ).arg( query );
  } else if( attr == i18n( "Email" ) ) {
    result = result.arg( "mail" ).arg( query );
  } else if( attr == i18n( "Phone Number" ) ) {
    result = result.arg( "telePhonenumber" ).arg( query );
  } else {
    // Error?
    result = QString::null;
  }
  return result;
}

void LDAPSearchDialogImpl::slotStartSearch()
{
  cancelQuery();
  //closeButton->setEnabled( false );
  QApplication::setOverrideCursor( Qt::waitCursor );
  searchButton->setText( i18n("Stop" ) );
  disconnect( searchButton, SIGNAL( clicked() ),
	      this, SLOT( slotStartSearch() ) );
  connect( searchButton, SIGNAL( clicked() ),
	   this, SLOT( slotStopSearch() ) );

  QString filter = makeFilter( searchEdit->text().stripWhiteSpace(), filterCombo->currentText() );

   // loop in the list and run the KldapClients 
  resultListView->clear();
  for( KLdapClient* client = ldapclientlist.first(); client; client = ldapclientlist.next() ) {
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
  for( KLdapClient* client = ldapclientlist.first(); client; client = ldapclientlist.next() ) {
    if( client->isActive() ) return;
  }
  disconnect( searchButton, SIGNAL( clicked() ),
	      this, SLOT( slotStopSearch() ) );
  connect( searchButton, SIGNAL( clicked() ),
	   this, SLOT( slotStartSearch() ) );
  searchButton->setText( i18n("Search" ) );  
  QApplication::restoreOverrideCursor();
  //closeButton->setEnabled( true );
}

void LDAPSearchDialogImpl::slotError( const QString& err )
{
  QApplication::restoreOverrideCursor();
  KMessageBox::error( this, err );
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
      addr.setNameFromString( QString::fromUtf8( cli->_attrs["cn"].first() ) );

      // email
      KLdapAttrValue lst = cli->_attrs["mail"];
      KLdapAttrValue::ConstIterator it = lst.begin();
      bool pref = true;
      if ( it != lst.end() ) {
        addr.insertEmail( QString::fromUtf8( *it ), pref );
        pref = false;
        ++it;
      }

      // phone
      addr.insertPhoneNumber( QString::fromUtf8( cli->_attrs["telePhonenumber"].first() ) );

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
  while( cli ) {
    if( cli->isSelected() ) {
      QString email = QString::fromUtf8(cli->_attrs["mail"].first()).stripWhiteSpace();
      if( !email.isEmpty() ) {
	QString name = QString::fromUtf8(cli->_attrs["cn"].first()).stripWhiteSpace();
	if( name.isEmpty() ) {
	  result << email;
	} else {
	  result << name + " <" + email + ">";
	}
      }
    }
    cli = static_cast<ContactListItem*>( cli->nextSibling() );
  }
  return result.join(", ");
}

void LDAPSearchDialogImpl::slotSendMail()
{
  kapp->invokeMailer( selectedEMails(), "" );
}
#include "ldapsearchdialogimpl.moc"
