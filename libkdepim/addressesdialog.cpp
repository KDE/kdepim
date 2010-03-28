/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of libkdepim.
 *
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *  Copyright (c) 2003 Aaron J. Seigo <aseigo@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "addressesdialog.h"

#include "distributionlist.h"

#include <kabc/contactgroup.h>
#include <kldap/ldapsearchdialog.h>
#include <kpimutils/email.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionDialog>
#include <Akonadi/EntityTreeView>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/contact/contactsfiltermodel.h>

#include <KDebug>
#include <KGlobal>
#include <KIconLoader>
#include <KInputDialog>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KRun>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KVBox>

#include <QtCore/QPointer>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItemModel>

using namespace Akonadi;

namespace KPIM {

class ContactViewItem : public QStandardItem
{
  public:
    enum ItemType
    {
      Contact,
      Group
    };

    ContactViewItem( const QString &label, const KABC::Addressee &contact )
      : QStandardItem( label ), m_type( Contact ), m_contact( contact )
    {
    }

    ContactViewItem( const QString &label, const KABC::ContactGroup &group )
      : QStandardItem( label ), m_type( Group ), m_group( group )
    {
    }

    ~ContactViewItem()
    {
    }

    ItemType itemType() const
    {
      return m_type;
    }

    KABC::Addressee contact() const
    {
      return m_contact;
    }

    KABC::ContactGroup group() const
    {
      return m_group;
    }

  private:
    const ItemType m_type;
    const KABC::Addressee m_contact;
    const KABC::ContactGroup m_group;
};

class ProxyModel : public ContactsFilterModel
{
  public:
    AddressesDialog *q;
    QMap<QString, ContactViewItem*> m_contactMap;
    QMap<QString, ContactViewItem*> m_groupMap;

    void emitLayoutChanged( const QModelIndex &index = QModelIndex() )
    {
      if( index.isValid() ) {
        Q_ASSERT( index.model() == this );
        emit dataChanged( index, index );
      } else {
        emit layoutChanged();
      }

      invalidateFilter();
    }

    explicit ProxyModel( AddressesDialog *q )
      : ContactsFilterModel( q ), q( q )
    {
    }

    ~ProxyModel()
    {
    }

  protected:
    virtual bool filterAcceptsRow( int row, const QModelIndex &parent ) const
    {
      const QModelIndex index = sourceModel()->index( row, 0, parent );
      const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.hasPayload<KABC::Addressee>() ) {
        const KABC::Addressee contact = item.payload<KABC::Addressee>();
        if ( contact.preferredEmail().isEmpty() || m_contactMap.contains( contact.preferredEmail() ) )
          return false;
      } else if ( item.hasPayload<KABC::ContactGroup>() ) {
        const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
        if ( ( group.dataCount() < 1 && group.contactReferenceCount() < 1 &&
               group.contactGroupReferenceCount() < 1 ) || m_groupMap.contains( group.id() ) )
          return false;
      }

      return ContactsFilterModel::filterAcceptsRow( row, parent );
    }
};

typedef QPair<QString, QString> ItemPair;

static void extractMailsFromGroup( const KABC::ContactGroup &group, QList<ItemPair> &mails, QStringList &groupsDone )
{
  Q_ASSERT( ! group.id().isEmpty() );
  groupsDone.append( group.id() );

  for ( uint i = 0; i < group.dataCount(); ++i )
    mails.append( ItemPair( group.data( i ).name(), group.data( i ).email() ) );

  for ( uint i = 0; i < group.contactReferenceCount(); ++i )
    mails.append( ItemPair( QString(), group.contactReference( i ).preferredEmail() ) );

  for ( uint i = 0; i < group.contactGroupReferenceCount(); ++i ) {
    const Akonadi::Item item( group.contactGroupReference( i ).uid().toLongLong() );
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item );

    job->fetchScope().fetchFullPayload();
    if ( job->exec() ) {
      Q_ASSERT( job->items().count() == 1 );
      Q_ASSERT( job->items().first().hasPayload<KABC::ContactGroup>() );
      const KABC::ContactGroup subGroup = job->items().first().payload<KABC::ContactGroup>();
      if ( !groupsDone.contains( subGroup.id() ) )
        extractMailsFromGroup( subGroup, mails, groupsDone );
    }
  }
}

