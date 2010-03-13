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
#include "ldapsearchdialog.h"
#include "distributionlist.h"

#include "contactstreemodel.h"
#include "contactsfilterproxymodel.h"

#include <kpimutils/email.h>
#ifndef KDEPIM_NO_KRESOURCES
#include <kresources/selectdialog.h>

#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#endif
#include <kabc/contactgroup.h>

#include <Akonadi/Session>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/EntityTreeView>
#include <Akonadi/CollectionDialog>

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

#include <QLayout>
#include <QLabel>
#include <QWidget>
#include <QPixmap>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QPointer>

using namespace Akonadi;

namespace KPIM {

class AddresseeViewItem : public QStandardItem
{
  public:
    enum ItemType { Address, Group };
    AddresseeViewItem( const QString &label, const KABC::Addressee& addr )
      : QStandardItem( label ), m_type( Address ), m_addr( addr ) {}
    AddresseeViewItem( const QString &label, const KABC::ContactGroup& group )
      : QStandardItem( label ), m_type( Group ), m_group( group ) {}
    ~AddresseeViewItem() {}
    ItemType itemType() const {
      return m_type;
    }
    const KABC::Addressee& addressee() {
      return m_addr;
    }
    const KABC::ContactGroup& group() {
      return m_group;
    }
  private:
    const ItemType m_type;
    const KABC::Addressee m_addr;
    const KABC::ContactGroup m_group;
};

#if 0
// private start :
struct AddresseeViewItem::AddresseeViewItemPrivate {
  KABC::Addressee               address;
  AddresseeViewItem::Category   category;
  KABC::Addressee::List         addresses;
};

class AddressPickerUI : public QWidget, public Ui::AddressPickerUI
{
public:
  AddressPickerUI( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

struct AddressesDialog::AddressesDialogPrivate {
  AddressesDialogPrivate() :
    ui(0), personal(0), recent(0),
    toItem(0), ccItem(0), bccItem(0),
    ldapSearchDialog(0)
  {}

  AddresseeViewItem *personal;
  AddresseeViewItem *recent;

  AddresseeViewItem *toItem;
  AddresseeViewItem *ccItem;
  AddresseeViewItem *bccItem;

  QHash<QString, AddresseeViewItem*> groupDict;

  KABC::Addressee::List recentAddresses;
  LdapSearchDialog  *ldapSearchDialog;
};
// privates end

AddresseeViewItem::AddresseeViewItem( AddresseeViewItem *parent, const KABC::Addressee& addr,
                                      int emailIndex )
  : QObject( 0 ), QTreeWidgetItem( parent )
{
  d = new AddresseeViewItemPrivate;
  d->address = addr;
  d->category = Entry;

  setText( 0, addr.realName() );
  setText( 1, ( emailIndex == 0 ? addr.preferredEmail() : addr.emails()[ emailIndex ] ) );
  if ( text( 0 ).trimmed().isEmpty() )
    setText( 0, addr.preferredEmail() );

  if ( addr.photo().url().isEmpty() ) {
    if ( addr.photo().data().isNull() )
      setIcon( 0, KIconLoader::global()->loadIcon( "x-office-contact", KIconLoader::Small ) );
    else
      setIcon( 0, QPixmap::fromImage( addr.photo().data().scaled( 16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation
) ) );
  } else {
    setIcon( 0, KIconLoader::global()->loadIcon( addr.photo().url(), KIconLoader::Small ) );
  }
}

AddresseeViewItem::AddresseeViewItem( QTreeWidget *lv, const QString& name, Category cat )
  : QObject(0), QTreeWidgetItem( lv )
{
  d = new AddresseeViewItemPrivate;
  d->category = cat;
  setText( 0, name );
}

AddresseeViewItem::AddresseeViewItem(  AddresseeViewItem *parent, const QString& name,
                                       const KABC::Addressee::List &lst )
  : QObject(0), QTreeWidgetItem( parent )
{
  d = new AddresseeViewItemPrivate;
  d->category = FilledGroup;
  d->addresses = lst;
  setText( 0, name );
  setText( 1, i18n("<group>") );
}

AddresseeViewItem::AddresseeViewItem(  AddresseeViewItem *parent, const QString &name )
  : QObject( 0 ), QTreeWidgetItem( parent )
{
  d = new AddresseeViewItemPrivate;
  d->category = DistList;

  setIcon( 0, KIconLoader::global()->loadIcon( "x-mail-distribution-list", KIconLoader::Small ) );
  setText( 0, name );
  setText( 1, i18n( "<group>" ) );
}

AddresseeViewItem::~AddresseeViewItem()
{
  delete d;
  d = 0;
}

KABC::Addressee
AddresseeViewItem::addressee() const
{
  return d->address;
}

KABC::Addressee::List
AddresseeViewItem::addresses() const
{
  return d->addresses;
}

AddresseeViewItem::Category
AddresseeViewItem::category() const
{
  return d->category;
}

QString
AddresseeViewItem::name() const
{
  return text(0);
}

QString
AddresseeViewItem::email() const
{
  return text(1);
}

bool AddresseeViewItem::matches(const QString& txt) const
{
    return d->address.realName().contains(txt, Qt::CaseInsensitive)
        || d->address.preferredEmail().contains(txt, Qt::CaseInsensitive);
}

bool AddresseeViewItem::operator < ( const QTreeWidgetItem& other ) const
{
  if ( category() == Group || category() == Entry )
    return QTreeWidgetItem::operator < ( other );

  // We define our custom sort behavior so that the To, CC and BCC groups are always
  // sorted in that order, regardless of actual sort direction.

  const AddresseeViewItem &item = static_cast<const AddresseeViewItem&>( other );
  int a = static_cast<int>( category() );
  int b = static_cast<int>( item.category() );

  Qt::SortOrder sortOrder = treeWidget()->header()->sortIndicatorOrder();

  if ( sortOrder == Qt::AscendingOrder )
    return ( a < b );
  else
    return ( a > b );
}
#endif

class ProxyModel : public ContactsFilterModel
{
  public:
    AddressesDialog *q;
    QMap<QString, AddresseeViewItem*> m_addressMap;
    QMap<QString, AddresseeViewItem*> m_groupMap;

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

    explicit ProxyModel( AddressesDialog *q ) : ContactsFilterModel( q ), q( q )
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
      if( item.hasPayload<KABC::Addressee>() ) {
        const KABC::Addressee addr = item.payload<KABC::Addressee>();
        if( addr.preferredEmail().isEmpty() || m_addressMap.contains( addr.preferredEmail() ) )
          return false;
      } else if( item.hasPayload<KABC::ContactGroup>() ) {
        const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
        if( ( group.dataCount() < 1 && group.contactReferenceCount() < 1 && group.contactGroupReferenceCount() < 1 ) || m_groupMap.contains( group.id() ) )
          return false;
      }
      return ContactsFilterModel::filterAcceptsRow( row, parent );
    }
};

AddressesDialog::AddressesDialog( QWidget* parent, Akonadi::Session *session )
  : KDialog( parent )
  , m_session( session )
  , m_toItem( 0 ), m_ccItem( 0 ), m_bccItem( 0 )
  , m_ldapSearchDialog( 0 )
{
  setCaption( i18n( "Address Selection" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  KHBox *page = new KHBox( this );
  setMainWidget( page );

  if( ! m_session ) {
    m_session = new Akonadi::Session( "AddressesDialog", this );
  }

  Akonadi::ItemFetchScope scope;
  scope.fetchFullPayload( true );
  //scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

  m_recorder = new Akonadi::ChangeRecorder( this );
  m_recorder->setSession( m_session );
  m_recorder->fetchCollection( true );
  m_recorder->setItemFetchScope( scope );
  m_recorder->setCollectionMonitored( Akonadi::Collection::root() );
  m_recorder->setMimeTypeMonitored( KABC::Addressee::mimeType(), true );
  m_recorder->setMimeTypeMonitored( KABC::ContactGroup::mimeType(), true );

  Akonadi::ContactsTreeModel *model = new Akonadi::ContactsTreeModel( m_recorder, this );
  m_availableModel = new ProxyModel( this );
  m_availableModel->setSourceModel( model );

  KVBox *leftbox = new KVBox( page );
  m_availableView = new Akonadi::EntityTreeView( leftbox );
  m_availableView->header()->hide();
  m_availableView->setRootIsDecorated( true );
  m_availableView->setModel( m_availableModel );
  m_availableView->setFocus();
  m_availableView->header()->setResizeMode( QHeaderView::ResizeToContents );
  connect( m_availableView->selectionModel(), SIGNAL( selectionChanged(QItemSelection,QItemSelection) ), SLOT( availableSelectionChanged() ) );

  KHBox *filterbox = new KHBox( leftbox );
  QLabel *filterlabel = new QLabel( i18n("Filter on:"), filterbox );
  KLineEdit *filteredit = new KLineEdit( filterbox );
  filteredit->setClearButtonShown(true);
  connect( filteredit, SIGNAL(textChanged(QString)), m_availableModel, SLOT(setFilterString(QString)) );
  filterlabel->setBuddy( filteredit );

  QPushButton *ldapbtn = new QPushButton( i18n("Search Directory Service"), leftbox );
  connect( ldapbtn, SIGNAL(clicked()), SLOT(searchLdap())  );

  QWidget *btnwidget = new QWidget( page );
  QVBoxLayout *btnlayout = new QVBoxLayout( btnwidget );
  btnwidget->setLayout( btnlayout );

  m_tobtn = new QPushButton( i18n("To >>"), this );
  btnlayout->addWidget( m_tobtn );
  connect( m_tobtn, SIGNAL(clicked()), this, SLOT(addSelectedTo()) );
  m_ccbtn = new QPushButton( i18n("CC >>"), this );
  btnlayout->addWidget( m_ccbtn );
  connect( m_ccbtn, SIGNAL(clicked()), this, SLOT(addSelectedCC()) );
  m_bccbtn = new QPushButton( i18n("BCC >>"), this );
  btnlayout->addWidget( m_bccbtn );
  connect( m_bccbtn, SIGNAL(clicked()), this, SLOT(addSelectedBCC()) );
  m_rembtn = new QPushButton( i18n("<< Remove"), this );
  btnlayout->addWidget( m_rembtn );
  connect( m_rembtn, SIGNAL(clicked()), this, SLOT(removeEntry()) );

  KVBox *rightbox = new KVBox( page );

  m_selectedView = new Akonadi::EntityTreeView( rightbox );
  m_selectedView->header()->hide();
  m_selectedView->setRootIsDecorated( true );
  m_selectedModel = new QStandardItemModel( this );
  m_selectedView->setModel( m_selectedModel );
  m_selectedView->header()->setResizeMode( QHeaderView::ResizeToContents );
  connect( m_selectedView->selectionModel(), SIGNAL( selectionChanged(QItemSelection,QItemSelection) ), SLOT( selectedSelectionChanged() ) );
  connect( m_selectedModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(selectedCountChanged())  );
  connect( m_selectedModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(selectedCountChanged())  );

  m_savebtn = new QPushButton( i18n("Save as Distribution List..."), rightbox );
  m_savebtn->setEnabled( false );
  connect( m_savebtn, SIGNAL(clicked()), this, SLOT(saveAsDistributionList()) );

  availableSelectionChanged();
  selectedSelectionChanged();
  setInitialSize( QSize( 780, 450 ) );
}

AddressesDialog::~AddressesDialog()
{
}

void AddressesDialog::availableSelectionChanged()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  const Akonadi::Item item = index.isValid() ? index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>() : Akonadi::Item();
  const bool enabled = item.isValid() && ( item.hasPayload<KABC::Addressee>() || item.hasPayload<KABC::ContactGroup>() );
  m_tobtn->setEnabled( enabled );
  m_ccbtn->setEnabled( enabled );
  m_bccbtn->setEnabled( enabled );
}

void AddressesDialog::selectedSelectionChanged()
{
  const QModelIndex index = m_selectedView->selectionModel()->currentIndex();
  AddresseeViewItem *item = dynamic_cast<AddresseeViewItem*>( m_selectedModel->itemFromIndex( index ) );
  const bool enabled = item != 0;
  m_rembtn->setEnabled( enabled );
}

void AddressesDialog::selectedCountChanged()
{
  const bool enabled = m_toItem || m_ccItem || m_bccItem;
  m_savebtn->setEnabled( enabled );
}

void AddressesDialog::addSelectedTo()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );
  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  addToSelected( item, selectedToItem() );
}

void AddressesDialog::addSelectedCC()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );
  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  addToSelected( item, selectedCcItem() );
}

