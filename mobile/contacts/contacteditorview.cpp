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

#include <incidenceeditors/incidenceeditor-ng/editoritemmanager.h>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KABC/Addressee>

using namespace Akonadi;

class ContactEditorView::Private : public Akonadi::ItemEditorUi
{
  ContactEditorView *const q;

  public:
    explicit Private( ContactEditorView *parent )
      : q( parent ), mItemManager( new EditorItemManager( this ) ), mEditorGeneral( 0 )
    {
    }

    ~Private()
    {
      delete mItemManager;
    }

    void addDetailEditor( EditorBase *editor );

  public: // slots
    void saveFinished();
    void saveFailed( const QString &errorMessage );
    void collectionChanged( const Akonadi::Collection &collection );

  public: // ItemEditorGeneralUi interface
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
    Collection mCollection;

    EditorItemManager *mItemManager;

    EditorGeneral *mEditorGeneral;

    QList<EditorBase*> mDetailEditors;
};

void ContactEditorView::Private::addDetailEditor( EditorBase *editor )
{
  if ( editor != 0 ) {
    mDetailEditors << editor;

    if ( mItem.hasPayload<KABC::Addressee>() ) {
      editor->loadContact( mItem.payload<KABC::Addressee>() );
    }
  }
}

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
  Q_ASSERT( item.hasPayload<KABC::Addressee>() );
 
  mItem = item;
  mCollection = item.parentCollection();

  const KABC::Addressee contact = mItem.payload<KABC::Addressee>();
  
  if ( mEditorGeneral != 0 ) {
    mEditorGeneral->setDefaultCollection( mCollection );
    mEditorGeneral->loadContact( contact );
  }

  Q_FOREACH( EditorBase *editor, mDetailEditors ) {
    editor->loadContact( contact );
  }
}

Item ContactEditorView::Private::save( const Item &item )
{
  Item result = item;

  result.setMimeType( KABC::Addressee::mimeType() );

  KABC::Addressee contact;
  if ( mEditorGeneral != 0 ) {
    mEditorGeneral->saveContact( contact );
  }
 
  Q_FOREACH( EditorBase *editor, mDetailEditors ) {
    editor->saveContact( contact );
  }

  result.setPayload<KABC::Addressee>( contact );

  return result;
}

void ContactEditorView::Private::collectionChanged( const Akonadi::Collection &collection )
{
  mCollection = collection;
}

Collection ContactEditorView::Private::selectedCollection() const
{
  return mCollection;
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

ContactEditorView::ContactEditorView( QWidget *parent )
  : KDeclarativeFullScreenView( QLatin1String( "contact-editor" ), parent ),
    d( new Private( this ) )
{
  qmlRegisterType<DeclarativeEditorGeneral>( "org.kde.contacteditors", 4, 5, "ContactEditorGeneral" );
  qmlRegisterType<DeclarativeEditorBusiness>( "org.kde.contacteditors", 4, 5, "ContactEditorBusiness" );
  qmlRegisterType<DeclarativeEditorLocation>( "org.kde.contacteditors", 4, 5, "ContactEditorLocation" );
  qmlRegisterType<DeclarativeEditorCrypto>( "org.kde.contacteditors", 4, 5, "ContactEditorCrypto" );
  qmlRegisterType<DeclarativeEditorMore>( "org.kde.contacteditors", 4, 5, "ContactEditorMore" );
  
  connect( d->mItemManager, SIGNAL( itemSaveFinished() ), SLOT( saveFinished() ) );
  connect( d->mItemManager, SIGNAL( itemSaveFailed( QString ) ), SLOT( saveFailed( QString ) ) );
}    

ContactEditorView::~ContactEditorView()
{
  delete d;
}

void ContactEditorView::save()
{
  d->mItemManager->save();
}

void ContactEditorView::cancel()
{
  deleteLater();
}

void ContactEditorView::setEditorGeneral( EditorGeneral *editor )
{
  d->mEditorGeneral = editor;

  if ( d->mEditorGeneral != 0 ) {
    if ( d->mCollection.isValid() ) {
      d->mEditorGeneral->setDefaultCollection( d->mCollection );
    }
    connect( d->mEditorGeneral, SIGNAL( saveClicked() ), SLOT( save() ) );
    connect( d->mEditorGeneral, SIGNAL( collectionChanged( Akonadi::Collection ) ),
             SLOT( collectionChanged( Akonadi::Collection ) ) );
  }
}

void ContactEditorView::setEditorBusiness( EditorBusiness *editor )
{
  d->addDetailEditor( editor );
}

void ContactEditorView::setEditorLocation( EditorLocation *editor )
{
  d->addDetailEditor( editor );
}

void ContactEditorView::setEditorCrypto( EditorCrypto *editor )
{
  d->addDetailEditor( editor );
}

void ContactEditorView::setEditorMore( EditorMore *editor )
{
  d->addDetailEditor( editor );
}

void ContactEditorView::loadContact( const Item &item )
{
  d->mItemManager->load( item );
}

#include "contacteditorview.moc"
