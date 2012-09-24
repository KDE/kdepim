/*
 * This file is part of libkldap.
 *
 * Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 * Author: Steffen Hansen <hansen@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ldapsearchdialog.h"

#include "ldapclient.h"

#include <QtCore/QPair>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

#include <akonadi/collection.h>
#include <akonadi/itemcreatejob.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcmultidialog.h>
#include <kdialogbuttonbox.h>
#include <kldap/ldapobject.h>
#include <kldap/ldapserver.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

using namespace KLDAP;

static QString asUtf8( const QByteArray &val )
{
  if ( val.isEmpty() ) {
    return QString();
  }

  const char *data = val.data();

  //QString::fromUtf8() bug workaround
  if ( data[ val.size() - 1 ] == '\0' ) {
    return QString::fromUtf8( data, val.size() - 1 );
  } else {
    return QString::fromUtf8( data, val.size() );
  }
}

static QString join( const KLDAP::LdapAttrValue &lst, const QString &sep )
{
  QString res;
  bool alredy = false;
  for ( KLDAP::LdapAttrValue::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
    if ( alredy ) {
      res += sep;
    }

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
    keys[ i18n( "City" ) ] = "l"; //krazy:exclude=doublequote_chars
    keys[ i18n( "Organization" ) ] = "o"; //krazy:exclude=doublequote_chars
    keys[ i18n( "Company" ) ] = "Company";
    keys[ i18n( "Department" ) ] = "department";
    keys[ i18n( "Zip Code" ) ] = "postalCode";
    keys[ i18n( "Postal Address" ) ] = "postalAddress";
    keys[ i18n( "Description" ) ] = "description";
    keys[ i18n( "User ID" ) ] = "uid";
  }

  return keys;
}

static QString makeFilter( const QString &query, const QString &attr, bool startsWith )
{
  /* The reasoning behind this filter is:
   * If it's a person, or a distlist, show it, even if it doesn't have an email address.
   * If it's not a person, or a distlist, only show it if it has an email attribute.
   * This allows both resource accounts with an email address which are not a person and
   * person entries without an email address to show up, while still not showing things
   * like structural entries in the ldap tree. */
  QString result( "&(|(objectclass=person)(objectclass=groupofnames)(mail=*))(" );
  if ( query.isEmpty() ) {
    // Return a filter that matches everything
    return result + "|(cn=*)(sn=*)" + ')';
  }

  if ( attr == i18nc( "Search attribute: Name of contact", "Name" ) ) {
    result += startsWith ? "|(cn=%1*)(sn=%2*)" : "|(cn=*%1*)(sn=*%2*)";
    result = result.arg( query ).arg( query );
  } else {
    result += startsWith ? "%1=%2*" : "%1=*%2*";
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

static KABC::Addressee convertLdapAttributesToAddressee( const KLDAP::LdapAttrMap &attrs )
{
  KABC::Addressee addr;

  // name
  if ( !attrs.value( "cn" ).isEmpty() ) {
    addr.setNameFromString( asUtf8( attrs["cn"].first() ) );
  }

  // email
  KLDAP::LdapAttrValue lst = attrs["mail"];
  KLDAP::LdapAttrValue::ConstIterator it = lst.constBegin();
  bool pref = true;
  while ( it != lst.constEnd() ) {
    addr.insertEmail( asUtf8( *it ), pref );
    pref = false;
    ++it;
  }

  if ( !attrs.value( "o" ).isEmpty() ) {
    addr.setOrganization( asUtf8( attrs[ "o" ].first() ) );
  }
  if ( addr.organization().isEmpty() && !attrs.value( "Company" ).isEmpty() ) {
    addr.setOrganization( asUtf8( attrs[ "Company" ].first() ) );
  }

  // Address
  KABC::Address workAddr( KABC::Address::Work );

  if ( !attrs.value( "department" ).isEmpty() ) {
    addr.setDepartment( asUtf8( attrs[ "department" ].first() ) );
  }

  if ( !workAddr.isEmpty() ) {
    addr.insertAddress( workAddr );
  }

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
      if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() ) {
        return qMakePair( KLDAP::LdapAttrMap(), QString() );
      }

      return qMakePair( mContactList.at( index.row() ), mServerList.at( index.row() ) );
    }

    QString email( const QModelIndex &index ) const
    {
      if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() ) {
        return QString();
      }

      return asUtf8( mContactList.at( index.row() ).value( "mail" ).first() ).trimmed();
    }

    QString fullName( const QModelIndex &index ) const
    {
      if ( !index.isValid() || index.row() < 0 || index.row() >= mContactList.count() ) {
        return QString();
      }

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
      if ( !parent.isValid() ) {
        return mContactList.count();
      } else {
        return 0;
      }
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
      if ( !parent.isValid() ) {
        return 18;
      } else {
        return 0;
      }
    }

    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const
    {
      if ( orientation == Qt::Vertical || role != Qt::DisplayRole || section < 0 || section > 17 ) {
        return QVariant();
      }

      switch ( section ) {
        case 0:
          return i18n( "Full Name" );
          break;
        case 1:
          return i18nc( "@title:column Column containing email addresses", "Email" );
          break;
        case 2:
          return i18n( "Home Number" );
          break;
        case 3:
          return i18n( "Work Number" );
          break;
        case 4:
          return i18n( "Mobile Number" );
          break;
        case 5:
          return i18n( "Fax Number" );
          break;
        case 6:
          return i18n( "Company" );
          break;
        case 7:
          return i18n( "Organization" );
          break;
        case 8:
          return i18n( "Street" );
          break;
        case 9:
          return i18nc( "@title:column Column containing the residential state of the address",
                        "State" );
          break;
        case 10:
          return i18n( "Country" );
          break;
        case 11:
          return i18n( "Zip Code" );
          break;
        case 12:
          return i18n( "Postal Address" );
          break;
        case 13:
          return i18n( "City" );
          break;
        case 14:
          return i18n( "Department" );
          break;
        case 15:
          return i18n( "Description" );
          break;
        case 16:
          return i18n( "User ID" );
          break;
        case 17:
          return i18nc( "@title:column Column containing title of the person", "Title" );
          break;
        default:
          return QVariant();
          break;
      };

      return QVariant();
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( !index.isValid() ) {
        return QVariant();
      }

      if ( index.row() < 0 || index.row() >= mContactList.count() ||
           index.column() < 0 || index.column() > 17 ) {
        return QVariant();
      }

      if ( role == ServerRole ) {
        return mServerList.at( index.row() );
      }

      if ( role != Qt::DisplayRole ) {
        return QVariant();
      }

      const KLDAP::LdapAttrMap map = mContactList.at( index.row() );

      switch ( index.column() ) {
        case 0:
          return join( map.value( "cn" ), ", " );
          break;
        case 1:
          return join( map.value( "mail" ), ", " );
          break;
        case 2:
          return join( map.value( "homePhone" ), ", " );
          break;
        case 3:
          return join( map.value( "telephoneNumber" ), ", " );
          break;
        case 4:
          return join( map.value( "mobile" ), ", " );
          break;
        case 5:
          return join( map.value( "facsimileTelephoneNumber" ), ", " );
          break;
        case 6:
          return join( map.value( "Company" ), ", " );
          break;
        case 7:
          return join( map.value( "o" ), ", " );
          break;
        case 8:
          return join( map.value( "street" ), ", " );
          break;
        case 9:
          return join( map.value( "st" ), ", " );
          break;
        case 10:
          return join( map.value( "co" ), ", " );
          break;
        case 11:
          return join( map.value( "postalCode" ), ", " );
          break;
        case 12:
          return join( map.value( "postalAddress" ), ", " );
          break;
        case 13:
          return join( map.value( "l" ), ", " );
          break;
        case 14:
          return join( map.value( "department" ), ", " );
          break;
        case 15:
          return join( map.value( "description" ), ", " );
          break;
        case 16:
          return join( map.value( "uid" ), ", " );
          break;
        case 17:
          return join( map.value( "title" ), ", " );
          break;
        default:
          return QVariant();
          break;
      }

      return QVariant();
    }

  private:
    QList<KLDAP::LdapAttrMap> mContactList;
    QStringList mServerList;
};