static KABC::Addressee::List allContacts( QStandardItem* parent )
{
  KABC::Addressee::List contacts;

  if ( !parent )
    return contacts;

  for ( int i = parent->rowCount() - 1; i >= 0; --i ) {
    const ContactViewItem *item = dynamic_cast<ContactViewItem*>( parent->child( i ) );
    Q_ASSERT( item );

    switch ( item->itemType() ) {
      case ContactViewItem::Contact:
        contacts << item->contact();
        break;
      case ContactViewItem::Group:
        {
          QList<ItemPair> mails;
          QStringList groupsDone;

          extractMailsFromGroup( item->group(), mails, groupsDone );
          foreach ( const ItemPair &pair, mails ) {
            const QString name = pair.first;
            const QString email = pair.second;
            if ( email.isEmpty() )
              continue;

            KABC::Addressee contact;
            contact.setNameFromString( name );
            contact.insertEmail( email );
            contacts << contact;
          }
        }
        break;
    }
  }

  return contacts;
}

static QStringList allDistributionLists( QStandardItem* parent )
{
  QStringList lists;

  if ( !parent )
    return lists;

  const int count = parent->rowCount();
  for ( int i = 0; i < count; ++i) {
    const ContactViewItem *item = dynamic_cast<ContactViewItem*>( parent->child( i ) );
    Q_ASSERT( item );

    if ( item->itemType() == ContactViewItem::Group )
      lists << item->group().name();
  }

  return lists;
}

static QStringList entryToString( const KABC::Addressee::List &contacts )
{
  QStringList entries;

  foreach ( const KABC::Addressee &contact, contacts )
    entries += contact.fullEmail();

  return entries;
}

static void appendToContactGroup( QStandardItem *parentItem, KABC::ContactGroup &group )
{
  if ( ! parentItem )
    return;

  for ( int i = 0; i < parentItem->rowCount(); ++i ) {
    ContactViewItem *item = dynamic_cast<ContactViewItem*>( parentItem->child( i ) );
    Q_ASSERT( item );

    switch ( item->itemType() ) {
      case ContactViewItem::Contact:
        {
          const KABC::Addressee contact = item->contact();
          const QString id = contact.uid();
          Q_ASSERT( !id.isEmpty() );

          group.append( KABC::ContactGroup::ContactReference( id ) );
        }
        break;
      case ContactViewItem::Group:
        {
          const KABC::ContactGroup itemGroup = item->group();
          const QString id = itemGroup.id();
          Q_ASSERT( !id.isEmpty() );

          group.append( KABC::ContactGroup::ContactGroupReference( id ) );
        }
        break;
    }
  }
}


class AddressesDialog::Private
{
  public:
    Private( AddressesDialog *qq, Akonadi::Session *session )
      : q( qq ),
        m_session( session ),
        m_toItem( 0 ), m_ccItem( 0 ), m_bccItem( 0 ),
        m_ldapSearchDialog( 0 )
    {
      if ( !m_session )
        m_session = new Akonadi::Session( "AddressesDialog", q );

      Akonadi::ItemFetchScope scope;
      scope.fetchFullPayload( true );

      m_recorder = new Akonadi::ChangeRecorder( q );
      m_recorder->setSession( m_session );
      m_recorder->fetchCollection( true );
      m_recorder->setItemFetchScope( scope );
      m_recorder->setCollectionMonitored( Akonadi::Collection::root() );
      m_recorder->setMimeTypeMonitored( KABC::Addressee::mimeType(), true );
      m_recorder->setMimeTypeMonitored( KABC::ContactGroup::mimeType(), true );

      Akonadi::ContactsTreeModel *model = new Akonadi::ContactsTreeModel( m_recorder, q );
      m_availableModel = new ProxyModel( q );
      m_availableModel->setSourceModel( model );
    }

    QStandardItem* selectedToItem();
    QStandardItem* selectedCcItem();
    QStandardItem* selectedBccItem();

