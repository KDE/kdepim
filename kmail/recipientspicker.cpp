/*
    This file is part of KMail.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "recipientspicker.h"

#include "globalsettings.h"

#include <libkdepim/recentaddresses.h>
#include <libkdepim/ldapsearchdialog.h>

#include <libemailfunctions/email.h>

#ifndef KDEPIM_NEW_DISTRLISTS
#include <kabc/distributionlist.h>
#endif

#include <klistview.h>
#include <klocale.h>
#include <kabc/resource.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kwin.h>
#include <kmessagebox.h>

#include <tqlayout.h>
#include <tqcombobox.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqlabel.h>

#ifdef KDEPIM_NEW_DISTRLISTS
RecipientItem::RecipientItem( KABC::AddressBook *ab )
  : mAddressBook( ab )
{
}
#else
RecipientItem::RecipientItem()
  : mDistributionList( 0 )
{
}
#endif

#ifdef KDEPIM_NEW_DISTRLISTS
void RecipientItem::setDistributionList( KPIM::DistributionList &list )
{
  mDistributionList = list;

  mIcon = KGlobal::iconLoader()->loadIcon( "kdmconfig", KIcon::Small );

  mName = list.name();
  mKey = list.name();

  int count = list.entries( mAddressBook ).count();
  mEmail = i18n( "1 email address", "%n email addresses", count );

  mRecipient = mName;

  mTooltip = createTooltip( list );
}
#else
void RecipientItem::setDistributionList( KABC::DistributionList *list )
{
  mDistributionList = list;

  mIcon = KGlobal::iconLoader()->loadIcon( "kdmconfig", KIcon::Small );

  mName = list->name();
  mKey = list->name();

  int count = list->entries().count();
  mEmail = i18n( "1 email address", "%n email addresses", count );

  mRecipient = mName;

  mTooltip = createTooltip( list );
}
#endif

void RecipientItem::setAddressee( const KABC::Addressee &a,
  const TQString &email )
{
  mAddressee = a;
  mEmail = email;
  mRecipient = mAddressee.fullEmail( mEmail );

  TQImage img = a.photo().data();
  if ( !img.isNull() )
    mIcon = img.smoothScale( 20, 20, TQImage::ScaleMin );
  else
    mIcon = KGlobal::iconLoader()->loadIcon( "personal", KIcon::Small );

  mName = mAddressee.realName();
  mKey = mAddressee.realName() + '|' + mEmail;

  mTooltip = "<qt>";
  if ( !mAddressee.realName().isEmpty() ) {
    mTooltip += mAddressee.realName() + "<br/>";
  }
  mTooltip += "<b>" + mEmail + "</b>";
}

TQPixmap RecipientItem::icon() const
{
  return mIcon;
}

TQString RecipientItem::name() const
{
  return mName;
}

TQString RecipientItem::email() const
{
  return mEmail;
}

TQString RecipientItem::recipient() const
{
  return mRecipient;
}

TQString RecipientItem::tooltip() const
{
  return mTooltip;
}

#ifdef KDEPIM_NEW_DISTRLISTS
KPIM::DistributionList& RecipientItem::distributionList() {
  return mDistributionList;
}
#else
KABC::DistributionList * RecipientItem::distributionList() {
  return mDistributionList;
}
#endif

#ifdef KDEPIM_NEW_DISTRLISTS
TQString RecipientItem::createTooltip( KPIM::DistributionList &distributionList ) const
{
  TQString txt = "<qt>";

  txt += "<b>" + i18n( "Distribution List %1" ).arg ( distributionList.name() ) + "</b>";
  txt += "<ul>";
  KPIM::DistributionList::Entry::List entries = distributionList.entries( mAddressBook );
  KPIM::DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    txt += "<li>";
    txt += (*it).addressee.realName() + ' ';
    txt += "<em>";
    if ( (*it).email.isEmpty() ) txt += (*it).addressee.preferredEmail();
    else txt += (*it).email;
    txt += "</em>";
    txt += "<li/>";
  }
  txt += "</ul>";
  txt += "</qt>";

  return txt;
}
#else
TQString RecipientItem::createTooltip( KABC::DistributionList *distributionList ) const
{
  TQString txt = "<qt>";

  txt += "<b>" + i18n("Distribution List %1" ).arg ( distributionList->name() ) + "</b>";
  txt += "<ul>";
  KABC::DistributionList::Entry::List entries = distributionList->entries();
  KABC::DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    txt += "<li>";
    txt += (*it).addressee.realName() + ' ';
    txt += "<em>";
    if ( (*it).email.isEmpty() ) txt += (*it).addressee.preferredEmail();
    else txt += (*it).email;
    txt += "</em>";
    txt += "</li>";
  }
  txt += "</ul>";
  txt += "</qt>";

  return txt;
}
#endif

void RecipientItem::setRecipientType( const TQString &type )
{
  mType = type;
}

TQString RecipientItem::recipientType() const
{
  return mType;
}


RecipientViewItem::RecipientViewItem( RecipientItem *item, KListView *listView )
  : KListViewItem( listView ), mRecipientItem( item )
{
  setText( 0, item->recipientType() );
  setText( 1, item->name() );
  setText( 2, item->email() );

  setPixmap( 1, item->icon() );
}

RecipientItem *RecipientViewItem::recipientItem() const
{
  return mRecipientItem;
}


RecipientsListToolTip::RecipientsListToolTip( TQWidget *parent,
  KListView *listView )
  : TQToolTip( parent )
{
  mListView = listView;
}

void RecipientsListToolTip::maybeTip( const TQPoint & pos )
{
  TQRect r;
  TQListViewItem *item = mListView->itemAt( pos );
  RecipientViewItem *i = static_cast<RecipientViewItem *>( item );

  if( item ) {
    r = mListView->itemRect( item );
    TQString tipText( i->recipientItem()->tooltip() );
    if ( !tipText.isEmpty() ) {
      tip( r, tipText );
    }
  }
}


RecipientsCollection::RecipientsCollection( const TQString &id )
{
  mId = id;
  mTitle = id;
  mIsReferenceContainer = false;
}

RecipientsCollection::~RecipientsCollection()
{
  deleteAll();
}

void RecipientsCollection::setReferenceContainer( bool isReferenceContainer )
{
  mIsReferenceContainer = isReferenceContainer;
}

bool RecipientsCollection::isReferenceContainer() const
{
  return mIsReferenceContainer;
}

void RecipientsCollection::setTitle( const TQString &title )
{
  mTitle = title;
}

TQString RecipientsCollection::title() const
{
  return mTitle;
}

void RecipientsCollection::addItem( RecipientItem *item )
{
  mKeyMap.insert( item->key(), item );
}

RecipientItem::List RecipientsCollection::items() const
{
  return mKeyMap.values();
}

bool RecipientsCollection::hasEquivalentItem( RecipientItem *item ) const
{
  return mKeyMap.find( item->key() ) != mKeyMap.end();
}

RecipientItem * RecipientsCollection::getEquivalentItem( RecipientItem *item) const
{
  TQMap<TQString, RecipientItem *>::ConstIterator it;
  it = mKeyMap.find( item->key() );
  if ( it == mKeyMap.end() )
    return 0;
  return (*it);
}

void RecipientsCollection::clear()
{
  mKeyMap.clear();
}

void RecipientsCollection::deleteAll()
{
  if ( !isReferenceContainer() ) {
    TQMap<TQString, RecipientItem *>::ConstIterator it;
    for( it = mKeyMap.begin(); it != mKeyMap.end(); ++it ) {
      delete *it;
    }
  }
  clear();
}

TQString RecipientsCollection::id() const
{
  return mId;
}

SearchLine::SearchLine( TQWidget *parent, KListView *listView )
  : KListViewSearchLine( parent, listView )
{
}

void SearchLine::keyPressEvent( TQKeyEvent *ev )
{
  if ( ev->key() == Key_Down ) emit downPressed();

  KListViewSearchLine::keyPressEvent( ev );
}


RecipientsPicker::RecipientsPicker( TQWidget *parent )
  : TQDialog( parent, "RecipientsPicker" )
#ifndef KDEPIM_NEW_DISTRLISTS
    , mDistributionListManager( 0 )
#endif
    ,mLdapSearchDialog( 0 )
{
//  KWin::setType( winId(), NET::Dock );

  setCaption( i18n("Select Recipient") );

  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  TQBoxLayout *resLayout = new TQHBoxLayout( topLayout );

  TQLabel *label = new TQLabel( i18n("Address book:"), this );
  resLayout->addWidget( label );

  mCollectionCombo = new TQComboBox( this );
  resLayout->addWidget( mCollectionCombo );
  resLayout->addItem(new TQSpacerItem(1, 1, TQSizePolicy::Expanding));

//  connect( mCollectionCombo, TQT_SIGNAL( highlighted( int ) ),
//    TQT_SLOT( updateList() ) );
  connect( mCollectionCombo, TQT_SIGNAL( activated( int ) ),
    TQT_SLOT( updateList() ) );

  TQBoxLayout *searchLayout = new TQHBoxLayout( topLayout );

  TQToolButton *button = new TQToolButton( this );
  button->setIconSet( KGlobal::iconLoader()->loadIconSet(
              KApplication::reverseLayout() ? "clear_left":"locationbar_erase", KIcon::Small, 0 ) );
  searchLayout->addWidget( button );
  connect( button, TQT_SIGNAL( clicked() ), TQT_SLOT( resetSearch() ) );

  label = new TQLabel( i18n("&Search:"), this );
  searchLayout->addWidget( label );

  mRecipientList = new KListView( this );
  mRecipientList->setSelectionMode( TQListView::Extended );
  mRecipientList->setAllColumnsShowFocus( true );
  mRecipientList->setFullWidth( true );
  topLayout->addWidget( mRecipientList );
  mRecipientList->addColumn( i18n("->") );
  mRecipientList->addColumn( i18n("Name") );
  mRecipientList->addColumn( i18n("Email") );
  connect( mRecipientList, TQT_SIGNAL( doubleClicked( TQListViewItem *,
    const TQPoint &, int ) ), TQT_SLOT( slotPicked() ) );
  connect( mRecipientList, TQT_SIGNAL( returnPressed( TQListViewItem * ) ),
    TQT_SLOT( slotPicked() ) );

  new RecipientsListToolTip( mRecipientList->viewport(), mRecipientList );

  mSearchLine = new SearchLine( this, mRecipientList );
  searchLayout->addWidget( mSearchLine );
  label->setBuddy( label );
  connect( mSearchLine, TQT_SIGNAL( downPressed() ), TQT_SLOT( setFocusList() ) );

  mSearchLDAPButton = new TQPushButton( i18n("Search &Directory Service"), this );
  searchLayout->addWidget( mSearchLDAPButton );
  connect( mSearchLDAPButton, TQT_SIGNAL( clicked() ), TQT_SLOT( slotSearchLDAP() ) );

  TQBoxLayout *buttonLayout = new TQHBoxLayout( topLayout );

  buttonLayout->addStretch( 1 );

  mToButton = new TQPushButton( i18n("Add as To"), this );
  buttonLayout->addWidget( mToButton );
  connect( mToButton, TQT_SIGNAL( clicked() ), TQT_SLOT( slotToClicked() ) );

  mCcButton = new TQPushButton( i18n("Add as CC"), this );
  buttonLayout->addWidget( mCcButton );
  connect( mCcButton, TQT_SIGNAL( clicked() ), TQT_SLOT( slotCcClicked() ) );

  mBccButton = new TQPushButton( i18n("Add as BCC"), this );
  buttonLayout->addWidget( mBccButton );
  connect( mBccButton, TQT_SIGNAL( clicked() ), TQT_SLOT( slotBccClicked() ) );
  // BCC isn't commonly used, so hide it for now
  //mBccButton->hide();

  TQPushButton *closeButton = new TQPushButton( i18n("&Cancel"), this );
  buttonLayout->addWidget( closeButton );
  connect( closeButton, TQT_SIGNAL( clicked() ), TQT_SLOT( close() ) );

  {
    using namespace KABC;
    mAddressBook = KABC::StdAddressBook::self( true );
    connect( mAddressBook, TQT_SIGNAL( addressBookChanged( AddressBook * ) ),
             this, TQT_SLOT( insertAddressBook( AddressBook * ) ) );
  }

  initCollections();

  mCollectionCombo->setCurrentItem( 0 );

  updateList();

  mSearchLine->setFocus();

  readConfig();

  setTabOrder( mCollectionCombo, mSearchLine );
  setTabOrder( mSearchLine, mRecipientList );
  setTabOrder( closeButton, mCollectionCombo );
}

RecipientsPicker::~RecipientsPicker()
{
  writeConfig();

#ifndef KDEPIM_NEW_DISTRLISTS
  delete mDistributionListManager;
#endif

  TQMap<int,RecipientsCollection *>::ConstIterator it;
  for( it = mCollectionMap.begin(); it != mCollectionMap.end(); ++it ) {
    delete *it;
  }
}

void RecipientsPicker::initCollections()
{
  mAllRecipients = new RecipientsCollection( i18n("All") );
  mAllRecipients->setReferenceContainer( true );
  mDistributionLists = new RecipientsCollection( i18n("Distribution Lists") );
  mSelectedRecipients = new RecipientsCollection( i18n("Selected Recipients") );

  insertCollection( mAllRecipients );
  insertAddressBook( mAddressBook );
  insertCollection( mDistributionLists );
  insertRecentAddresses();
  insertCollection( mSelectedRecipients );

  rebuildAllRecipientsList();
}

void RecipientsPicker::insertAddressBook( KABC::AddressBook *addressbook )
{
  TQMap<KABC::Resource *,RecipientsCollection *> collectionMap;

  TQPtrList<KABC::Resource> resources = addressbook->resources();
  KABC::Resource *res;
  for( res = resources.first(); res; res = resources.next() ) {
    RecipientsCollection *collection = new RecipientsCollection( res->identifier() );
    collectionMap.insert( res, collection );
    collection->setTitle( res->resourceName() );
  }

  TQMap<TQString,RecipientsCollection *> categoryMap;

  KABC::AddressBook::Iterator it;
  for( it = addressbook->begin(); it != addressbook->end(); ++it ) {
    TQStringList emails = (*it).emails();
    TQStringList::ConstIterator it3;
    for( it3 = emails.begin(); it3 != emails.end(); ++it3 ) {
#ifdef KDEPIM_NEW_DISTRLISTS
      RecipientItem *item = new RecipientItem( mAddressBook );
#else
      RecipientItem *item = new RecipientItem;
#endif
      item->setAddressee( *it, *it3 );

      TQMap<KABC::Resource *,RecipientsCollection *>::ConstIterator collIt;
      collIt = collectionMap.find( it->resource() );
      if ( collIt != collectionMap.end() ) {
        (*collIt)->addItem( item );
      }

      TQStringList categories = (*it).categories();
      TQStringList::ConstIterator catIt;
      for( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
        TQMap<TQString, RecipientsCollection *>::ConstIterator catMapIt;
        catMapIt = categoryMap.find( *catIt );
        RecipientsCollection *collection;
        if ( catMapIt == categoryMap.end() ) {
          collection = new RecipientsCollection( *catIt );
          collection->setReferenceContainer( true );
          categoryMap.insert( *catIt, collection );
        } else {
          collection = *catMapIt;
        }
        collection->addItem( item );
      }
    }
  }

  TQMap<KABC::Resource *,RecipientsCollection *>::ConstIterator it2;
  for( it2 = collectionMap.begin(); it2 != collectionMap.end(); ++it2 ) {
    insertCollection( *it2 );
  }

  TQMap<TQString, RecipientsCollection *>::ConstIterator it3;
  for( it3 = categoryMap.begin(); it3 != categoryMap.end(); ++it3 ) {
    insertCollection( *it3 );
  }

  insertDistributionLists();
  rebuildAllRecipientsList();
  updateList();
}

void RecipientsPicker::insertDistributionLists()
{
  mDistributionLists->deleteAll();

#ifdef KDEPIM_NEW_DISTRLISTS
  TQValueList<KPIM::DistributionList> lists = KPIM::DistributionList::allDistributionLists( mAddressBook );
  for ( uint i = 0; i < lists.count(); ++i ) {
    RecipientItem *item = new RecipientItem( mAddressBook );
    item->setDistributionList( lists[ i ] );
    mDistributionLists->addItem( item );
  }
#else
  delete mDistributionListManager;
  mDistributionListManager =
    new KABC::DistributionListManager( KABC::StdAddressBook::self( true ) );

  mDistributionListManager->load();

  TQStringList lists = mDistributionListManager->listNames();

  TQStringList::Iterator listIt;
  for ( listIt = lists.begin(); listIt != lists.end(); ++listIt ) {
    KABC::DistributionList *list = mDistributionListManager->list( *listIt );
    RecipientItem *item = new RecipientItem;
    item->setDistributionList( list );
    mDistributionLists->addItem( item );
  }
#endif
}

void RecipientsPicker::insertRecentAddresses()
{
  RecipientsCollection *collection = new RecipientsCollection( i18n("Recent Addresses") );

  KConfig config( "kmailrc" );
  KABC::Addressee::List recents =
    KRecentAddress::RecentAddresses::self( &config )->kabcAddresses();

  KABC::Addressee::List::ConstIterator it;
  for( it = recents.begin(); it != recents.end(); ++it ) {
#ifdef KDEPIM_NEW_DISTRLISTS
    RecipientItem *item = new RecipientItem( mAddressBook );
#else
    RecipientItem *item = new RecipientItem;
#endif
    item->setAddressee( *it, (*it).preferredEmail() );
    collection->addItem( item );
  }

  insertCollection( collection );
}

void RecipientsPicker::insertCollection( RecipientsCollection *coll )
{
  int index = 0;
  TQMap<int,RecipientsCollection *>::ConstIterator it;
  for ( it = mCollectionMap.begin(); it != mCollectionMap.end(); ++it ) {
    if ( (*it)->id() == coll->id() ) {
      delete *it;
      mCollectionMap.remove( index );
      mCollectionMap.insert( index, coll );
      return;
    }
    index++;
  }

  mCollectionCombo->insertItem( coll->title(), index );
  mCollectionMap.insert( index, coll );
}

void RecipientsPicker::updateRecipient( const Recipient &recipient )
{
  RecipientItem::List allRecipients = mAllRecipients->items();
  RecipientItem::List::ConstIterator itAll;
  for( itAll = allRecipients.begin(); itAll != allRecipients.end(); ++itAll ) {
    if ( (*itAll)->recipient() == recipient.email() ) {
      (*itAll)->setRecipientType( recipient.typeLabel() );
    }
  }
  updateList();
}

void RecipientsPicker::setRecipients( const Recipient::List &recipients )
{
  mSelectedRecipients->deleteAll();

  Recipient::List::ConstIterator it;
  for( it = recipients.begin(); it != recipients.end(); ++it ) {
    RecipientItem *item = 0;

    // if recipient is a distribution list, create
    // a detached copy.
    RecipientItem::List items = mDistributionLists->items();
    RecipientItem::List::ConstIterator distIt;
#ifdef KDEPIM_NEW_DISTRLISTS
    for ( distIt = items.begin(); distIt != items.end(); ++distIt ) {
      if ( (*it).email() == (*distIt)->name() ) {
        item = new RecipientItem( mAddressBook );
        item->setDistributionList( (*distIt)->distributionList() );
      }
    }
#else
    for ( distIt = items.begin(); distIt != items.end(); ++distIt ) {
      if ( (*it).email() == (*distIt)->name() ) {
        item = new RecipientItem();
        item->setDistributionList( (*distIt)->distributionList() );
      }
    }
#endif

    if ( !item ) {
      KABC::Addressee a;
      TQString name;
      TQString email;
      KABC::Addressee::parseEmailAddress( (*it).email(), name, email );
      a.setNameFromString( name );
      a.insertEmail( email );

#ifdef KDEPIM_NEW_DISTRLISTS
      item = new RecipientItem( mAddressBook );
#else
      item = new RecipientItem;
#endif
      item->setAddressee( a, a.preferredEmail() );
    }

    item->setRecipientType( (*it).typeLabel() );
    mSelectedRecipients->addItem( item );
  }

  updateList();
}

void RecipientsPicker::setDefaultButton( TQPushButton *button )
{
//  button->setText( "<qt><b>" + button->text() + "</b></qt>" );
  button->setDefault( true );
}

void RecipientsPicker::setDefaultType( Recipient::Type type )
{
  mDefaultType = type;

  if ( type == Recipient::To ) {
    setDefaultButton( mToButton );
  } else if ( type == Recipient::Cc ) {
    setDefaultButton( mCcButton );
  } else if ( type == Recipient::Bcc ) {
    setDefaultButton( mBccButton );
  }
}

void RecipientsPicker::rebuildAllRecipientsList()
{
  mAllRecipients->clear();

  TQMap<int,RecipientsCollection *>::ConstIterator it;
  for( it = mCollectionMap.begin(); it != mCollectionMap.end(); ++it ) {
    // skip self
    if ( (*it) == mAllRecipients )
      continue;

    RecipientItem::List coll = (*it)->items();

    RecipientItem::List::ConstIterator rcptIt;
    for ( rcptIt = coll.begin(); rcptIt != coll.end(); ++rcptIt ) {
      mAllRecipients->addItem( *rcptIt );
    }
  }
}

void RecipientsPicker::updateList()
{
  mRecipientList->clear();

  RecipientsCollection *coll = mCollectionMap[ mCollectionCombo->currentItem() ];

  RecipientItem::List items = coll->items();
  RecipientItem::List::ConstIterator it;
  for( it = items.begin(); it != items.end(); ++it ) {
    if ( coll != mSelectedRecipients ) {
      RecipientItem *selItem = mSelectedRecipients->getEquivalentItem( *it );
      if ( selItem ) {
        (*it)->setRecipientType( selItem->recipientType() );
      } else {
        (*it)->setRecipientType( TQString() );
      }
    }
    new RecipientViewItem( *it, mRecipientList );
  }

  mSearchLine->updateSearch();
}

void RecipientsPicker::slotToClicked()
{
  pick( Recipient::To );
}

void RecipientsPicker::slotCcClicked()
{
  pick( Recipient::Cc );
}

void RecipientsPicker::slotBccClicked()
{
  pick( Recipient::Bcc );
}

void RecipientsPicker::slotPicked( TQListViewItem *viewItem )
{
  RecipientViewItem *item = static_cast<RecipientViewItem *>( viewItem );
  if ( item ) {
    RecipientItem *i = item->recipientItem();
    emit pickedRecipient( Recipient( i->recipient(), Recipient::Undefined ) );
  }
  close();
}

void RecipientsPicker::slotPicked()
{
  pick( mDefaultType );
}

void RecipientsPicker::pick( Recipient::Type type )
{
  kdDebug() << "RecipientsPicker::pick " << int( type ) << endl;

  int count = 0;
  TQListViewItemIterator it( mRecipientList ,
            TQListViewItemIterator::Visible | TQListViewItemIterator::Selected );
  for ( ; it.current(); ++it )
      ++count;

  if ( count > GlobalSettings::self()->maximumRecipients() ) {
    KMessageBox::sorry( this,
        i18n("You selected 1 recipient. The maximum supported number of "
             "recipients is %1. Please adapt the selection.",
             "You selected %n recipients. The maximum supported number of "
             "recipients is %1. Please adapt the selection.", count)
      .arg( GlobalSettings::self()->maximumRecipients() ) );
    return;
  }

  it = TQListViewItemIterator( mRecipientList ,
            TQListViewItemIterator::Visible | TQListViewItemIterator::Selected );
  for ( ; it.current(); ++it ) {
    RecipientViewItem *item = static_cast<RecipientViewItem *>( it.current() );
    if ( item ) {
      RecipientItem *i = item->recipientItem();
      Recipient r = i->recipient();
      r.setType( type );
      emit pickedRecipient( r );
    }
  }
  close();
}

void RecipientsPicker::keyPressEvent( TQKeyEvent *ev )
{
  if ( ev->key() == Key_Escape ) close();

  TQWidget::keyPressEvent( ev );
}

void RecipientsPicker::readConfig()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "RecipientsPicker" );
  TQSize size = cfg->readSizeEntry( "Size" );
  if ( !size.isEmpty() ) {
    resize( size );
  }
  int currentCollection = cfg->readNumEntry( "CurrentCollection", -1 );
  if ( currentCollection >= 0 &&
       currentCollection < mCollectionCombo->count() ) {
    mCollectionCombo->setCurrentItem( currentCollection );
  }
}

void RecipientsPicker::writeConfig()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "RecipientsPicker" );
  cfg->writeEntry( "Size", size() );
  cfg->writeEntry( "CurrentCollection", mCollectionCombo->currentItem() );
}

void RecipientsPicker::setFocusList()
{
  mRecipientList->setFocus();
}

void RecipientsPicker::resetSearch()
{
  mSearchLine->setText( TQString::null );
}

void RecipientsPicker::slotSearchLDAP()
{
    if ( !mLdapSearchDialog ) {
        mLdapSearchDialog = new KPIM::LDAPSearchDialog( this );
        connect( mLdapSearchDialog, TQT_SIGNAL( addresseesAdded() ),
                 TQT_SLOT(ldapSearchResult() ) );
    }
    mLdapSearchDialog->setSearchText( mSearchLine->text() );
    mLdapSearchDialog->show();

}

void RecipientsPicker::ldapSearchResult()
{
    TQStringList emails = KPIM::splitEmailAddrList( mLdapSearchDialog->selectedEMails() );
    TQStringList::iterator it( emails.begin() );
    TQStringList::iterator end( emails.end() );
    for ( ; it != end; ++it ){
        TQString name;
        TQString email;
        KPIM::getNameAndMail( (*it), name, email );
        KABC::Addressee ad;
        ad.setNameFromString( name );
        ad.insertEmail( email );
#ifdef KDEPIM_NEW_DISTRLISTS
        RecipientItem *item = new RecipientItem( mAddressBook );
#else
        RecipientItem *item = new RecipientItem;
#endif
        item->setAddressee( ad, ad.preferredEmail() );
        emit pickedRecipient( Recipient( item->recipient(), Recipient::Undefined ) );
    }
}

#include "recipientspicker.moc"
