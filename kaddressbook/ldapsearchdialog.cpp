/* ldapsearchdialogimpl.cpp - LDAP access
 *      Copyright (C) 2002 Klar�vdalens Datakonsult AB
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


#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <addresseelineedit.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kabcore.h"
#include "ldapsearchdialog.h"
#include "kablock.h"

static QString asUtf8( const QByteArray &val )
{
  if ( val.isEmpty() )
    return QString::null;

  const char *data = val.data();

  //QString::fromUtf8() bug workaround
  if ( data[ val.size() - 1 ] == '\0' ) 
    return QString::fromUtf8( data, val.size() - 1 );
  else
    return QString::fromUtf8( data, val.size() );
}

static QString join( const KPIM::LdapAttrValue& lst, const QString& sep )
{
  QString res;
  bool alredy = false;
  for ( KPIM::LdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( alredy )
      res += sep;
    alredy = TRUE;
    res += asUtf8( *it );
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
    keys[ i18n( "Home Number" ) ] = "homePhone";
    keys[ i18n( "Work Number" ) ] = "telephoneNumber";
    keys[ i18n( "Mobile Number" ) ] = "mobile";
    keys[ i18n( "Fax Number" ) ] = "facsimileTelephoneNumber";
    keys[ i18n( "Pager" ) ] = "pager";
    keys[ i18n( "Street") ] = "street";
    keys[ i18n( "State" ) ] = "st";
    keys[ i18n( "Country" ) ] = "co";
    keys[ i18n( "City" ) ] = "l";
    keys[ i18n( "Organization" ) ] = "o";
    keys[ i18n( "Company" ) ] = "Company";
    keys[ i18n( "Department" ) ] = "department";
    keys[ i18n( "Zip Code" ) ] = "postalCode";
    keys[ i18n( "Postal Address" ) ] = "postalAddress";
    keys[ i18n( "Description" ) ] = "description";
    keys[ i18n( "User ID" ) ] = "uid";
  }
  return keys;
}

class ContactListItem : public QListViewItem
{
  public:
    ContactListItem( QListView* parent, const KPIM::LdapAttrMap& attrs )
      : QListViewItem( parent ), mAttrs( attrs )
    { }

    KPIM::LdapAttrMap mAttrs;

    virtual QString text( int col ) const
    {
      // Look up a suitable attribute for column col
      QString colName = listView()->columnText( col );
      return join( mAttrs[ adrbookattr2ldap()[ colName ] ], ", " );
    }
};

LDAPSearchDialog::LDAPSearchDialog( KABC::AddressBook *ab, KABCore *core,
                                    QWidget* parent, const char* name )
  : KDialogBase( Plain, i18n( "Search for Addresses in Directory" ), Help | User1 |
    User2 | User3 | Cancel, Default, parent, name, false, true ),
    mAddressBook( ab ), mCore( core )
{
  setButtonCancel( KStdGuiItem::close() );
  QFrame *page = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( page, marginHint(), spacingHint() );

  QGroupBox *groupBox = new QGroupBox( i18n( "Search for Addresses in Directory" ),
                                       page );
  groupBox->setFrameShape( QGroupBox::Box );
  groupBox->setFrameShadow( QGroupBox::Sunken );
  groupBox->setColumnLayout( 0, Qt::Vertical );
  QGridLayout *boxLayout = new QGridLayout( groupBox->layout(), 2,
                                            5, spacingHint() );
  boxLayout->setColStretch( 1, 1 );

  QLabel *label = new QLabel( i18n( "Search for:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );

  mSearchEdit = new KLineEdit( groupBox );
  boxLayout->addWidget( mSearchEdit, 0, 1 );
  label->setBuddy( mSearchEdit );

  label = new QLabel( i18n( "in" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mFilterCombo = new KComboBox( groupBox );
  mFilterCombo->insertItem( i18n( "Name" ) );
  mFilterCombo->insertItem( i18n( "Email" ) );
  mFilterCombo->insertItem( i18n( "Home Number" ) );
  mFilterCombo->insertItem( i18n( "Work Number" ) );
  boxLayout->addWidget( mFilterCombo, 0, 3 );

  QSize buttonSize;
  mSearchButton = new QPushButton( i18n( "Stop" ), groupBox );
  buttonSize = mSearchButton->sizeHint();
  mSearchButton->setText( i18n( "Search" ) );
  if ( buttonSize.width() < mSearchButton->sizeHint().width() )
    buttonSize = mSearchButton->sizeHint();
  mSearchButton->setFixedWidth( buttonSize.width() );

  mSearchButton->setDefault( true );
  boxLayout->addWidget( mSearchButton, 0, 4 );

  mRecursiveCheckbox = new QCheckBox( i18n( "Recursive search" ), groupBox  );
  mRecursiveCheckbox->setChecked( true );
  boxLayout->addMultiCellWidget( mRecursiveCheckbox, 1, 1, 0, 4 );

  mSearchType = new KComboBox( groupBox );
  mSearchType->insertItem( i18n( "Contains" ) );
  mSearchType->insertItem( i18n( "Starts With" ) );
  boxLayout->addMultiCellWidget( mSearchType, 1, 1, 3, 4 );
  
  topLayout->addWidget( groupBox );

  mResultListView = new QListView( page );
  mResultListView->setSelectionMode( QListView::Multi );
  mResultListView->setAllColumnsShowFocus( true );
  mResultListView->setShowSortIndicator( true );
  topLayout->addWidget( mResultListView );

  resize( QSize( 600, 400).expandedTo( minimumSizeHint() ) );

  setButtonText( User1, i18n( "Unselect All" ) );
  setButtonText( User2, i18n( "Select All" ) );
  setButtonText( User3, i18n( "Add Selected" ) );

  mNumHosts = 0;
  mIsOK = false;

  connect( mRecursiveCheckbox, SIGNAL( toggled( bool ) ),
	   this, SLOT( slotSetScope( bool ) ) );
  connect( mSearchButton, SIGNAL( clicked() ),
	   this, SLOT( slotStartSearch() ) );

  setTabOrder(mSearchEdit, mFilterCombo);
  setTabOrder(mFilterCombo, mSearchButton);
  mSearchEdit->setFocus();

  restoreSettings();
}

LDAPSearchDialog::~LDAPSearchDialog()
{
  saveSettings();
}

void LDAPSearchDialog::restoreSettings()
{
  // Create one KPIM::LdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at 
  // the beginning of the process
  mLdapClientList.setAutoDelete( true );
  mLdapClientList.clear();

  KConfig kabConfig( "kaddressbookrc" );
  kabConfig.setGroup( "LDAPSearch" );
  mSearchType->setCurrentItem( kabConfig.readNumEntry( "SearchType", 0 ) );

  // then read the config file and register all selected 
  // server in the list
  KConfig* config = KPIM::LdapSearch::config();
  KConfigGroupSaver saver( config, "LDAP" );
  mNumHosts = config->readUnsignedNumEntry( "NumSelectedHosts" ); 
  if ( !mNumHosts ) {
    KMessageBox::error( this, i18n( "You must select a LDAP server before searching.\nYou can do this from the menu Settings/Configure KAddressBook." ) );
    mIsOK = false;
  } else {
    mIsOK = true;
    for ( int j = 0; j < mNumHosts; ++j ) {
      KPIM::LdapClient* ldapClient = new KPIM::LdapClient( 0, this, "ldapclient" );
      KPIM::LdapServer ldapServer;
      KPIM::LdapSearch::readConfig( ldapServer, config, j, true );
      ldapClient->setServer( ldapServer );
      QStringList attrs;

      for ( QMap<QString,QString>::ConstIterator it = adrbookattr2ldap().begin(); it != adrbookattr2ldap().end(); ++it )
        attrs << *it;
        
      ldapClient->setAttrs( attrs );

      connect( ldapClient, SIGNAL( result( const KPIM::LdapObject& ) ),
	       this, SLOT( slotAddResult( const KPIM::LdapObject& ) ) );
      connect( ldapClient, SIGNAL( done() ),
	       this, SLOT( slotSearchDone() ) ); 
      connect( ldapClient, SIGNAL( error( const QString& ) ),
	       this, SLOT( slotError( const QString& ) ) );

      mLdapClientList.append( ldapClient );     
    }

/** CHECKIT*/
    while ( mResultListView->header()->count() > 0 ) {
      mResultListView->removeColumn(0);
    }

    mResultListView->addColumn( i18n( "Full Name" ) );
    mResultListView->addColumn( i18n( "Email" ) );
    mResultListView->addColumn( i18n( "Home Number" ) );
    mResultListView->addColumn( i18n( "Work Number" ) );
    mResultListView->addColumn( i18n( "Mobile Number" ) );
    mResultListView->addColumn( i18n( "Fax Number" ) );
    mResultListView->addColumn( i18n( "Company" ) );
    mResultListView->addColumn( i18n( "Organization" ) );
    mResultListView->addColumn( i18n( "Street" ) );
    mResultListView->addColumn( i18n( "State" ) );
    mResultListView->addColumn( i18n( "Country" ) );
    mResultListView->addColumn( i18n( "Zip Code" ) );
    mResultListView->addColumn( i18n( "Postal Address" ) );
    mResultListView->addColumn( i18n( "City" ) );
    mResultListView->addColumn( i18n( "Department" ) );
    mResultListView->addColumn( i18n( "Description" ) );
    mResultListView->addColumn( i18n( "User ID" ) );
    mResultListView->addColumn( i18n( "Title" ) );

    mResultListView->clear();
  }
}