static QList< QPair<KLDAP::LdapAttrMap, QString> > selectedItems( QAbstractItemView *view )
{
  QList< QPair<KLDAP::LdapAttrMap, QString> > contacts;

  ContactListModel *model = static_cast<ContactListModel*>( view->model() );

  const QModelIndexList selected = view->selectionModel()->selectedRows();
  for ( int i = 0; i < selected.count(); ++i ) {
    contacts.append( model->contact( selected.at( i ) ) );
  }

  return contacts;
}

class LdapSearchDialog::Private
{
  public:
    Private( LdapSearchDialog *qq )
      : q( qq ),
        mNumHosts( 0 ),
        mIsConfigured( false ),
        mModel( 0 )
    {
    }

    void saveSettings();
    void restoreSettings();
    void cancelQuery();

    void slotAddResult( const KLDAP::LdapClient&, const KLDAP::LdapObject& );
    void slotSetScope( bool );
    void slotStartSearch();
    void slotStopSearch();
    void slotSearchDone();
    void slotError( const QString& );
    void slotSelectAll();
    void slotUnselectAll();
    void slotSelectionChanged();

    LdapSearchDialog *q;
    int mNumHosts;
    QList<KLDAP::LdapClient*> mLdapClientList;
    bool mIsConfigured;
    KABC::Addressee::List mSelectedContacts;