void AddressesDialog::addSelectedBCC()
{
  const QModelIndex index = m_availableView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );
  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  addToSelected( item, selectedBccItem() );
}

void AddressesDialog::removeEntry()
{
  const QModelIndex index = m_selectedView->selectionModel()->currentIndex();
  Q_ASSERT( index.isValid() );
  AddresseeViewItem *item = dynamic_cast<AddresseeViewItem*>( m_selectedModel->itemFromIndex( index ) );
  Q_ASSERT( item );
  AddresseeViewItem *removedItem = 0;
  switch( item->itemType() ) {
    case AddresseeViewItem::Address: {
      KABC::Addressee addr = item->addressee();
      const QString email = addr.preferredEmail();
      Q_ASSERT( ! email.isEmpty() );
      if( m_availableModel->m_addressMap.contains( email ) ) {
        removedItem = m_availableModel->m_addressMap.take( email );
      }
    } break;
    case AddresseeViewItem::Group: {
      KABC::ContactGroup group = item->group();
      const QString id = group.id();
      Q_ASSERT( ! id.isEmpty() );
      if( m_availableModel->m_groupMap.contains( id ) ) {
        removedItem = m_availableModel->m_groupMap.take( id );
      }
    } break;
  }
  if( removedItem ) {
    QStandardItem *parentitem = removedItem->parent();
    Q_ASSERT( parentitem );
    parentitem->takeRow( removedItem->row() );
    if( parentitem->rowCount() == 0 ) {
      if( parentitem->parent() ) {
        parentitem->parent()->takeRow( parentitem->row() );
      } else if( parentitem->model() == m_selectedModel ) {
        m_selectedModel->invisibleRootItem()->takeRow( parentitem->row() );
      }
      if( parentitem == m_toItem ) {
        delete m_toItem;
        m_toItem = 0;
      } else if( parentitem == m_ccItem ) {
        delete m_ccItem;
        m_ccItem = 0;
      } else if( parentitem == m_bccItem ) {
        delete m_bccItem;
        m_bccItem = 0;
      }
    }
    delete removedItem;
    m_availableModel->emitLayoutChanged();
  }
}