    void addToSelected( const Akonadi::Item& item, QStandardItem *parentItem );
    void addContactToSelected( const KABC::Addressee& contact, QStandardItem *parentItem );
    void addGroupToSelected( const KABC::ContactGroup& group, QStandardItem *parentItem );

    // private slots
    void availableSelectionChanged();
    void selectedSelectionChanged();
    void selectedCountChanged();
    void addSelectedTo();
    void addSelectedCC();
    void addSelectedBCC();
    void removeEntry();
    void searchLdap();
    void ldapSearchResult();
    void saveAsDistributionList();
    void saveAsDistributionListDone( KJob* );

    AddressesDialog *q;
    Akonadi::Session *m_session;
    Akonadi::ChangeRecorder *m_recorder;
    Akonadi::EntityTreeView *m_availableView, *m_selectedView;
    ProxyModel *m_availableModel;
    QStandardItemModel *m_selectedModel;
    QStandardItem *m_toItem, *m_ccItem, *m_bccItem;
    QPushButton *m_toButton, *m_ccButton, *m_bccButton, *m_removeButton, *m_saveButton;
    KLDAP::LdapSearchDialog  *m_ldapSearchDialog;
};

QStandardItem* AddressesDialog::Private::selectedToItem()
{
  if ( !m_toItem ) {
    m_toItem = new QStandardItem( i18n( "To" ) );
    m_selectedModel->appendRow( m_toItem );
  }

  return m_toItem;
}

QStandardItem* AddressesDialog::Private::selectedCcItem()
{
  if ( !m_ccItem ) {
    m_ccItem = new QStandardItem( i18n( "CC" ) );
    m_selectedModel->appendRow( m_ccItem );
  }

  return m_ccItem;
}

QStandardItem* AddressesDialog::Private::selectedBccItem()
{
  if ( !m_bccItem ) {
    m_bccItem = new QStandardItem( i18n( "BCC" ) );
    m_selectedModel->appendRow( m_bccItem );
  }

  return m_bccItem;
}

void AddressesDialog::Private::addToSelected( const Akonadi::Item &item, QStandardItem *parentItem )
{
  if ( item.hasPayload<KABC::Addressee>() )
    addContactToSelected( item.payload<KABC::Addressee>(), parentItem );
  else if( item.hasPayload<KABC::ContactGroup>() )
    addGroupToSelected( item.payload<KABC::ContactGroup>(), parentItem );
  else
    Q_ASSERT( false );
}

void AddressesDialog::Private::addContactToSelected( const KABC::Addressee &contact, QStandardItem *parentItem )
{
  Q_ASSERT( parentItem );

  const QString email = contact.preferredEmail();
  if ( email.isEmpty() ) {
    kDebug() << "Attendee has no email address";
    return;
  }

  if( m_availableModel->m_contactMap.contains( email ) ) {
    kDebug() << "Attendee already added, email=" << email;
    return; //already got it
  }

  const KABC::Picture photo = contact.photo();
  QString name = contact.realName().trimmed();
  if ( name.isEmpty() )
    name = email;

  ContactViewItem *item = new ContactViewItem( name, contact );
  item->setIcon( photo.isIntern() ? KIcon( QPixmap::fromImage( photo.data().scaled( QSize( 16, 16 ) ) ) )
                                  : KIcon( QLatin1String( "user-identity" ) ) );
  m_availableModel->m_contactMap[ email ] = item;
  parentItem->appendRow( item );
  m_selectedView->setExpanded( m_selectedModel->indexFromItem( parentItem ), true );
  m_availableModel->emitLayoutChanged( m_availableView->selectionModel()->currentIndex() );
}

void AddressesDialog::Private::addGroupToSelected( const KABC::ContactGroup &group, QStandardItem *parentItem )
{
  Q_ASSERT( parentItem );

  if ( m_availableModel->m_groupMap.contains( group.id() ) ) {
    kDebug() << "Group already added, email=" << group.id();
    return; //already got it
  }

  ContactViewItem *item = new ContactViewItem( group.name(), group );
  item->setIcon( KIcon( QLatin1String( "x-mail-distribution-list" ) ) );

  m_availableModel->m_groupMap[ group.id() ] = item;
  parentItem->appendRow( item );
  m_selectedView->setExpanded( m_selectedModel->indexFromItem( parentItem ), true );
  m_availableModel->emitLayoutChanged( m_availableView->selectionModel()->currentIndex() );
}

