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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <QApplication>
#include <QCheckBox>
#include <qgroupbox.h>
#include <q3header.h>
#include <QLabel>
#include <QMap>
#include <q3listview.h>
#include <QPointer>
#include <QPushButton>
//Added by qt3to4:
#include <QGridLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QVBoxLayout>

#include <addresseelineedit.h>
#include <KRandom>
#include <KDialogButtonBox>
#include <kcombobox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>
#include <kconfiggroup.h>

#include "kabcore.h"
#include "ldapsearchdialog.h"
#include "kablock.h"
#include "distributionlistpicker.h"


static QString asUtf8( const QByteArray &val )
{
  if ( val.isEmpty() )
    return QString();

  const char *data = val.data();

  //QString::fromUtf8() bug workaround
  if ( data[ val.size() - 1 ] == '\0' )
    return QString::fromUtf8( data, val.size() - 1 );
  else
    return QString::fromUtf8( data, val.size() );
}

static QString join( const KLDAP::LdapAttrValue& lst, const QString& sep )
{
  QString res;
  bool alredy = false;
  for ( KLDAP::LdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( alredy )
      res += sep;
    alredy = true;
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
    keys[ i18n( "Street" ) ] = "street";
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

class ContactListItem : public Q3ListViewItem
{
  public:
    ContactListItem( Q3ListView* parent, const KLDAP::LdapAttrMap& attrs )
      : Q3ListViewItem( parent ), mAttrs( attrs )
    { }

    KLDAP::LdapAttrMap mAttrs;

    virtual QString text( int col ) const
    {
      // Look up a suitable attribute for column col
      const QString colName = listView()->columnText( col );
      const QString ldapAttrName = adrbookattr2ldap()[ colName ];
      return join( mAttrs[ ldapAttrName ], ", " );
    }
};

class LDAPSearchDialog::Private
{
  public:
    static QList<ContactListItem*> selectedItems( Q3ListView* );
    QMap<const ContactListItem*, QString> itemToServer;
};

QList<ContactListItem*> LDAPSearchDialog::Private::selectedItems( Q3ListView* view )
{
  QList<ContactListItem*> selected;
  ContactListItem* cli = static_cast<ContactListItem*>( view->firstChild() );
  while ( cli ) {
    if ( cli->isSelected() )
      selected.append( cli );
    cli = static_cast<ContactListItem*>( cli->nextSibling() );
  }
  return selected;
}

LDAPSearchDialog::LDAPSearchDialog( KABC::AddressBook *ab, KABCore *core,
                                    QWidget* parent )
  : KDialog( parent ),
    mAddressBook( ab ), mCore( core ), d( new Private )
{
  setCaption( i18n( "Search for Addresses in Directory" ) );
  setButtons( Help | User1 | User2 | Cancel );
  setDefaultButton( User1 );
  setModal( false );
  showButtonSeparator( true );
  setButtonGuiItem( KDialog::Cancel, KStandardGuiItem::close() );
  QFrame *page = new QFrame( this );
  setMainWidget( page );
  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );

  QGroupBox *groupBox = new QGroupBox( i18n( "Search for Addresses in Directory" ),
                                       page );
  QGridLayout *boxLayout = new QGridLayout();
  groupBox->setLayout( boxLayout );
  boxLayout->setSpacing( spacingHint() );
  boxLayout->setColumnStretch( 1, 1 );

  QLabel *label = new QLabel( i18n( "Search for:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );

  mSearchEdit = new KLineEdit( groupBox );
  boxLayout->addWidget( mSearchEdit, 0, 1 );
  label->setBuddy( mSearchEdit );

  label = new QLabel( i18nc( "In LDAP attribute", "in" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mFilterCombo = new KComboBox( groupBox );
  mFilterCombo->addItem( i18n( "Name" ) );
  mFilterCombo->addItem( i18n( "Email" ) );
  mFilterCombo->addItem( i18n( "Home Number" ) );
  mFilterCombo->addItem( i18n( "Work Number" ) );
  boxLayout->addWidget( mFilterCombo, 0, 3 );

  QSize buttonSize;
  mSearchButton = new QPushButton( i18n( "Stop" ), groupBox );
  buttonSize = mSearchButton->sizeHint();
  mSearchButton->setText( i18n( "&Search" ) );
  if ( buttonSize.width() < mSearchButton->sizeHint().width() )
    buttonSize = mSearchButton->sizeHint();
  mSearchButton->setFixedWidth( buttonSize.width() );

  mSearchButton->setDefault( true );
  boxLayout->addWidget( mSearchButton, 0, 4 );

  mRecursiveCheckbox = new QCheckBox( i18n( "Recursive search" ), groupBox  );
  mRecursiveCheckbox->setChecked( true );
  boxLayout->addWidget( mRecursiveCheckbox, 1, 0, 1, 5 );

  mSearchType = new KComboBox( groupBox );
  mSearchType->addItem( i18n( "Contains" ) );
  mSearchType->addItem( i18n( "Starts With" ) );
  boxLayout->addWidget( mSearchType, 1, 3, 1, 2 );

  topLayout->addWidget( groupBox );

  mResultListView = new Q3ListView( page );
  mResultListView->setSelectionMode( Q3ListView::Multi );
  mResultListView->setAllColumnsShowFocus( true );
  mResultListView->setShowSortIndicator( true );
  topLayout->addWidget( mResultListView );

  KDialogButtonBox *buttons = new KDialogButtonBox( page, Qt::Horizontal );
  buttons->addButton( i18n( "Select All" ), QDialogButtonBox::ActionRole, this, SLOT( slotSelectAll() ) );
  buttons->addButton( i18n( "Unselect All" ), QDialogButtonBox::ActionRole, this, SLOT( slotUnselectAll() ) );

  topLayout->addWidget( buttons );

  resize( QSize( 600, 400).expandedTo( minimumSizeHint() ) );

  setButtonText( User1, i18n( "Add Selected" ) );
  setButtonText( User2, i18n( "Add to Distribution List..." ) );

  mNumHosts = 0;
  mIsOK = false;

  connect( mRecursiveCheckbox, SIGNAL( toggled( bool ) ),
	   this, SLOT( slotSetScope( bool ) ) );
  connect( mSearchButton, SIGNAL( clicked() ),
	   this, SLOT( slotStartSearch() ) );

  setTabOrder(mSearchEdit, mFilterCombo);
  setTabOrder(mFilterCombo, mSearchButton);
  mSearchEdit->setFocus();
  connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
  connect(this,SIGNAL(user2Clicked()),this,SLOT(slotUser2()));
  connect(this,SIGNAL(helpClicked()),this,SLOT(slotHelp()));
  restoreSettings();
}

LDAPSearchDialog::~LDAPSearchDialog()
{
  saveSettings();
  delete d;
}

void LDAPSearchDialog::restoreSettings()
{
  // Create one KPIM::LdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at
  // the beginning of the process
  qDeleteAll(mLdapClientList);
  mLdapClientList.clear();

  KConfig _kabConfig( "kaddressbookrc" );
  KConfigGroup kabConfig(&_kabConfig, "LDAPSearch" );
  mSearchType->setCurrentIndex( kabConfig.readEntry( "SearchType", 0 ) );

  // then read the config file and register all selected
  // server in the list
  KConfig* config = KPIM::LdapSearch::config();
  KConfigGroup group( config, "LDAP" );
  mNumHosts = group.readEntry( "NumSelectedHosts", 0 );
  if ( !mNumHosts ) {
    KMessageBox::error( this, i18n( "You must select a LDAP server before searching.\nYou can do this from the menu Settings/Configure KAddressBook." ) );
    mIsOK = false;
  } else {
    mIsOK = true;
    for ( int j = 0; j < mNumHosts; ++j ) {
      KLDAP::LdapServer ldapServer;
      KPIM::LdapClient* ldapClient = new KPIM::LdapClient( 0, this, "ldapclient" );
      KPIM::LdapSearch::readConfig( ldapServer, group, j, true );
      ldapClient->setServer( ldapServer );
      QStringList attrs;

      for ( QMap<QString, QString>::ConstIterator it = adrbookattr2ldap().begin(); it != adrbookattr2ldap().end(); ++it )
        attrs << *it;

      ldapClient->setAttrs( attrs );

      connect( ldapClient, SIGNAL( result( const KPIM::LdapClient&, const KLDAP::LdapObject& ) ),
               this, SLOT( slotAddResult( const KPIM::LdapClient&, const KLDAP::LdapObject& ) ) );
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
    d->itemToServer.clear();
  }
}

void LDAPSearchDialog::saveSettings()
{
  KConfig _config( "kaddressbookrc" );
  KConfigGroup config(&_config, "LDAPSearch" );
  config.writeEntry( "SearchType", mSearchType->currentIndex() );
  config.sync();
}

void LDAPSearchDialog::cancelQuery()
{
  Q_FOREACH( KPIM::LdapClient* client , mLdapClientList ) {
    client->cancelQuery();
  }
}

void LDAPSearchDialog::slotAddResult( const KPIM::LdapClient&, const KLDAP::LdapObject& obj )
{
  ContactListItem* item = new ContactListItem( mResultListView, obj.attributes() );
  //d->itemToServer[item] = obj.client->server().host();
}

void LDAPSearchDialog::slotSetScope( bool rec )
{
    Q_FOREACH( KPIM::LdapClient* client , mLdapClientList ) {
    if ( rec )
      client->setScope( "sub" );
    else
      client->setScope( "one" );
  }
}

QString LDAPSearchDialog::makeFilter( const QString& query, const QString& attr,
                                      bool startsWith )
{
  /* The reasoning behind this filter is:
   * If it's a person, or a distlist, show it, even if it doesn't have an email address.
   * If it's not a person, or a distlist, only show it if it has an email attribute.
   * This allows both resource accounts with an email address which are not a person and
   * person entries without an email address to show up, while still not showing things
   * like structural entries in the ldap tree. */
  QString result( "&(|(objectclass=person)(objectclass=groupofnames)(mail=*))(" );
  if( query.isEmpty() )
    // Return a filter that matches everything
    return result + "|(cn=*)(sn=*)" + ')';

  if ( attr == i18n( "Name" ) ) {
    result += startsWith ? "|(cn=%1*)(sn=%2*)" : "|(cn=*%1*)(sn=*%2*)";
    result = result.arg( query ).arg( query );
  } else {
    result += (startsWith ? "%1=%2*" : "%1=*%2*");
    if ( attr == i18n( "Email" ) ) {
      result = result.arg( "mail" ).arg( query );
    } else if ( attr == i18n( "Home Number" ) ) {
      result = result.arg( "homePhone" ).arg( query );
    } else if ( attr == i18n( "Work Number" ) ) {
      result = result.arg( "telephoneNumber" ).arg( query );
    } else {
      // Error?
      result.clear();
      return result;
    }
  }
  result += ')';
  return result;
}

void LDAPSearchDialog::slotStartSearch()
{
  cancelQuery();

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mSearchButton->setText( i18n( "Stop" ) );

  disconnect( mSearchButton, SIGNAL( clicked() ),
              this, SLOT( slotStartSearch() ) );
  connect( mSearchButton, SIGNAL( clicked() ),
           this, SLOT( slotStopSearch() ) );

  bool startsWith = (mSearchType->currentIndex() == 1);

  QString filter = makeFilter( mSearchEdit->text().trimmed(), mFilterCombo->currentText(), startsWith );

   // loop in the list and run the KPIM::LdapClients
  mResultListView->clear();
  d->itemToServer.clear();
  Q_FOREACH( KPIM::LdapClient* const client , mLdapClientList )
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
  Q_FOREACH( KPIM::LdapClient* client , mLdapClientList ) {
    if ( client->isActive() )
      return;
  }

  disconnect( mSearchButton, SIGNAL( clicked() ),
              this, SLOT( slotStopSearch() ) );
  connect( mSearchButton, SIGNAL( clicked() ),
           this, SLOT( slotStartSearch() ) );

  mSearchButton->setText( i18n( "&Search" ) );
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
      QString email = asUtf8( cli->mAttrs[ "mail" ].first() ).trimmed();
      if ( !email.isEmpty() ) {
        QString name = asUtf8( cli->mAttrs[ "cn" ].first() ).trimmed();
        if ( name.isEmpty() ) {
          result << email;
        } else {
          result << name + " <" + email + '>';
        }
      }
    }
    cli = static_cast<ContactListItem*>( cli->nextSibling() );
  }

  return result.join( ", " );
}

void LDAPSearchDialog::slotHelp()
{
  KToolInvocation::invokeHelp( "ldap-queries" );
}

void LDAPSearchDialog::slotUnselectAll()
{
  mResultListView->selectAll( false );
}

void LDAPSearchDialog::slotSelectAll()
{
  mResultListView->selectAll( true );
}

KABC::Addressee LDAPSearchDialog::convertLdapAttributesToAddressee( const KLDAP::LdapAttrMap& attrs )
{
  KABC::Addressee addr;

  // name
  addr.setNameFromString( asUtf8( attrs["cn"].first() ) );

  // email
  KLDAP::LdapAttrValue lst = attrs["mail"];
  KLDAP::LdapAttrValue::ConstIterator it = lst.begin();
  bool pref = true;
  if ( it != lst.end() ) {
    addr.insertEmail( asUtf8( *it ), pref );
    pref = false;
    ++it;
  }

  addr.setOrganization( asUtf8( attrs[ "o" ].first() ) );
  if ( addr.organization().isEmpty() )
    addr.setOrganization( asUtf8( attrs[ "Company" ].first() ) );

  // Address
  KABC::Address workAddr( KABC::Address::Work );

  addr.setDepartment( asUtf8( attrs[ "department" ].first() ) );

  if ( !workAddr.isEmpty() )
    addr.insertAddress( workAddr );

  // phone
  KABC::PhoneNumber homeNr = asUtf8( attrs[  "homePhone" ].first() );
  homeNr.setType( KABC::PhoneNumber::Home );
  addr.insertPhoneNumber( homeNr );

  KABC::PhoneNumber workNr = asUtf8( attrs[  "telephoneNumber" ].first() );
  workNr.setType( KABC::PhoneNumber::Work );
  addr.insertPhoneNumber( workNr );

  KABC::PhoneNumber faxNr = asUtf8( attrs[  "facsimileTelephoneNumber" ].first() );
  faxNr.setType( KABC::PhoneNumber::Fax );
  addr.insertPhoneNumber( faxNr );

  KABC::PhoneNumber cellNr = asUtf8( attrs[  "mobile" ].first() );
  cellNr.setType( KABC::PhoneNumber::Cell );
  addr.insertPhoneNumber( cellNr );

  KABC::PhoneNumber pagerNr = asUtf8( attrs[  "pager" ].first() );
  pagerNr.setType( KABC::PhoneNumber::Pager );
  addr.insertPhoneNumber( pagerNr );
  return addr;
}

KPIM::DistributionList LDAPSearchDialog::selectDistributionList()
{
  QPointer<KPIM::DistributionListPickerDialog> picker = new KPIM::DistributionListPickerDialog( mCore->addressBook(), this );
  picker->setLabelText( i18n( "Select a distribution list to add the selected contacts to." ) );
  picker->setCaption( i18n( "Select Distribution List" ) );
  picker->exec();
  const KPIM::DistributionList list = KPIM::DistributionList::findByName( mCore->addressBook(), picker
? picker->selectedDistributionList() : QString() );
  delete picker;
  return list;
}

KABC::Addressee::List LDAPSearchDialog::importContactsUnlessTheyExist( const QList<ContactListItem*>& selectedItems,
                                                                       KABC::Resource * const resource )
{
    const QDateTime now = QDateTime::currentDateTime();
    QStringList importedAddrs;
    KABC::Addressee::List localAddrs;

    KABLock::self( mCore->addressBook() )->lock( resource );

    for ( QList<ContactListItem*>::ConstIterator it = selectedItems.begin(); it != selectedItems.end(); ++it ) {
      const ContactListItem * const cli = *it;
      KABC::Addressee addr = convertLdapAttributesToAddressee( cli->mAttrs );
      const KABC::Addressee::List existing = mCore->addressBook()->findByEmail( addr.preferredEmail() );

      if ( existing.isEmpty() ) {
        addr.setUid( KRandom::randomString( 10 ) );
        addr.setNote( i18nc( "arguments are host name, datetime", "Imported from LDAP directory %1 on %2", d->itemToServer[cli], KGlobal::locale()->formatDateTime( now ) ) );
        addr.setResource( resource );
        mCore->addressBook()->insertAddressee( addr );
        importedAddrs.append( addr.fullEmail() );
        localAddrs.append( addr );
      } else {
        localAddrs.append( existing.first() );
      }
    }
    KABLock::self( mCore->addressBook() )->unlock( resource );
    if ( !importedAddrs.isEmpty() ) {
      KMessageBox::informationList( this, i18np( "The following contact was imported into your address book:",
                                    "The following %1 contacts were imported into your address book:", importedAddrs.count() ),
                                    importedAddrs );
      emit addresseesAdded();
    }
    return localAddrs;
}

void LDAPSearchDialog::slotUser2()
{
    KABC::Resource *resource = mCore->requestResource( this );
    if ( !resource ) return;

    const QList<ContactListItem*> selectedItems = d->selectedItems( mResultListView );
    if ( selectedItems.isEmpty() ) {
      KMessageBox::information( this, i18n( "Please select the contacts you want to add to the distribution list." ), i18n( "No Contacts Selected" ) );
      return;
    }
    KPIM::DistributionList dist = selectDistributionList();
    if ( dist.isEmpty() )
      return;

    KABC::Addressee::List localAddrs = importContactsUnlessTheyExist( selectedItems, resource );

    if ( localAddrs.isEmpty() )
      return;

    Q_FOREACH ( const KABC::Addressee i, localAddrs ) {
        dist.insertEntry( i, QString() );
    }
    KABLock::self( mCore->addressBook() )->lock( resource );
    mCore->addressBook()->insertAddressee( dist );
    emit addresseesAdded();
    KABLock::self( mCore->addressBook() )->unlock( resource );
}

void LDAPSearchDialog::slotUser1()
{
    KABC::Resource *resource = mCore->requestResource( this );
    if ( !resource ) return;
    const QList<ContactListItem*> selectedItems = d->selectedItems( mResultListView );
    importContactsUnlessTheyExist( selectedItems, resource );
}

#include "ldapsearchdialog.moc"