QStandardItem* AddressesDialog::selectedToItem()
{
  if( ! m_toItem ) {
    m_toItem = new QStandardItem( i18n("To") );
    m_selectedModel->appendRow( m_toItem );
  }
  return m_toItem;
}

QStandardItem* AddressesDialog::selectedCcItem()
{
  if( ! m_ccItem ) {
    m_ccItem = new QStandardItem( i18n("CC") );
    m_selectedModel->appendRow( m_ccItem );
  }
  return m_ccItem;
}

QStandardItem* AddressesDialog::selectedBccItem()
{
  if( ! m_bccItem ) {
    m_bccItem = new QStandardItem( i18n("BCC") );
    m_selectedModel->appendRow( m_bccItem );
  }
  return m_bccItem;
}

void
AddressesDialog::setSelectedTo( const QStringList& l )
{
  QString name, email;
  for ( QStringList::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
    KABC::Addressee::parseEmailAddress( *it, name, email );
    KABC::Addressee addr;
    addr.setNameFromString( name );
    addr.insertEmail( email );
    addAddresseeToSelected( addr, selectedToItem() );
  }
}

void
AddressesDialog::setSelectedCC( const QStringList& l )
{
  QString name, email;
  for ( QStringList::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
    KABC::Addressee::parseEmailAddress( *it, name, email );
    KABC::Addressee addr;
    addr.setNameFromString( name );
    addr.insertEmail( email );
    addAddresseeToSelected( addr, selectedCcItem() );
  }
}

void
AddressesDialog::setSelectedBCC( const QStringList& l )
{
  QString name, email;
  for ( QStringList::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
    KABC::Addressee::parseEmailAddress( *it, name, email );
    KABC::Addressee addr;
    addr.setNameFromString( name );
    addr.insertEmail( email );
    addAddresseeToSelected( addr, selectedBccItem() );
  }
}

#if 0
void
AddressesDialog::setRecentAddresses( const KABC::Addressee::List& list )
{
  d->recentAddresses = list;

  updateRecentAddresses();

  checkForSingleAvailableGroup();
}

