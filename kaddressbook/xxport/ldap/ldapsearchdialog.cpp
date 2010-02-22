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

#include "ldapsearchdialog.h"

#include <QtCore/QPair>
#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QCloseEvent>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

#include <akonadi/collection.h>
#include <akonadi/itemcreatejob.h>
#include <kabc/distributionlist.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcmultidialog.h>
#include <kdialogbuttonbox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

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
  for ( KLDAP::LdapAttrValue::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
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
    keys[ i18nc( "@item LDAP search key", "Title" ) ] = "title";
    keys[ i18n( "Full Name" ) ] = "cn";
    keys[ i18nc( "@item LDAP search key", "Email" ) ] = "mail";
    keys[ i18n( "Home Number" ) ] = "homePhone";
    keys[ i18n( "Work Number" ) ] = "telephoneNumber";
    keys[ i18n( "Mobile Number" ) ] = "mobile";
    keys[ i18n( "Fax Number" ) ] = "facsimileTelephoneNumber";
    keys[ i18n( "Pager" ) ] = "pager";
    keys[ i18n( "Street" ) ] = "street";
    keys[ i18nc( "@item LDAP search key", "State" ) ] = "st";
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

class ContactListModel : public QAbstractTableModel
{
  public:
    enum Role {
      ServerRole = Qt::UserRole + 1
    };

    ContactListModel( QObject *parent )
      : QAbstractTableModel( parent )
    {
    }

    void addContact( const KLDAP::LdapAttrMap &contact, const QString &server )
    {
      mContactList.append( contact );
      mServerList.append( server );
      reset();
    }

    QPair<KLDAP::LdapAttrMap, QString> contact( const QModelIndex &index ) const
    {
      if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() )
        return qMakePair( KLDAP::LdapAttrMap(), QString() );

      return qMakePair( mContactList.at( index.row() ), mServerList.at( index.row() ) );
    }

    QString email( const QModelIndex &index ) const
    {
      if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() )
        return QString();

      return asUtf8( mContactList.at( index.row() ).value( "mail" ).first() ).trimmed();
    }

    QString fullName( const QModelIndex &index ) const
    {
      if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() )
        return QString();

      return asUtf8( mContactList.at( index.row() ).value( "cn" ).first() ).trimmed();
    }

    void clear()
    {
      mContactList.clear();
      mServerList.clear();
      reset();
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
      if ( !parent.isValid() )
        return mContactList.count();
      else
        return 0;
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
      if ( !parent.isValid() )
        return 18;
      else
        return 0;
    }

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
    {
      if ( orientation == Qt::Vertical || role != Qt::DisplayRole || section < 0 || section > 17 )
        return QVariant();

      switch ( section ) {
        case  0: return i18n( "Full Name" ); break;
        case  1: return i18nc( "@title:column Column containing email addresses", "Email" ); break;
        case  2: return i18n( "Home Number" ); break;
        case  3: return i18n( "Work Number" ); break;
        case  4: return i18n( "Mobile Number" ); break;
        case  5: return i18n( "Fax Number" ); break;
        case  6: return i18n( "Company" ); break;
        case  7: return i18n( "Organization" ); break;
        case  8: return i18n( "Street" ); break;
        case  9: return i18nc( "@title:column Column containing the residential state of the address", "State" ); break;
        case 10: return i18n( "Country" ); break;
        case 11: return i18n( "Zip Code" ); break;
        case 12: return i18n( "Postal Address" ); break;
        case 13: return i18n( "City" ); break;
        case 14: return i18n( "Department" ); break;
        case 15: return i18n( "Description" ); break;
        case 16: return i18n( "User ID" ); break;
        case 17: return i18nc( "@title:column Column containing title of the person", "Title" ); break;
        default: return QVariant(); break;
      };

      return QVariant();
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( !index.isValid() )
        return QVariant();

      if ( index.row() < 0 || index.row() >= mContactList.count() || index.column() < 0 || index.column() > 17 )
        return QVariant();

      if ( role == ServerRole )
        return mServerList.at( index.row() );

      if ( role != Qt::DisplayRole )
        return QVariant();

      const KLDAP::LdapAttrMap map = mContactList.at( index.row() );

      switch ( index.column() ) {
        case  0: return join( map.value( "cn" ), ", " ); break;
        case  1: return join( map.value( "mail" ), ", " ); break;
        case  2: return join( map.value( "homePhone" ), ", " ); break;
        case  3: return join( map.value( "telephoneNumber" ), ", " ); break;
        case  4: return join( map.value( "mobile" ), ", " ); break;
        case  5: return join( map.value( "facsimileTelephoneNumber" ), ", " ); break;
        case  6: return join( map.value( "Company" ), ", " ); break;
        case  7: return join( map.value( "o" ), ", " ); break;
        case  8: return join( map.value( "street" ), ", " ); break;
        case  9: return join( map.value( "st" ), ", " ); break;
        case 10: return join( map.value( "co" ), ", " ); break;
        case 11: return join( map.value( "postalCode" ), ", " ); break;
        case 12: return join( map.value( "postalAddress" ), ", " ); break;
        case 13: return join( map.value( "l" ), ", " ); break;
        case 14: return join( map.value( "department" ), ", " ); break;
        case 15: return join( map.value( "description" ), ", " ); break;
        case 16: return join( map.value( "uid" ), ", " ); break;
        case 17: return join( map.value( "title" ), ", " ); break;
        default: return QVariant(); break;
      }

      return QVariant();
    }

  private:
    QList<KLDAP::LdapAttrMap> mContactList;
    QStringList mServerList;
};