    KComboBox *mFilterCombo;
    KComboBox *mSearchType;
    KLineEdit *mSearchEdit;

    QCheckBox *mRecursiveCheckbox;
    QTableView *mResultView;
    QPushButton *mSearchButton;
    ContactListModel *mModel;
};

LdapSearchDialog::LdapSearchDialog( QWidget *parent )
  : KDialog( parent ), d( new Private( this ) )
{
  setCaption( i18n( "Import Contacts from LDAP" ) );
#ifdef _WIN32_WCE
  setButtons( Help | User1 | Cancel );
#else
  setButtons( Help | User1 | User2 | Cancel );
#endif
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

  d->mSearchEdit = new KLineEdit( groupBox );
  boxLayout->addWidget( d->mSearchEdit, 0, 1 );
  label->setBuddy( d->mSearchEdit );

  label = new QLabel( i18nc( "In LDAP attribute", "in" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  d->mFilterCombo = new KComboBox( groupBox );
  d->mFilterCombo->addItem( i18nc( "@item:inlistbox Name of the contact", "Name" ) );
  d->mFilterCombo->addItem( i18nc( "@item:inlistbox email address of the contact", "Email" ) );
  d->mFilterCombo->addItem( i18nc( "@item:inlistbox", "Home Number" ) );
  d->mFilterCombo->addItem( i18nc( "@item:inlistbox", "Work Number" ) );
  boxLayout->addWidget( d->mFilterCombo, 0, 3 );

  QSize buttonSize;
  d->mSearchButton = new QPushButton( i18n( "Stop" ), groupBox );
  buttonSize = d->mSearchButton->sizeHint();
  d->mSearchButton->setText( i18nc( "@action:button Start searching", "&Search" ) );
  if ( buttonSize.width() < d->mSearchButton->sizeHint().width() ) {
    buttonSize = d->mSearchButton->sizeHint();
  }
  d->mSearchButton->setFixedWidth( buttonSize.width() );

  d->mSearchButton->setDefault( true );
  boxLayout->addWidget( d->mSearchButton, 0, 4 );

  d->mRecursiveCheckbox = new QCheckBox( i18n( "Recursive search" ), groupBox );
  d->mRecursiveCheckbox->setChecked( true );
  boxLayout->addWidget( d->mRecursiveCheckbox, 1, 0, 1, 5 );

  d->mSearchType = new KComboBox( groupBox );
  d->mSearchType->addItem( i18n( "Contains" ) );
  d->mSearchType->addItem( i18n( "Starts With" ) );
  boxLayout->addWidget( d->mSearchType, 1, 3, 1, 2 );

  topLayout->addWidget( groupBox );

  d->mResultView = new QTableView( page );
  d->mResultView->setSelectionMode( QTableView::MultiSelection );
  d->mResultView->setSelectionBehavior( QTableView::SelectRows );
  d->mModel = new ContactListModel( d->mResultView );
  d->mResultView->setModel( d->mModel );
  d->mResultView->verticalHeader()->hide();
  connect( d->mResultView, SIGNAL(clicked(QModelIndex)),
           SLOT(slotSelectionChanged()) );
  topLayout->addWidget( d->mResultView );

  KDialogButtonBox *buttons = new KDialogButtonBox( page, Qt::Horizontal );
  buttons->addButton( i18n( "Select All" ),
                      QDialogButtonBox::ActionRole, this, SLOT(slotSelectAll()) );
  buttons->addButton( i18n( "Unselect All" ),
                      QDialogButtonBox::ActionRole, this, SLOT(slotUnselectAll()) );

  topLayout->addWidget( buttons );

  resize( QSize( 600, 400 ).expandedTo( minimumSizeHint() ) );

  setButtonText( User1, i18n( "Add Selected" ) );
#ifndef _WIN32_WCE
  setButtonText( User2, i18n( "Configure LDAP Servers..." ) );
#endif

  connect( d->mRecursiveCheckbox, SIGNAL(toggled(bool)),
           this, SLOT(slotSetScope(bool)) );
  connect( d->mSearchButton, SIGNAL(clicked()),
           this, SLOT(slotStartSearch()) );

  setTabOrder( d->mSearchEdit, d->mFilterCombo );
  setTabOrder( d->mFilterCombo, d->mSearchButton );
  d->mSearchEdit->setFocus();

  connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
#ifndef _WIN32_WCE
  connect( this, SIGNAL(user2Clicked()), this, SLOT(slotUser2()) );
#endif
  d->slotSelectionChanged();
  d->restoreSettings();
}

LdapSearchDialog::~LdapSearchDialog()
{
  d->saveSettings();
  delete d;
}

void LdapSearchDialog::setSearchText( const QString &text )
{
  d->mSearchEdit->setText( text );
}

KABC::Addressee::List LdapSearchDialog::selectedContacts() const
{
  return d->mSelectedContacts;
}

void LdapSearchDialog::Private::slotSelectionChanged()
{
  q->enableButton( KDialog::User1, mResultView->selectionModel()->hasSelection() );
}

void LdapSearchDialog::Private::restoreSettings()
{
  // Create one KLDAP::LdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at
  // the beginning of the process
  qDeleteAll( mLdapClientList ) ;
  mLdapClientList.clear();

  KConfig *config = KLDAP::LdapClientSearch::config();

  KConfigGroup searchGroup( config, "LDAPSearch" );
  mSearchType->setCurrentIndex( searchGroup.readEntry( "SearchType", 0 ) );

  // then read the config file and register all selected
  // server in the list
  KConfigGroup group( config, "LDAP" );
  mNumHosts = group.readEntry( "NumSelectedHosts", 0 );
  if ( !mNumHosts ) {
    mIsConfigured = false;
  } else {
    mIsConfigured = true;
    for ( int j = 0; j < mNumHosts; ++j ) {
      KLDAP::LdapServer ldapServer;
      KLDAP::LdapClient *ldapClient = new KLDAP::LdapClient( 0, q );
      KLDAP::LdapClientSearch::readConfig( ldapServer, group, j, true );
      ldapClient->setServer( ldapServer );
      QStringList attrs;

      for ( QMap<QString, QString>::ConstIterator it = adrbookattr2ldap().constBegin();
            it != adrbookattr2ldap().constEnd(); ++it ) {
        attrs << *it;
      }

      ldapClient->setAttributes( attrs );

      q->connect( ldapClient, SIGNAL(result(KLDAP::LdapClient,KLDAP::LdapObject)),
                  q, SLOT(slotAddResult(KLDAP::LdapClient,KLDAP::LdapObject)) );
      q->connect( ldapClient, SIGNAL(done()),
                  q, SLOT(slotSearchDone()) );
      q->connect( ldapClient, SIGNAL(error(QString)),
                  q, SLOT(slotError(QString)) );

      mLdapClientList.append( ldapClient );
    }

    mModel->clear();
  }
}

void LdapSearchDialog::Private::saveSettings()
{
  KConfig *config = KLDAP::LdapClientSearch::config();
  KConfigGroup group( config, "LDAPSearch" );
  group.writeEntry( "SearchType", mSearchType->currentIndex() );
  group.sync();
}

void LdapSearchDialog::Private::cancelQuery()
{
  Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
    client->cancelQuery();
  }
}

void LdapSearchDialog::Private::slotAddResult( const KLDAP::LdapClient &client,
                                               const KLDAP::LdapObject &obj )
{
  mModel->addContact( obj.attributes(), client.server().host() );
}

void LdapSearchDialog::Private::slotSetScope( bool rec )
{
    Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
    if ( rec ) {
      client->setScope( "sub" );
    } else {
      client->setScope( "one" );
    }
  }
}