void
AddressesDialog::updateRecentAddresses()
{
  static const QString &recentGroup = KGlobal::staticQString( i18n( "Recent Addresses" ) );

  if ( !d->recent ) {
    d->recent = new AddresseeViewItem( m_ui->mAvailableView, recentGroup );
    d->recent->setHidden( true );
    d->groupDict.insert( recentGroup, d->recent );
  }

  KABC::Addressee::List::ConstIterator it;
  for ( it = d->recentAddresses.constBegin(); it != d->recentAddresses.constEnd(); ++it )
    addAddresseeToAvailable( *it, d->recent );

  if ( d->recent->childCount() > 0 ) {
    d->recent->setHidden( false );
  }
}
#endif

void
AddressesDialog::setShowCC( bool b )
{
  m_ccbtn->setVisible( b );
}

void
AddressesDialog::setShowBCC( bool b )
{
  m_bccbtn->setVisible( b );
}

QStringList
AddressesDialog::to() const
{
  QStringList emails = allDistributionLists( m_toItem );
  KABC::Addressee::List l = toAddresses();
  emails += entryToString( l );
  return emails;
}

QStringList
AddressesDialog::cc() const
{
  QStringList emails = allDistributionLists( m_ccItem );
  KABC::Addressee::List l = ccAddresses();
  emails += entryToString( l );
  return emails;
}

QStringList
AddressesDialog::bcc() const
{
  QStringList emails = allDistributionLists( m_bccItem );
  KABC::Addressee::List l = bccAddresses();
  emails += entryToString( l );
  return emails;
}

KABC::Addressee::List
AddressesDialog::toAddresses()  const
{
  return allAddressee( m_toItem );
}

KABC::Addressee::List
AddressesDialog::allToAddressesNoDuplicates()  const
{
  KABC::Addressee::List aList = allAddressee( m_toItem );
#if 0
  const QStringList dList = toDistributionLists();
  KABC::AddressBook* abook = KABC::StdAddressBook::self( true );
  for ( QStringList::ConstIterator it = dList.constBegin(); it != dList.constEnd(); ++it ) {
    const KABC::DistributionList *list = abook->findDistributionListByName( *it );
    if ( !list )
      continue;
    const KABC::DistributionList::Entry::List eList = list->entries();
    KABC::DistributionList::Entry::List::ConstIterator eit;
    for( eit = eList.constBegin(); eit != eList.constEnd(); ++eit ) {
      KABC::Addressee a = (*eit).addressee();
      if ( !a.preferredEmail().isEmpty() && !aList.contains( a ) ) {
        aList.append( a ) ;
      }
    }
  }
#endif
  return aList;
}

KABC::Addressee::List
AddressesDialog::ccAddresses()  const
{
  return allAddressee( m_ccItem );
}

KABC::Addressee::List
AddressesDialog::bccAddresses()  const
{
  return allAddressee( m_bccItem );
}

QStringList
AddressesDialog::toDistributionLists() const
{
  return allDistributionLists( m_toItem );
}

QStringList
AddressesDialog::ccDistributionLists() const
{
  return allDistributionLists( m_ccItem );
}

QStringList
AddressesDialog::bccDistributionLists() const
{
  return allDistributionLists( m_bccItem );
}

#if 0
void
AddressesDialog::updateAvailableAddressees()
{
  m_ui->mAvailableView->clear();
  d->groupDict.clear();

  static const QString &personalGroup = KGlobal::staticQString( i18n( "Other Addresses" ) );
  d->personal = new AddresseeViewItem( m_ui->mAvailableView, personalGroup );
  d->personal->setHidden( true );
  d->groupDict.insert( personalGroup, d->personal );

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  for( KABC::AddressBook::Iterator it = addressBook->begin();
       it != addressBook->end(); ++it ) {
    addAddresseeToAvailable( *it, d->personal );
  }

  d->recent = 0;
  updateRecentAddresses();

  addDistributionLists();
  if ( d->personal->childCount() > 0 ) {
    d->personal->setHidden( false );
  }

  checkForSingleAvailableGroup();
}

void AddressesDialog::checkForSingleAvailableGroup()
{
  int itemIndex = 0;
  QTreeWidgetItem* item = m_ui->mAvailableView->topLevelItem( itemIndex );
  QTreeWidgetItem* firstGroup = 0;
  int found = 0;
  while ( item )
  {
    if ( !item->isHidden() )
    {
      if ( !firstGroup &&
           static_cast<AddresseeViewItem*>( item )->category() != AddresseeViewItem::Entry )
      {
        firstGroup = item;
      }
      ++found;
    }
    item = m_ui->mAvailableView->topLevelItem( ++itemIndex );
  }

  if ( found == 1 && firstGroup )
  {
    m_ui->mAvailableView->expandItem( firstGroup );
  }
}

void
AddressesDialog::availableSelectionChanged()
{
  bool selection = !m_ui->mAvailableView->selectedItems().isEmpty();
  m_ui->mToButton->setEnabled(selection);
  m_ui->mCCButton->setEnabled(selection);
  m_ui->mBCCButton->setEnabled(selection);
}

void
AddressesDialog::selectedSelectionChanged()
{
  bool selection = !m_ui->mSelectedView->selectedItems().isEmpty();
  m_ui->mRemoveButton->setEnabled(selection);
}