void AddressesDialog::Private::availableSelectionChanged()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  const Akonadi::Item item = index.isValid() ? index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>()
                                             : Akonadi::Item();

  const bool enabled = item.isValid() && (item.hasPayload<KABC::Addressee>() || item.hasPayload<KABC::ContactGroup>());

  m_toButton->setEnabled( enabled );
  m_ccButton->setEnabled( enabled );
  m_bccButton->setEnabled( enabled );
}

void AddressesDialog::Private::selectedSelectionChanged()
{
  const QModelIndex index = m_selectedView->selectionModel()->currentIndex();
  const ContactViewItem *item = dynamic_cast<ContactViewItem*>( m_selectedModel->itemFromIndex( index ) );

  m_removeButton->setEnabled( (item != 0) );
}

void AddressesDialog::Private::selectedCountChanged()
{
  const bool enabled = (m_toItem || m_ccItem || m_bccItem);
  m_saveButton->setEnabled( enabled );
}

void AddressesDialog::Private::addSelectedTo()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  addToSelected( item, selectedToItem() );
}

void AddressesDialog::Private::addSelectedCC()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  addToSelected( item, selectedCcItem() );
}

void AddressesDialog::Private::addSelectedBCC()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  addToSelected( item, selectedBccItem() );
}

void AddressesDialog::Private::removeEntry()
{
  const QModelIndex index = m_selectedView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );

  const ContactViewItem *item = dynamic_cast<ContactViewItem*>( m_selectedModel->itemFromIndex( index ) );
  Q_ASSERT( item );

  ContactViewItem *removedItem = 0;
  switch ( item->itemType() ) {
    case ContactViewItem::Contact:
      {
        const KABC::Addressee contact = item->contact();
        const QString email = contact.preferredEmail();
        Q_ASSERT( !email.isEmpty() );

        if ( m_availableModel->m_contactMap.contains( email ) )
          removedItem = m_availableModel->m_contactMap.take( email );
      }
      break;
    case ContactViewItem::Group:
      {
        const KABC::ContactGroup group = item->group();
        const QString id = group.id();
        Q_ASSERT( ! id.isEmpty() );

        if ( m_availableModel->m_groupMap.contains( id ) )
          removedItem = m_availableModel->m_groupMap.take( id );
      }
      break;
  }

  if ( removedItem ) {
    QStandardItem *parentItem = removedItem->parent();
    Q_ASSERT( parentItem );

    parentItem->takeRow( removedItem->row() );
    if ( parentItem->rowCount() == 0 ) {
      if ( parentItem->parent() )
        parentItem->parent()->takeRow( parentItem->row() );
      else if ( parentItem->model() == m_selectedModel )
        m_selectedModel->invisibleRootItem()->takeRow( parentItem->row() );

      if ( parentItem == m_toItem ) {
        delete m_toItem;
        m_toItem = 0;
      } else if( parentItem == m_ccItem ) {
        delete m_ccItem;
        m_ccItem = 0;
      } else if( parentItem == m_bccItem ) {
        delete m_bccItem;
        m_bccItem = 0;
      }
    }

    delete removedItem;

    m_availableModel->emitLayoutChanged();
  }
}

void AddressesDialog::Private::saveAsDistributionList()
{
  Q_ASSERT( m_toItem || m_ccItem || m_bccItem );

  QPointer<CollectionDialog> dlg = new CollectionDialog( q );
  dlg->setMimeTypeFilter( QStringList() << KABC::ContactGroup::mimeType() );
  dlg->setAccessRightsFilter( Collection::CanCreateItem );
  dlg->setCaption( i18n( "Select Address Book" ) );
  dlg->setDescription( i18n( "Select the address book the new contact group shall be saved in:" ) );

  if ( dlg->exec() == KDialog::Accepted && !dlg.isNull() ) {

    const Akonadi::Collection collection = dlg->selectedCollection();
    Q_ASSERT( collection.isValid() );

    bool ok = false;
    const QString name = KInputDialog::getText( i18n( "New Contact Group" ),
                                                i18n( "Please enter name of the new contact group:" ),
                                                QString(), &ok, q );
    if ( !ok || name.isEmpty() )
      return;

    KABC::ContactGroup group;
    group.setName( name );

    appendToContactGroup( m_toItem, group );
    appendToContactGroup( m_ccItem, group );
    appendToContactGroup( m_bccItem, group );

    Akonadi::Item item;
    item.setMimeType( KABC::ContactGroup::mimeType() );
    item.setPayload<KABC::ContactGroup>( group );

    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
    q->connect( job, SIGNAL( result( KJob* ) ), SLOT( storeDone( KJob* ) ) );
    job->start();
  }
}