void LdapSearchDialog::Private::slotStartSearch()
{
  cancelQuery();

  if ( !mIsConfigured ) {
    KMessageBox::error( q, i18n( "You must select an LDAP server before searching." ) );
    q->slotUser2();
    return;
  }

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor( Qt::WaitCursor );
#endif
  mSearchButton->setText( i18n( "Stop" ) );

  q->disconnect( mSearchButton, SIGNAL(clicked()),
                 q, SLOT(slotStartSearch()) );
  q->connect( mSearchButton, SIGNAL(clicked()),
              q, SLOT(slotStopSearch()) );

  const bool startsWith = (mSearchType->currentIndex() == 1);

  const QString filter = makeFilter( mSearchEdit->text().trimmed(),
                                     mFilterCombo->currentText(), startsWith );

   // loop in the list and run the KLDAP::LdapClients
  mModel->clear();
  Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
    client->startQuery( filter );
  }

  saveSettings();
}

void LdapSearchDialog::Private::slotStopSearch()
{
  cancelQuery();
  slotSearchDone();
}

void LdapSearchDialog::Private::slotSearchDone()
{
  // If there are no more active clients, we are done.
  Q_FOREACH( KLDAP::LdapClient *client, mLdapClientList ) {
    if ( client->isActive() ) {
      return;
    }
  }

  q->disconnect( mSearchButton, SIGNAL(clicked()),
                 q, SLOT(slotStopSearch()) );
  q->connect( mSearchButton, SIGNAL(clicked()),
              q, SLOT(slotStartSearch()) );

  mSearchButton->setText( i18nc( "@action:button Start searching", "&Search" ) );
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif
}