void
AddressesDialog::initConnections()
{
  connect( m_ui->mFilterEdit, SIGNAL(textChanged(const QString &)),
           SLOT(filterChanged(const QString &)) );
  connect( m_ui->mToButton, SIGNAL(clicked()),
           SLOT(addSelectedTo()) );
  connect( m_ui->mCCButton, SIGNAL(clicked()),
           SLOT(addSelectedCC()) );
  connect( m_ui->mBCCButton, SIGNAL(clicked()),
           SLOT(addSelectedBCC())  );
  connect( m_ui->mSaveAs, SIGNAL(clicked()),
           SLOT(saveAs())  );
  connect( m_ui->mRemoveButton, SIGNAL(clicked()),
           SLOT(removeEntry()) );
  connect( m_ui->mAvailableView, SIGNAL(itemSelectionChanged()),
           SLOT(availableSelectionChanged())  );
  connect( m_ui->mAvailableView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
           SLOT(addSelectedTo()) );
  connect( m_ui->mSelectedView, SIGNAL(itemSelectionChanged()),
           SLOT(selectedSelectionChanged()) );
  connect( m_ui->mSelectedView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
           SLOT(removeEntry()) );
  connect( KABC::StdAddressBook::self( true ), SIGNAL( addressBookChanged(AddressBook*) ),
           this, SLOT( updateAvailableAddressees() ) );
}

void
AddressesDialog::addAddresseeToAvailable( const KABC::Addressee& addr,
                                          AddresseeViewItem* defaultParent, bool useCategory )
{
  if ( addr.preferredEmail().isEmpty() )
    return;

  if ( useCategory ) {
    QStringList categories = addr.categories();

    for ( QStringList::Iterator it = categories.begin(); it != categories.end(); ++it ) {
      if ( !d->groupDict.contains( *it ) ) {  //we don't have the category yet
        AddresseeViewItem* category = new AddresseeViewItem( m_ui->mAvailableView, *it );
        d->groupDict.insert( *it,  category );
      }

      for ( int i = 0; i < addr.emails().count(); ++i ) {
        new AddresseeViewItem( d->groupDict.value( *it ), addr, i );
      }
    }
  }

  bool noCategory = false;
  if ( useCategory ) {
    if ( addr.categories().isEmpty() )
      noCategory = true;
  } else
    noCategory = true;

  if ( defaultParent && noCategory ) { // only non-categorized items here
    new AddresseeViewItem( defaultParent, addr );
  }
}
#endif

void
AddressesDialog::addToSelected( const Akonadi::Item& item, QStandardItem *parentItem )
{
  if( item.hasPayload<KABC::Addressee>() ) {
    addAddresseeToSelected( item.payload<KABC::Addressee>(), parentItem );
  } else if( item.hasPayload<KABC::ContactGroup>() ) {
    addGroupToSelected( item.payload<KABC::ContactGroup>(), parentItem );
  } else {
    Q_ASSERT( false );
  }
}

void
AddressesDialog::addAddresseeToSelected( const KABC::Addressee& addr, QStandardItem *parentItem )
{
#if 0
  if ( addr.preferredEmail().isEmpty() ) {
    return;
  }  int childIndex = 0;
  AddresseeViewItem *myChild = static_cast<AddresseeViewItem*>( defaultParent->QTreeWidgetItem::child( childIndex ) );
  while( myChild ) {
    if ( myChild->addressee().preferredEmail() == addr.preferredEmail() )
      return;
    myChild = static_cast<AddresseeViewItem*>(
        defaultParent->QTreeWidgetItem::child( ++childIndex ) );
  }
  new AddresseeViewItem( defaultParent, addr );
  m_ui->mSelectedView->
  m_ui->mSaveAs->setEnabled(true);
#else
  Q_ASSERT( parentItem );
  const QString email = addr.preferredEmail();
  if ( email.isEmpty() ) {
    kDebug() << "Attendee has no email address";
    return;
  }
  if( m_availableModel->m_addressMap.contains( email ) ) {
    kDebug() << "Attendee already added, email=" << email;
    return; //already got it
  }
  KABC::Picture pic = addr.photo();
  QString name = addr.realName().trimmed();
  if( name.isEmpty() ) {
    name = email;
  }
  AddresseeViewItem *item = new AddresseeViewItem( name, addr );
  item->setIcon( pic.isIntern()
                    ? KIcon( QPixmap::fromImage( pic.data().scaled( QSize( 16, 16 ) ) ) )
                    : KIcon( QLatin1String( "x-office-contact" ) ) );
  m_availableModel->m_addressMap[ email ] = item;
  parentItem->appendRow( item );
  m_selectedView->setExpanded( m_selectedModel->indexFromItem( parentItem ), true );
  //m_selectedView->selectionModel()->setCurrentIndex( m_selectedModel->indexFromItem(item), QItemSelectionModel::SelectCurrent );
  m_availableModel->emitLayoutChanged( m_availableView->selectionModel()->currentIndex() );
#endif
}

void
AddressesDialog::addGroupToSelected( const KABC::ContactGroup& group, QStandardItem *parentItem )
{
  Q_ASSERT( parentItem );
  if( m_availableModel->m_groupMap.contains( group.id() ) ) {
    kDebug() << "Group already added, email=" << group.id();
    return; //already got it
  }

  AddresseeViewItem *item = new AddresseeViewItem( group.name(), group );
  item->setIcon( KIcon( QLatin1String( "x-mail-distribution-list" ) ) );
  m_availableModel->m_groupMap[ group.id() ] = item;
  parentItem->appendRow( item );
  m_selectedView->setExpanded( m_selectedModel->indexFromItem( parentItem ), true );
  m_availableModel->emitLayoutChanged( m_availableView->selectionModel()->currentIndex() );
}

