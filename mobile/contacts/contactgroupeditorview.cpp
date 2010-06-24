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

#include "contactgroupeditorview.h"

#include "declarativeeditors.h"

#include <incidenceeditors/incidenceeditor-ng/editoritemmanager.h>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KABC/ContactGroup>

using namespace Akonadi;

class ContactGroupEditorView::Private : public Akonadi::ItemEditorUi
{
  ContactGroupEditorView *const q;

  public:
    explicit Private( ContactGroupEditorView *parent )
      : q( parent ), mItemManager( new EditorItemManager( this ) ), mEditor( 0 )
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

  public: // ItemEditorUi interface
    bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const
    {
      return partIdentifiers.contains( Item::FullPayload );
    }
    
    bool hasSupportedPayload( const Item &item ) const
    {
      return item.hasPayload<KABC::ContactGroup>();
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

    EditorContactGroup *mEditor;
};

void ContactGroupEditorView::Private::saveFinished()
{
  kDebug();
  q->deleteLater();
}

void ContactGroupEditorView::Private::saveFailed( const QString &errorMessage )
{
  kError() << errorMessage;
}

void ContactGroupEditorView::Private::load( const Item &item )
{
  Q_ASSERT( item.hasPayload<KABC::ContactGroup>() );
 
  mItem = item;
  mCollection = item.parentCollection();

  const KABC::ContactGroup contactGroup = mItem.payload<KABC::ContactGroup>();
  
  if ( mEditor != 0 ) {
    mEditor->setDefaultCollection( mCollection );
    mEditor->loadContactGroup( contactGroup );
  }
}

Item ContactGroupEditorView::Private::save( const Item &item )
{
  Item result = item;

  result.setMimeType( KABC::ContactGroup::mimeType() );

  KABC::ContactGroup contactGroup;
  if ( mEditor != 0 ) {
    mEditor->saveContactGroup( contactGroup );
  }
 
  result.setPayload<KABC::ContactGroup>( contactGroup );

  return result;
}

void ContactGroupEditorView::Private::collectionChanged( const Akonadi::Collection &collection )
{
  mCollection = collection;
}

Collection ContactGroupEditorView::Private::selectedCollection() const
{
  return ( !mCollection.isValid() && mEditor != 0 ? mEditor->selectedCollection() : mCollection );
}

void ContactGroupEditorView::Private::reject( RejectReason reason, const QString &errorMessage )
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

ContactGroupEditorView::ContactGroupEditorView( QWidget *parent )
  : KDeclarativeFullScreenView( QLatin1String( "contactgroup-editor" ), parent ),
    d( new Private( this ) )
{
  qmlRegisterType<DeclarativeEditorContactGroup>( "org.kde.contacteditors", 4, 5, "ContactGroupEditor" );
  
  connect( d->mItemManager, SIGNAL( itemSaveFinished() ), SLOT( saveFinished() ) );
  connect( d->mItemManager, SIGNAL( itemSaveFailed( QString ) ), SLOT( saveFailed( QString ) ) );
}    

ContactGroupEditorView::~ContactGroupEditorView()
{
  delete d;
}

void ContactGroupEditorView::save()
{
  d->mItemManager->save();
}

void ContactGroupEditorView::cancel()
{
  deleteLater();
}

void ContactGroupEditorView::setEditor( EditorContactGroup *editor )
{
  d->mEditor = editor;

  if ( d->mEditor != 0 ) {
    if ( d->mCollection.isValid() ) {
      d->mEditor->setDefaultCollection( d->mCollection );
    }
    connect( d->mEditor, SIGNAL( saveClicked() ), SLOT( save() ) );
    connect( d->mEditor, SIGNAL( collectionChanged( Akonadi::Collection ) ),
             SLOT( collectionChanged( Akonadi::Collection ) ) );
    connect( d->mEditor, SIGNAL( requestLaunchAccountWizard() ),
             this, SIGNAL( requestLaunchAccountWizard() ) );
  }
}

void ContactGroupEditorView::loadContactGroup( const Item &item )
{
  d->mItemManager->load( item );
}

#include "contactgroupeditorview.moc"