void LDAPSearchDialog::saveSettings()
{
  KConfig config( "kaddressbookrc" );
  config.setGroup( "LDAPSearch" );
  config.writeEntry( "SearchType", mSearchType->currentItem() );
  config.sync();
}

void LDAPSearchDialog::cancelQuery()
{
  for ( KPIM::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    client->cancelQuery();
  }
}

void LDAPSearchDialog::slotAddResult( const KPIM::LdapObject& obj )
{
  new ContactListItem( mResultListView, obj.attrs );
}

void LDAPSearchDialog::slotSetScope( bool rec )
{
  for ( KPIM::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    if ( rec )
      client->setScope( "sub" );
    else
      client->setScope( "one" );  
  }
}

QString LDAPSearchDialog::makeFilter( const QString& query, const QString& attr,
                                      bool startsWith )
{
  if ( query.isEmpty() )
    // Return a filter that matches everything
    return QString( "|(cn=*)(sn=*)" );

  if ( attr == i18n( "Name" ) ) {
    QString result( (startsWith ? "|(cn=%1*)(sn=%2*)" : "|(cn=*%1*)(sn=*%2*)") );

    result = result.arg( query ).arg( query );

    return result;
  } else {
    QString result( (startsWith ? "%1=%2*" : "%1=*%2*") );

    if ( attr == i18n( "Email" ) ) {
      result = result.arg( "mail" ).arg( query );
    } else if ( attr == i18n( "Home Number" ) ) {
      result = result.arg( "homePhone" ).arg( query );
    } else if ( attr == i18n( "Work Number" ) ) {
      result = result.arg( "telephoneNumber" ).arg( query );
    } else {
      // Error?
      result = QString::null;
    }

    return result;
  }
}