#if 0
void
AddressesDialog::addAddresseesToSelected( AddresseeViewItem *parent,
                                          const QList<AddresseeViewItem*> addresses )
{
  Q_ASSERT( parent );

  QList<AddresseeViewItem*>::const_iterator itr = addresses.begin();

  if ( itr != addresses.end() )
  {
    m_ui->mSaveAs->setEnabled(true);
  }

  while ( itr != addresses.end() ) {
    AddresseeViewItem* address = (*itr);
    ++itr;

    if ( selectedToAvailableMapping.find( address ) != selectedToAvailableMapping.end() )
    {
      continue;
    }

    AddresseeViewItem* newItem = 0;
    if (address->category() == AddresseeViewItem::Entry)
    {
      newItem = new AddresseeViewItem( parent, address->addressee() );
    }
    else if (address->category() == AddresseeViewItem::DistList)
    {
      newItem = new AddresseeViewItem( parent, address->name() );
    }
    else
    {
      newItem = new AddresseeViewItem( parent, address->name(), allAddressee( address ) );
    }

    address->setSelected( false );
    address->setHidden( true );
    selectedToAvailableMapping.insert(address, newItem);
    selectedToAvailableMapping.insert(newItem, address);
  }

  m_ui->mSelectedView->expandItem( parent );
}
#endif

QStringList
AddressesDialog::entryToString( const KABC::Addressee::List& l ) const
{
  QStringList entries;
  for( KABC::Addressee::List::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
    entries += (*it).fullEmail();
  }
  return entries;
}

#if 0
QList<AddresseeViewItem*> AddressesDialog::selectedAvailableAddresses() const
{
  QList<AddresseeViewItem*> list;
  foreach( QTreeWidgetItem* item, m_ui->mAvailableView->selectedItems() ) {
    list.append( static_cast<AddresseeViewItem*>( item ) );
  }
  return list;
}

QList<AddresseeViewItem*> AddressesDialog::selectedSelectedAddresses() const
{
  QList<AddresseeViewItem*> list;;
  foreach( QTreeWidgetItem* item, m_ui->mSelectedView->selectedItems() ) {
    list.append( static_cast<AddresseeViewItem*>( item ) );
  }
  return list;
}

void
AddressesDialog::addSelectedTo()
{
  if ( !m_toItem )
  {
    m_toItem = new AddresseeViewItem( m_ui->mSelectedView, i18n("To"), AddresseeViewItem::To );
  }

  addAddresseesToSelected( m_toItem, selectedAvailableAddresses() );

  if ( m_toItem->childCount() > 0 )
    m_toItem->setHidden( false );
  else
  {
    delete m_toItem;
    m_toItem = 0;
  }
  availableSelectionChanged();
}

void
AddressesDialog::addSelectedCC()
{
  if ( !m_ccItem )
  {
    m_ccItem = new AddresseeViewItem( m_ui->mSelectedView, i18n("CC"), AddresseeViewItem::CC );
  }

  addAddresseesToSelected( m_ccItem, selectedAvailableAddresses() );

  if ( m_ccItem->childCount() > 0 )
    m_ccItem->setHidden( false );
  else
  {
    delete m_ccItem;
    m_ccItem = 0;
  }
  availableSelectionChanged();
}

void
AddressesDialog::addSelectedBCC()
{
  if ( !m_bccItem )
  {
    m_bccItem = new AddresseeViewItem( m_ui->mSelectedView, i18n("BCC"), AddresseeViewItem::BCC );
  }

  addAddresseesToSelected( m_bccItem, selectedAvailableAddresses() );

  if ( m_bccItem->childCount() > 0 )
    m_bccItem->setHidden( false );
  else
  {
    delete m_bccItem;
    m_bccItem = 0;
  }
  availableSelectionChanged();
}

void AddressesDialog::unmapSelectedAddress(AddresseeViewItem* item)
{
  AddresseeViewItem* correspondingItem = selectedToAvailableMapping[item];
  if (correspondingItem)
  {
    correspondingItem->setHidden( false );
    selectedToAvailableMapping.remove( item );
    selectedToAvailableMapping.remove( correspondingItem );
  }

  int childIndex = 0;
  AddresseeViewItem* child = static_cast<AddresseeViewItem*>(
      item->QTreeWidgetItem::child( childIndex) );
  while ( child )
  {
    unmapSelectedAddress( child );
    child = static_cast<AddresseeViewItem*>(
        item->QTreeWidgetItem::child( ++childIndex ) );
  }
}

void
AddressesDialog::removeEntry()
{
  QList<AddresseeViewItem*> lst;
  bool resetTo  = false;
  bool resetCC  = false;
  bool resetBCC = false;

  QList<AddresseeViewItem*> selectedItems = selectedSelectedAddresses();
  QList<AddresseeViewItem*>::iterator it = selectedItems.begin();
  while ( it != selectedItems.end() ) {
    AddresseeViewItem* item = (*it);
    ++it;
    if ( m_toItem == item )
      resetTo = true;
    else if ( m_ccItem == item )
      resetCC = true;
    else if( m_bccItem == item )
      resetBCC = true;
    // we may only append parent items
    unmapSelectedAddress(item);
    lst.append( item );
  }
  qDeleteAll( lst );
  lst.clear();
  if ( resetTo )
    m_toItem  = 0;
  else if ( m_toItem && m_toItem->childCount() == 0 )
  {
    delete m_toItem;
    m_toItem = 0;
  }
  if ( resetCC )
    m_ccItem = 0;
  else if ( m_ccItem && m_ccItem->childCount() == 0 )
  {
    delete m_ccItem;
    m_ccItem = 0;
  }
  if ( resetBCC )
    m_bccItem  = 0;
  else if ( m_bccItem && m_bccItem->childCount() == 0 )
  {
    delete m_bccItem;
    m_bccItem = 0;
  }
  m_ui->mSaveAs->setEnabled( m_ui->mSelectedView->topLevelItemCount() > 0 );
  selectedSelectionChanged();
}