class LDAPSearchDialog::Private
{
  public:
    static QList< QPair<KLDAP::LdapAttrMap, QString> > selectedItems( QAbstractItemView * );
};

QList< QPair<KLDAP::LdapAttrMap, QString> > LDAPSearchDialog::Private::selectedItems( QAbstractItemView* view )
{
  QList< QPair<KLDAP::LdapAttrMap, QString> > contacts;

  ContactListModel *model = static_cast<ContactListModel*>( view->model() );

  const QModelIndexList selected = view->selectionModel()->selectedRows();
  for ( int i = 0; i < selected.count(); ++i )
    contacts.append( model->contact( selected.at( i ) ) );

  return contacts;
}

LDAPSearchDialog::LDAPSearchDialog( QWidget* parent )
  : KDialog( parent ),
    mModel( 0 ),
    d( new Private )
{
  setCaption( i18n( "Import Contacts from LDAP" ) );
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
  mFilterCombo->addItem( i18nc( "@item:inlistbox Name of the contact", "Name" ) );
  mFilterCombo->addItem( i18nc( "@item:inlistbox email address of the contact", "Email" ) );
  mFilterCombo->addItem( i18nc( "@item:inlistbox", "Home Number" ) );
  mFilterCombo->addItem( i18nc( "@item:inlistbox", "Work Number" ) );
  boxLayout->addWidget( mFilterCombo, 0, 3 );

  QSize buttonSize;
  mSearchButton = new QPushButton( i18n( "Stop" ), groupBox );
  buttonSize = mSearchButton->sizeHint();
  mSearchButton->setText( i18nc( "@action:button Start searching", "&Search" ) );
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

  mResultView = new QTableView( page );
  mResultView->setSelectionMode( QTableView::MultiSelection );
  mResultView->setSelectionBehavior( QTableView::SelectRows );
  mModel = new ContactListModel( mResultView );
  mResultView->setModel( mModel );
  mResultView->verticalHeader()->hide();
  connect( mResultView, SIGNAL( clicked ( const QModelIndex & ) ), SLOT( slotSelectionChanged() ) );
  topLayout->addWidget( mResultView );

  KDialogButtonBox *buttons = new KDialogButtonBox( page, Qt::Horizontal );
  buttons->addButton( i18n( "Select All" ), QDialogButtonBox::ActionRole, this, SLOT( slotSelectAll() ) );
  buttons->addButton( i18n( "Unselect All" ), QDialogButtonBox::ActionRole, this, SLOT( slotUnselectAll() ) );

  topLayout->addWidget( buttons );

  resize( QSize( 600, 400).expandedTo( minimumSizeHint() ) );

  setButtonText( User1, i18n( "Import Selected" ) );
  setButtonText( User2, i18n( "Configure LDAP Servers..." ) );

  mNumHosts = 0;
  mIsConfigured = false;

  connect( mRecursiveCheckbox, SIGNAL( toggled( bool ) ),
           this, SLOT( slotSetScope( bool ) ) );
  connect( mSearchButton, SIGNAL( clicked() ),
           this, SLOT( slotStartSearch() ) );

  setTabOrder(mSearchEdit, mFilterCombo);
  setTabOrder(mFilterCombo, mSearchButton);
  mSearchEdit->setFocus();
  connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
  connect(this,SIGNAL(user2Clicked()),this,SLOT(slotUser2()));
  slotSelectionChanged();
  restoreSettings();
}