void LDAPSearchDialog::slotStartSearch()
{
  cancelQuery();

  QApplication::setOverrideCursor( Qt::waitCursor );
  mSearchButton->setText( i18n( "Stop" ) );

  disconnect( mSearchButton, SIGNAL( clicked() ),
              this, SLOT( slotStartSearch() ) );
  connect( mSearchButton, SIGNAL( clicked() ),
           this, SLOT( slotStopSearch() ) );

  bool startsWith = (mSearchType->currentItem() == 1);

  QString filter = makeFilter( mSearchEdit->text().stripWhiteSpace(), mFilterCombo->currentText(), startsWith );

   // loop in the list and run the KPIM::LdapClients 
  mResultListView->clear();
  for ( KPIM::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() )
    client->startQuery( filter );

  saveSettings();
}

void LDAPSearchDialog::slotStopSearch()
{
  cancelQuery();
  slotSearchDone();
}

void LDAPSearchDialog::slotSearchDone()
{
  // If there are no more active clients, we are done.
  for ( KPIM::LdapClient* client = mLdapClientList.first(); client; client = mLdapClientList.next() ) {
    if ( client->isActive() )
      return;
  }

  disconnect( mSearchButton, SIGNAL( clicked() ),
              this, SLOT( slotStopSearch() ) );
  connect( mSearchButton, SIGNAL( clicked() ),
           this, SLOT( slotStartSearch() ) );

  mSearchButton->setText( i18n( "Search" ) );
  QApplication::restoreOverrideCursor();
}

void LDAPSearchDialog::slotError( const QString& error )
{
  QApplication::restoreOverrideCursor();
  KMessageBox::error( this, error );
}