// copied from kabcore.cpp :(
// KDE4: should be in libkabc I think
static KABC::Resource *requestResource( KABC::AddressBook* abook, QWidget *parent )
{
  const QList<KABC::Resource*> kabcResources = abook->resources();

  QList<KRES::Resource*> kresResources;
  QList<KABC::Resource*>::const_iterator resIt;
  for ( resIt = kabcResources.begin(); resIt != kabcResources.end(); ++resIt) {
    if ( !(*resIt)->readOnly() ) {
      KRES::Resource *res = static_cast<KRES::Resource*>( *resIt );
      if ( res )
        kresResources.append( res );
    }
  }

  if ( kresResources.size() > 0 ) {
    KRES::Resource *res = KRES::SelectDialog::getResource( kresResources, parent );
    return static_cast<KABC::Resource*>( res );
  }
  else return 0;
}
#endif

void appendToContactGroup( QStandardItem *parentItem, KABC::ContactGroup &group )
{
  if( ! parentItem )
    return;
  for(int i = 0; i < parentItem->rowCount(); ++i) {
    AddresseeViewItem *item = dynamic_cast<AddresseeViewItem*>( parentItem->child( i ) );
    Q_ASSERT( item );
    switch( item->itemType() ) {
      case AddresseeViewItem::Address: {
        KABC::Addressee addr = item->addressee();
        const QString id = addr.uid();
        Q_ASSERT( ! id.isEmpty() );
        group.append( KABC::ContactGroup::ContactReference( id ) );
      } break;
      case AddresseeViewItem::Group: {
        KABC::ContactGroup group = item->group();
        const QString id = group.id();
        Q_ASSERT( ! id.isEmpty() );
        group.append( KABC::ContactGroup::ContactGroupReference( id ) );
      } break;
    }
  }
}

void
AddressesDialog::saveAsDistributionList()
{
#if 0
  if ( m_ui->mSelectedView->topLevelItemCount() == 0 ) {
    KMessageBox::information( 0,
                              i18n("There are no addresses in your list. "
                                   "First add some addresses from your address book, "
                                   "then try again.") );
    return;
  }

  bool ok = false;
  QString name = KInputDialog::getText( i18n("New Distribution List"),
                                        i18n("Please enter name:"),
                                        QString(), &ok,
                                        this );
  if ( !ok || name.isEmpty() )
    return;

  bool alreadyExists = false;
  KABC::AddressBook* abook = KABC::StdAddressBook::self( true );
  KABC::DistributionList *dlist = abook->findDistributionListByName( name );
  alreadyExists = (dlist != 0);

  if ( alreadyExists ) {
    KMessageBox::information( 0,
                              i18n( "<qt>Distribution list with the given name <b>%1</b> "
                                    "already exists. Please select a different name.</qt>" ,
                                name ) );
    return;
  }

  KABC::Resource* resource = requestResource( abook, this );
  if ( !resource )
    return;

  dlist = new KABC::DistributionList( resource, name );
  KABC::Addressee::List addrl = allAddressee( m_ui->mSelectedView, false );
  for ( KABC::Addressee::List::iterator itr = addrl.begin();
        itr != addrl.end(); ++itr ) {
    dlist->insertEntry( *itr );
  }
#else
  Q_ASSERT( m_toItem || m_ccItem || m_bccItem );

  QPointer<CollectionDialog> dlg = new CollectionDialog( this );
  dlg->setMimeTypeFilter( QStringList() << KABC::ContactGroup::mimeType() );
  dlg->setAccessRightsFilter( Collection::CanCreateItem );
  dlg->setCaption( i18n( "Select Address Book" ) );
  dlg->setDescription( i18n( "Select the address book the new contact group shall be saved in:" ) );
  if ( dlg->exec() != KDialog::Accepted )
    return;

  Akonadi::Collection collection = dlg->selectedCollection();
  Q_ASSERT( collection.isValid() );

  bool ok = false;
  QString name = KInputDialog::getText( i18n("New Contact Group"),
                                        i18n("Please enter name of the new contact group:"),
                                        QString(), &ok, this );
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
  connect( job, SIGNAL( result( KJob* ) ), SLOT( storeDone( KJob* ) ) );
  job->start();
#endif
}

void AddressesDialog::saveAsDistributionListDone( KJob *job )
{
  if( ! job->error() ) {
    KMessageBox::error( this, job->errorText() );
    return;
  }
  m_availableModel->emitLayoutChanged();
}

void
AddressesDialog::searchLdap()
{
    if ( !m_ldapSearchDialog ) {
      m_ldapSearchDialog = new LdapSearchDialog( this );
      connect( m_ldapSearchDialog, SIGNAL( addresseesAdded() ),
               SLOT(ldapSearchResult() ) );
    }
    m_ldapSearchDialog->show();
}

void
AddressesDialog::ldapSearchResult()
{
  QStringList emails = KPIMUtils::splitAddressList( m_ldapSearchDialog->selectedEMails() );
  QStringList::iterator it( emails.begin() );
  QStringList::iterator end( emails.end() );
  QString name, email;
  for ( ; it != end; ++it ){
      KPIMUtils::extractEmailAddressAndName( (*it), email, name );
      KABC::Addressee ad;
      ad.setNameFromString( name );
      ad.insertEmail( email );
      addAddresseeToSelected( ad, selectedToItem() );
  }
}

#if 0
void
AddressesDialog::filterChanged( const QString& txt )
{
  QTreeWidgetItemIterator it( m_ui->mAvailableView );
  bool showAll = false;

  if ( txt.isEmpty() )
    showAll = true;

  while ( (*it) ) {
    AddresseeViewItem* item = static_cast<AddresseeViewItem*>( (*it) );
    ++it;
    if ( showAll ) {
      item->setHidden( false );
      if ( item->category() == AddresseeViewItem::Group )
        m_ui->mAvailableView->collapseItem( item );//close to not have too many entries
      continue;
    }
    if ( item->category() == AddresseeViewItem::Entry ) {
      bool matches = item->matches( txt ) ;
      item->setHidden( !matches );
      QTreeWidgetItem *parent = item->QTreeWidgetItem::parent();
      if ( matches && parent ) {
        m_ui->mAvailableView->expandItem( parent );//open the parents with found entries
      }
    }
  }
}

