/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "contacteditorview.h"

#include "declarativeeditors.h"

#include <incidenceeditors/incidenceeditor-ng/itemeditor.h>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KABC/Addressee>

using namespace Akonadi;

class ContactEditorView::Private : public Akonadi::ItemEditorUi
{
  ContactEditorView *const q;

  public:
    explicit Private( ContactEditorView *parent )
      : q( parent ), mItemManager( new EditorItemManager( this ) ), mCollectionSelector( 0 ), mEditor( 0 )
    {
    }

    ~Private()
    {
      delete mItemManager;
    }

  public: // slots
    void saveFinished();
    void saveFailed( const QString &errorMessage );

  public: // ItemEditorUi interface
    bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const
    {
      return partIdentifiers.contains( Item::FullPayload );
    }
    
    bool hasSupportedPayload( const Item &item ) const
    {
      return item.hasPayload<KABC::Addressee>();
    }
    
    bool isDirty() const
    {
      return true;
    }
  
    bool isValid()
    {
      return true;
    }    

    void load( const Item &item );
    Item save( const Item &item );
    Collection selectedCollection() const;
    void reject( RejectReason reason, const QString &errorMessage = QString() );

  public:
    Item mItem;

    EditorItemManager *mItemManager;

    CollectionComboBox *mCollectionSelector;
    ContactEditor *mEditor;
};

void ContactEditorView::Private::saveFinished()
{
  kDebug();
}

void ContactEditorView::Private::saveFailed( const QString &errorMessage )
{
  kError() << errorMessage;
}

void ContactEditorView::Private::load( const Item &item )
{
  mItem = item;

  if ( mCollectionSelector != 0 ) {
    mCollectionSelector->setDefaultCollection( mItem.parentCollection() );
  }
}

Item ContactEditorView::Private::save( const Item &item )
{
  Item result = item;

  result.setMimeType( KABC::Addressee::mimeType() );

  return result;
}

Collection ContactEditorView::Private::selectedCollection() const
{
  return ( mCollectionSelector != 0 ? mCollectionSelector->currentCollection() : Collection() );
}

void ContactEditorView::Private::reject( RejectReason reason, const QString &errorMessage )
{
  switch ( reason ) {
    case ItemFetchFailed:
      kWarning() << "Item Fetch Failed:" << errorMessage;
      break;

    case ItemHasInvalidPayload:
      kWarning() << "Item has Invalid Payload:" << errorMessage;
      break;
  }
  
  q->deleteLater();
}

ContactEditorView::ContactEditorView( Mode mode, QWidget *parent )
  : KDeclarativeFullScreenView( QLatin1String( "contact-editor" ), parent ),
    d( new Private( this ) )
{
  qmlRegisterType<DeclarativeCollectionSelector>( "org.kde.contacteditors", 4, 5, "CollectionSelector" );
  if ( mode == Create ) {
    qmlRegisterType<DeclarativeContactCreator>( "org.kde.contacteditors", 4, 5, "ContactEditor" );
  } else {
    qmlRegisterType<DeclarativeContactEditor>( "org.kde.contacteditors", 4, 5, "ContactEditor" );
  }
  
  connect( d->mItemManager, SIGNAL( itemSaveFinished() ), SLOT( saveFinished() ) );
  connect( d->mItemManager, SIGNAL( itemSaveFailed( QString ) ), SLOT( saveFailed( QString ) ) );
}    

ContactEditorView::~ContactEditorView()
{
  delete d;
}

void ContactEditorView::save()
{
  if ( d->mEditor != 0 ) {
    d->mEditor->saveContact();
  } else {
    d->mItemManager->save();
  }
}

void ContactEditorView::cancel()
{
  deleteLater();
}

void ContactEditorView::setCollectionSelector( CollectionComboBox *comboBox )
{
  d->mCollectionSelector = comboBox;

  if ( d->mCollectionSelector != 0 ) {
    d->mCollectionSelector->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    d->mCollectionSelector->setDefaultCollection( d->mItem.parentCollection() );
  }
}

void ContactEditorView::setCreatorWidget( ContactCreateWidget *editor )
{
  d->mEditor = editor;
}

void ContactEditorView::setEditorWidget( ContactEditWidget *editor )
{
  d->mEditor = editor;

  if ( d->mEditor != 0 ) {
    d->mEditor->loadContact( d->mItem );
  }
}

void ContactEditorView::loadContact( const Item &item )
{
  if ( d->mEditor != 0 ) {
    d->mItem = item;
    d->mEditor->loadContact( d->mItem );
  } else {
    d->mItemManager->load( item );
  }
}

#include "contacteditorview.moc"