void LdapSearchDialog::Private::slotError( const QString &error )
{
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif
  KMessageBox::error( q, error );
}

void LdapSearchDialog::closeEvent( QCloseEvent *e )
{
  d->slotStopSearch();
  e->accept();
}

void LdapSearchDialog::Private::slotUnselectAll()
{
  mResultView->clearSelection();
}

void LdapSearchDialog::Private::slotSelectAll()
{
  mResultView->selectAll();
}

void LdapSearchDialog::slotUser1()
{
  // Import selected items

  d->mSelectedContacts.clear();

  const QList< QPair<KLDAP::LdapAttrMap, QString> >& items = selectedItems( d->mResultView );

  if ( !items.isEmpty() ) {
    const QDateTime now = QDateTime::currentDateTime();

    for ( int i = 0; i < items.count(); ++i ) {
      KABC::Addressee contact = convertLdapAttributesToAddressee( items.at( i ).first );

      // set a comment where the contact came from
      contact.setNote( i18nc( "arguments are host name, datetime",
                              "Imported from LDAP directory %1 on %2",
                              items.at( i ).second, KGlobal::locale()->formatDateTime( now ) ) );

      d->mSelectedContacts.append( contact );
    }
  }

  emit contactsAdded();

  accept();
}

void LdapSearchDialog::slotUser2()
{
  // Configure LDAP servers

  KCMultiDialog dialog( this );
  dialog.setCaption( i18n( "Configure the Address Book LDAP Settings" ) );
  dialog.addModule( "kcmldap.desktop" );

  if ( dialog.exec() ) { //krazy:exclude=crashy
    d->restoreSettings();
  }
}

#include "ldapsearchdialog.moc"