void AddressesDialog::Private::saveAsDistributionListDone( KJob *job )
{
  if ( ! job->error() ) {
    KMessageBox::error( q, job->errorText() );
    return;
  }

  m_availableModel->emitLayoutChanged();
}

void AddressesDialog::Private::searchLdap()
{
  if ( !m_ldapSearchDialog ) {
    m_ldapSearchDialog = new KLDAP::LdapSearchDialog( q );
    q->connect( m_ldapSearchDialog, SIGNAL( contactsAdded() ),
                SLOT( ldapSearchResult() ) );
  }

  m_ldapSearchDialog->show();
}

void AddressesDialog::Private::ldapSearchResult()
{
  const KABC::Addressee::List contacts = m_ldapSearchDialog->selectedContacts();

  foreach ( const KABC::Addressee &contact, contacts )
    addContactToSelected( contact, selectedToItem() );
}


AddressesDialog::AddressesDialog( QWidget* parent, Akonadi::Session *session )
  : KDialog( parent ), d( new Private( this, session ) )
{
  setCaption( i18n( "EMail Address Selection" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );

  KHBox *page = new KHBox( this );
  setMainWidget( page );

  KVBox *leftBox = new KVBox( page );
  d->m_availableView = new Akonadi::EntityTreeView( leftBox );
  d->m_availableView->header()->hide();
  d->m_availableView->setRootIsDecorated( true );
  d->m_availableView->setModel( d->m_availableModel );
  d->m_availableView->setFocus();
  d->m_availableView->header()->setResizeMode( QHeaderView::ResizeToContents );
  connect( d->m_availableView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( availableSelectionChanged() ) );

  KHBox *filterBox = new KHBox( leftBox );

  QLabel *filterLabel = new QLabel( i18n( "Filter on:" ), filterBox );
  KLineEdit *filterEdit = new KLineEdit( filterBox );
  filterEdit->setClearButtonShown( true );
  connect( filterEdit, SIGNAL( textChanged( const QString& ) ),
           d->m_availableModel, SLOT( setFilterString( const QString& ) ) );
  filterLabel->setBuddy( filterEdit );

  QPushButton *ldapButton = new QPushButton( i18n( "Search Directory Service" ), leftBox );
  connect( ldapButton, SIGNAL( clicked() ), SLOT( searchLdap() ) );

  QWidget *buttonWidget = new QWidget( page );
  QVBoxLayout *buttonLayout = new QVBoxLayout( buttonWidget );
  buttonWidget->setLayout( buttonLayout );

  d->m_toButton = new QPushButton( i18n( "To >>" ), this );
  buttonLayout->addWidget( d->m_toButton );
  connect( d->m_toButton, SIGNAL( clicked() ), this, SLOT( addSelectedTo() ) );

  d->m_ccButton = new QPushButton( i18n( "CC >>" ), this );
  buttonLayout->addWidget( d->m_ccButton );
  connect( d->m_ccButton, SIGNAL( clicked() ), this, SLOT( addSelectedCC() ) );

  d->m_bccButton = new QPushButton( i18n( "BCC >>" ), this );
  buttonLayout->addWidget( d->m_bccButton );
  connect( d->m_bccButton, SIGNAL( clicked() ), this, SLOT( addSelectedBCC() ) );

  d->m_removeButton = new QPushButton( i18n("<< Remove"), this );
  buttonLayout->addWidget( d->m_removeButton );
  connect( d->m_removeButton, SIGNAL( clicked() ), this, SLOT( removeEntry() ) );

  KVBox *rightBox = new KVBox( page );

  d->m_selectedView = new Akonadi::EntityTreeView( rightBox );
  d->m_selectedView->header()->hide();
  d->m_selectedView->setRootIsDecorated( true );
  d->m_selectedModel = new QStandardItemModel( this );
  d->m_selectedView->setModel( d->m_selectedModel );
  d->m_selectedView->header()->setResizeMode( QHeaderView::ResizeToContents );
  connect( d->m_selectedView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectedSelectionChanged() ) );
  connect( d->m_selectedModel, SIGNAL( rowsInserted( const QModelIndex, int, int ) ),
           this, SLOT( selectedCountChanged() ) );
  connect( d->m_selectedModel, SIGNAL( rowsRemoved( const QModelIndex, int, int ) ),
           this, SLOT( selectedCountChanged() ) );

  d->m_saveButton = new QPushButton( i18n( "Save as Distribution List..." ), rightBox );
  d->m_saveButton->setEnabled( false );
  connect( d->m_saveButton, SIGNAL( clicked() ), this, SLOT( saveAsDistributionList() ) );

  d->availableSelectionChanged();
  d->selectedSelectionChanged();
  setInitialSize( QSize( 780, 450 ) );
}