LDAPSearchDialog::~LDAPSearchDialog()
{
  saveSettings();
  delete d;
}

KABC::Addressee::List LDAPSearchDialog::selectedContacts() const
{
  return mSelectedContacts;
}

void LDAPSearchDialog::slotSelectionChanged()
{
  enableButton( KDialog::User1, mResultView->selectionModel()->hasSelection () );
}

void LDAPSearchDialog::restoreSettings()
{
  // Create one KPIM::LdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at
  // the beginning of the process
  qDeleteAll( mLdapClientList) ;
  mLdapClientList.clear();

  KConfig kabConfig( "kaddressbookrc" );
  KConfigGroup kabGroup( &kabConfig, "LDAPSearch" );
  mSearchType->setCurrentIndex( kabGroup.readEntry( "SearchType", 0 ) );

  // then read the config file and register all selected
  // server in the list
  KConfig* config = KPIM::LdapSearch::config();
  KConfigGroup group( config, "LDAP" );
  mNumHosts = group.readEntry( "NumSelectedHosts", 0 );
  if ( !mNumHosts ) {
    mIsConfigured = false;
  } else {
    mIsConfigured = true;
    for ( int j = 0; j < mNumHosts; ++j ) {
      KLDAP::LdapServer ldapServer;
      KPIM::LdapClient* ldapClient = new KPIM::LdapClient( 0, this, "ldapclient" );
      KPIM::LdapSearch::readConfig( ldapServer, group, j, true );
      ldapClient->setServer( ldapServer );
      QStringList attrs;

      for ( QMap<QString, QString>::ConstIterator it = adrbookattr2ldap().constBegin(); it != adrbookattr2ldap().constEnd(); ++it )
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

    mModel->clear();
  }
}

void LDAPSearchDialog::saveSettings()
{
  KConfig config( "kaddressbookrc" );
  KConfigGroup group( &config, "LDAPSearch" );
  group.writeEntry( "SearchType", mSearchType->currentIndex() );
  group.sync();
}

void LDAPSearchDialog::cancelQuery()
{
  Q_FOREACH( KPIM::LdapClient* client , mLdapClientList ) {
    client->cancelQuery();
  }
}

void LDAPSearchDialog::slotAddResult( const KPIM::LdapClient &client, const KLDAP::LdapObject& obj )
{
  mModel->addContact( obj.attributes(), client.server().host() );
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
                                      bool startsWith ) const
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

  if ( attr == i18nc( "Search attribute: Name of contact", "Name" ) ) {
    result += startsWith ? "|(cn=%1*)(sn=%2*)" : "|(cn=*%1*)(sn=*%2*)";
    result = result.arg( query ).arg( query );
  } else {
    result += (startsWith ? "%1=%2*" : "%1=*%2*");
    if ( attr == i18nc( "Search attribute: Email of the contact", "Email" ) ) {
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

  if ( !mIsConfigured ) {
    KMessageBox::error( this, i18n( "You must select a LDAP server before searching." ) );
    slotUser2();
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mSearchButton->setText( i18n( "Stop" ) );

  disconnect( mSearchButton, SIGNAL( clicked() ),
              this, SLOT( slotStartSearch() ) );
  connect( mSearchButton, SIGNAL( clicked() ),
           this, SLOT( slotStopSearch() ) );

  bool startsWith = (mSearchType->currentIndex() == 1);

  QString filter = makeFilter( mSearchEdit->text().trimmed(), mFilterCombo->currentText(), startsWith );

   // loop in the list and run the KPIM::LdapClients
  mModel->clear();
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

  mSearchButton->setText( i18nc( "@action:button Start searching", "&Search" ) );
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

void LDAPSearchDialog::slotUnselectAll()
{
  mResultView->clearSelection();
}

void LDAPSearchDialog::slotSelectAll()
{
  mResultView->selectAll();
}

KABC::Addressee LDAPSearchDialog::convertLdapAttributesToAddressee( const KLDAP::LdapAttrMap& attrs )
{
  KABC::Addressee addr;

  // name
  if ( !attrs.value( "cn" ).isEmpty() )
    addr.setNameFromString( asUtf8( attrs["cn"].first() ) );

  // email
  KLDAP::LdapAttrValue lst = attrs["mail"];
  KLDAP::LdapAttrValue::ConstIterator it = lst.constBegin();
  bool pref = true;
  while ( it != lst.constEnd() ) {
    addr.insertEmail( asUtf8( *it ), pref );
    pref = false;
    ++it;
  }

  if ( !attrs.value( "o" ).isEmpty() )
    addr.setOrganization( asUtf8( attrs[ "o" ].first() ) );
  if ( addr.organization().isEmpty() && !attrs.value( "Company" ).isEmpty() )
    addr.setOrganization( asUtf8( attrs[ "Company" ].first() ) );

  // Address
  KABC::Address workAddr( KABC::Address::Work );

  if ( !attrs.value( "department" ).isEmpty() )
    addr.setDepartment( asUtf8( attrs[ "department" ].first() ) );

  if ( !workAddr.isEmpty() )
    addr.insertAddress( workAddr );

  // phone
  if ( !attrs.value( "homePhone" ).isEmpty() ) {
    KABC::PhoneNumber homeNr = asUtf8( attrs[  "homePhone" ].first() );
    homeNr.setType( KABC::PhoneNumber::Home );
    addr.insertPhoneNumber( homeNr );
  }

  if ( !attrs.value( "telephoneNumber" ).isEmpty() ) {
    KABC::PhoneNumber workNr = asUtf8( attrs[  "telephoneNumber" ].first() );
    workNr.setType( KABC::PhoneNumber::Work );
    addr.insertPhoneNumber( workNr );
  }

  if ( !attrs.value( "facsimileTelephoneNumber" ).isEmpty() ) {
    KABC::PhoneNumber faxNr = asUtf8( attrs[  "facsimileTelephoneNumber" ].first() );
    faxNr.setType( KABC::PhoneNumber::Fax );
    addr.insertPhoneNumber( faxNr );
  }

  if ( !attrs.value( "mobile" ).isEmpty() ) {
    KABC::PhoneNumber cellNr = asUtf8( attrs[  "mobile" ].first() );
    cellNr.setType( KABC::PhoneNumber::Cell );
    addr.insertPhoneNumber( cellNr );
  }

  if ( !attrs.value( "pager" ).isEmpty() ) {
    KABC::PhoneNumber pagerNr = asUtf8( attrs[  "pager" ].first() );
    pagerNr.setType( KABC::PhoneNumber::Pager );
    addr.insertPhoneNumber( pagerNr );
  }

  return addr;
}

#if 0 //sbesauer
KABC::DistributionList *LDAPSearchDialog::selectDistributionList()
{
  QPointer<KPIM::DistributionListPickerDialog> picker = new KPIM::DistributionListPickerDialog( mCore->addressBook(), this );
  picker->setLabelText( i18n( "Select a distribution list to add the selected contacts to." ) );
  picker->setCaption( i18n( "Select Distribution List" ) );
  picker->exec();
  KABC::DistributionList *list = mCore->addressBook()->findDistributionListByName( picker
? picker->selectedDistributionList() : QString() );
  delete picker;
  return list;
}
#endif

void LDAPSearchDialog::slotUser1()
{
  // Import selected items

  mSelectedContacts.clear();

  const QList< QPair<KLDAP::LdapAttrMap, QString> >& selectedItems = d->selectedItems( mResultView );

  if ( !selectedItems.isEmpty() ) {
    const QDateTime now = QDateTime::currentDateTime();

    for ( int i = 0; i < selectedItems.count(); ++i ) {
      KABC::Addressee contact = convertLdapAttributesToAddressee( selectedItems.at( i ).first );

      // set a comment where the contact came from
      contact.setNote( i18nc( "arguments are host name, datetime", "Imported from LDAP directory %1 on %2",
                              selectedItems.at( i ).second, KGlobal::locale()->formatDateTime( now ) ) );

      mSelectedContacts.append( contact );
    }
  }

  accept();
}

void LDAPSearchDialog::slotUser2()
{
  // Configure LDAP servers

  KCMultiDialog dialog( this );
  dialog.setCaption( i18n("Configure the Address Book LDAP Settings") );
  dialog.addModule( "kcmldap.desktop" );

  if ( dialog.exec() )
    restoreSettings();
}

#include "ldapsearchdialog.moc"