void LDAPSearchDialog::closeEvent( QCloseEvent* e )
{
  slotStopSearch();
  e->accept();
}

/*!
 * Returns a ", " separated list of email addresses that were
 * checked by the user
 */
QString LDAPSearchDialog::selectedEMails() const
{
  QStringList result;
  ContactListItem* cli = static_cast<ContactListItem*>( mResultListView->firstChild() );
  while ( cli ) {
    if ( cli->isSelected() ) {
      QString email = asUtf8( cli->mAttrs[ "mail" ].first() ).stripWhiteSpace();
      if ( !email.isEmpty() ) {
        QString name = asUtf8( cli->mAttrs[ "cn" ].first() ).stripWhiteSpace();
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

void LDAPSearchDialog::slotHelp()
{
  kapp->invokeHelp( "ldap-queries" );
}

void LDAPSearchDialog::slotUser1()
{
  mResultListView->selectAll( false );
}

void LDAPSearchDialog::slotUser2()
{
  mResultListView->selectAll( true );
}

void LDAPSearchDialog::slotUser3()
{
  
  KABC::Resource *resource = mCore->requestResource( this );
  if ( !resource ) return;
  KABLock::self( mAddressBook )->lock( resource );

  ContactListItem* cli = static_cast<ContactListItem*>( mResultListView->firstChild() );
  while ( cli ) {
    if ( cli->isSelected() ) {
      KABC::Addressee addr;

      // name
      addr.setNameFromString( asUtf8( cli->mAttrs["cn"].first() ) );

      // email
      KPIM::LdapAttrValue lst = cli->mAttrs["mail"];
      KPIM::LdapAttrValue::ConstIterator it = lst.begin();
      bool pref = true;
      if ( it != lst.end() ) {
        addr.insertEmail( asUtf8( *it ), pref );
        pref = false;
        ++it;
      }

      addr.setOrganization( asUtf8( cli->mAttrs[ "o" ].first() ) );
      if ( addr.organization().isEmpty() )
         addr.setOrganization( asUtf8( cli->mAttrs[ "Company" ].first() ) );

      addr.insertCustom("KADDRESSBOOK", "X-Department", asUtf8( cli->mAttrs[ "department" ].first() ) );

      // Address
      KABC::Address workAddr( KABC::Address::Work );

      workAddr.setStreet( asUtf8( cli->mAttrs[ "street" ].first()) );
      workAddr.setLocality( asUtf8( cli->mAttrs[ "l" ].first()) );
      workAddr.setRegion( asUtf8( cli->mAttrs[ "st" ].first()));
      workAddr.setPostalCode( asUtf8( cli->mAttrs[ "postalCode" ].first()) );
      workAddr.setCountry( asUtf8( cli->mAttrs[ "co" ].first()) );

      if ( !workAddr.isEmpty() )
        addr.insertAddress( workAddr );

      // phone
      KABC::PhoneNumber homeNr = asUtf8( cli->mAttrs[  "homePhone" ].first() );
      homeNr.setType( KABC::PhoneNumber::Home );
      addr.insertPhoneNumber( homeNr );

      KABC::PhoneNumber workNr = asUtf8( cli->mAttrs[  "telephoneNumber" ].first() );
      workNr.setType( KABC::PhoneNumber::Work );
      addr.insertPhoneNumber( workNr );

      KABC::PhoneNumber faxNr = asUtf8( cli->mAttrs[  "facsimileTelephoneNumber" ].first() );
      faxNr.setType( KABC::PhoneNumber::Fax );
      addr.insertPhoneNumber( faxNr );

      KABC::PhoneNumber cellNr = asUtf8( cli->mAttrs[  "mobile" ].first() );
      cellNr.setType( KABC::PhoneNumber::Cell );
      addr.insertPhoneNumber( cellNr );

      KABC::PhoneNumber pagerNr = asUtf8( cli->mAttrs[  "pager" ].first() );
      pagerNr.setType( KABC::PhoneNumber::Pager );
      addr.insertPhoneNumber( pagerNr );

      if ( mAddressBook ) {
        addr.setResource( resource );
        mAddressBook->insertAddressee( addr );
      }
    }
    cli = static_cast<ContactListItem*>( cli->nextSibling() );
  }

  KABLock::self( mAddressBook )->unlock( resource );
  emit addresseesAdded();
}

#include "ldapsearchdialog.moc"