AddressesDialog::~AddressesDialog()
{
}

void AddressesDialog::setSelectedTo( const QStringList &contents )
{
  QString name, email;
  foreach ( const QString &content, contents ) {
    KABC::Addressee::parseEmailAddress( content, name, email );

    KABC::Addressee contact;
    contact.setNameFromString( name );
    contact.insertEmail( email );

    d->addContactToSelected( contact, d->selectedToItem() );
  }
}

void AddressesDialog::setSelectedCC( const QStringList &contents )
{
  QString name, email;
  foreach ( const QString &content, contents ) {
    KABC::Addressee::parseEmailAddress( content, name, email );

    KABC::Addressee contact;
    contact.setNameFromString( name );
    contact.insertEmail( email );
    d->addContactToSelected( contact, d->selectedCcItem() );
  }
}

void AddressesDialog::setSelectedBCC( const QStringList &contents )
{
  QString name, email;
  foreach ( const QString &content, contents ) {
    KABC::Addressee::parseEmailAddress( content, name, email );

    KABC::Addressee contact;
    contact.setNameFromString( name );
    contact.insertEmail( email );
    d->addContactToSelected( contact, d->selectedBccItem() );
  }
}

void AddressesDialog::setShowCC( bool visible )
{
  d->m_ccButton->setVisible( visible );
}

void AddressesDialog::setShowBCC( bool visible )
{
  d->m_bccButton->setVisible( visible );
}

QStringList AddressesDialog::emailAddresses( ReceiverType type ) const
{
  QStringList emails;

  switch ( type ) {
    case ToReceiver:
      emails = allDistributionLists( d->m_toItem );
      break;
    case CcReceiver:
      emails = allDistributionLists( d->m_ccItem );
      break;
    case BccReceiver:
      emails = allDistributionLists( d->m_bccItem );
      break;
  }

  emails += entryToString( contacts( type ) );

  return emails;
}

KABC::Addressee::List AddressesDialog::contacts( ReceiverType type )  const
{
  switch ( type ) {
    case ToReceiver:
      return allContacts( d->m_toItem );
      break;
    case CcReceiver:
      return allContacts( d->m_ccItem );
      break;
    case BccReceiver:
      return allContacts( d->m_bccItem );
      break;
  }

  return KABC::Addressee::List();
}

KABC::Addressee::List AddressesDialog::allToAddressesNoDuplicates()  const
{
  KABC::Addressee::List aList = allContacts( d->m_toItem );
  //TODO: remove duplicates
  return aList;
}

QStringList AddressesDialog::distributionLists( ReceiverType type ) const
{
  switch ( type ) {
    case ToReceiver:
      return allDistributionLists( d->m_toItem );
      break;
    case CcReceiver:
      return allDistributionLists( d->m_ccItem );
      break;
    case BccReceiver:
      return allDistributionLists( d->m_bccItem );
      break;
  }

  return QStringList();
}

}

#include "addressesdialog.moc"