KABC::Addressee::List
AddressesDialog::allAddressee( QTreeWidget* view, bool onlySelected ) const
{
  KABC::Addressee::List lst;
  QTreeWidgetItemIterator it( view );
  while ( *it ) {
    AddresseeViewItem* item = static_cast<AddresseeViewItem*>( *it );
    if ( !onlySelected || item->isSelected() ) {
      if ( item->category() != AddresseeViewItem::Entry  ) {
        int childIndex = 0;
        AddresseeViewItem *myChild = static_cast<AddresseeViewItem*>(
            item->QTreeWidgetItem::child( childIndex ) );
        while( myChild ) {
          lst.append( myChild->addressee() );
          myChild = static_cast<AddresseeViewItem*>(
              item->QTreeWidgetItem::child( ++childIndex ) );
        }
      } else {
        lst.append( item->addressee() );
      }
    }
    ++it;
  }

  return lst;
}
#endif

typedef QPair<QString,QString> ItemPair;
void extractMailsFromGroup( const KABC::ContactGroup &group, QList<ItemPair> &mails, QStringList &groupsDone )
{
  Q_ASSERT( ! group.id().isEmpty() );
  groupsDone.append( group.id() );
  for( uint i = 0; i < group.dataCount(); ++i )
    mails.append( ItemPair( group.data( i ).name(), group.data( i ).email() ) );
  for( uint i = 0; i < group.contactReferenceCount(); ++i )
    mails.append( ItemPair( QString(), group.contactReference( i ).preferredEmail() ) );

  for( uint i = 0; i < group.contactGroupReferenceCount(); ++i ) {
    const Akonadi::Item item( group.contactGroupReference( i ).uid().toLongLong() );
    Akonadi::ItemFetchJob *j = new Akonadi::ItemFetchJob( item );
    j->fetchScope().fetchFullPayload();
    if( j->exec() ) {
      Q_ASSERT( j->items().count() == 1 );
      Q_ASSERT( j->items().first().hasPayload<KABC::ContactGroup>() );
      const KABC::ContactGroup g = j->items().first().payload<KABC::ContactGroup>();
      if( ! groupsDone.contains( g.id() ) )
        extractMailsFromGroup( g, mails, groupsDone );
    }
  }
}

KABC::Addressee::List
AddressesDialog::allAddressee( QStandardItem* parent ) const
{
  KABC::Addressee::List lst;
  if ( !parent ) return lst;
#if 0
  if ( parent->category() == AddresseeViewItem::Entry )
  {
    lst.append( parent->addressee() );
    return lst;
  }
  int childIndex = 0;
  AddresseeViewItem *myChild = static_cast<AddresseeViewItem*>(
      parent->QTreeWidgetItem::child( childIndex ) );
  while( myChild ) {
    if ( myChild->category() == AddresseeViewItem::FilledGroup )
      lst += myChild->addresses();
    else if ( !myChild->addressee().isEmpty() )
      lst.append( myChild->addressee() );
    myChild = static_cast<AddresseeViewItem*>(
        parent->QTreeWidgetItem::child( ++childIndex ) );
  }
#else
  for(int i = parent->rowCount() - 1; i >= 0; --i) {
    AddresseeViewItem *item = dynamic_cast<AddresseeViewItem*>( parent->child( i ) );
    Q_ASSERT( item );
    switch( item->itemType() ) {
      case AddresseeViewItem::Address: {
        lst += item->addressee();
      } break;
      case AddresseeViewItem::Group: {
        QList<ItemPair> mails;
        QStringList groupsDone;
        extractMailsFromGroup( item->group(), mails, groupsDone );
        foreach( ItemPair p, mails ) {
          const QString name = p.first;
          const QString email = p.second;
          if( email.isEmpty() )
            continue;
          KABC::Addressee addr;
          addr.setNameFromString( name );
          addr.insertEmail( email );
          lst.append( addr );
        }
      } break;
    }
  }
#endif
  return lst;
}

QStringList
AddressesDialog::allDistributionLists( QStandardItem* parent ) const
{
  QStringList lists;

  if ( !parent )
    return lists;

  const int count = parent->rowCount();
  for( int i = 0; i < count; ++i) {
    AddresseeViewItem *item = dynamic_cast<AddresseeViewItem*>( parent->child( i ) );
    Q_ASSERT( item );
#if 0
    if ( item->category() == AddresseeViewItem::DistList )
      lists.append( item->name() );
#endif
  }

  return lists;
}

#if 0
void
AddressesDialog::addDistributionLists()
{
  KABC::AddressBook* abook = KABC::StdAddressBook::self( true );

  const QList<KABC::DistributionList*> distLists = abook->allDistributionLists();
  if ( distLists.isEmpty() )
    return;

  AddresseeViewItem *topItem = new AddresseeViewItem( m_ui->mAvailableView,
                                                      i18n( "Distribution Lists" ) );

  QList<KABC::DistributionList*>::ConstIterator listIt;
  for ( listIt = distLists.constBegin(); listIt != distLists.constEnd(); ++listIt ) {
    KABC::DistributionList *dlist = *listIt;
    KABC::DistributionList::Entry::List entries = dlist->entries();

    AddresseeViewItem *item = new AddresseeViewItem( topItem, dlist->name() );

    KABC::DistributionList::Entry::List::ConstIterator itemIt;
    for ( itemIt = entries.constBegin(); itemIt != entries.constEnd(); ++itemIt )
      addAddresseeToAvailable( (*itemIt).addressee(), item, false );
  }
}
#endif

} // namespace

#include "addressesdialog.moc"
