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

#include "editorcontactgroup.h"
#include "declarativewidgetbase.h"

#include <incidenceeditor-ng/editoritemmanager.h>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>

#include <KContacts/ContactGroup>
#include <KLocalizedString>
#include <QDebug>

#include <QDeclarativeItem>

using namespace Akonadi;

typedef DeclarativeWidgetBase<EditorContactGroup, ContactGroupEditorView, &ContactGroupEditorView::setEditor> DeclarativeEditorContactGroup;

class ContactGroupEditorView::Private : public IncidenceEditorNG::ItemEditorUi
{
  ContactGroupEditorView *const q;

  public:
    explicit Private( ContactGroupEditorView *parent )
      : q( parent ), mItemManager( new IncidenceEditorNG::EditorItemManager( this ) ), mEditor( 0 )
    {
    }

    ~Private()
    {
      delete mItemManager;
    }

  public: // slots
    void saveFinished();
    void saveFailed( IncidenceEditorNG::EditorItemManager::SaveAction, const QString &errorMessage );
    void collectionChanged( const Akonadi::Collection &collection );

  public: // ItemEditorUi interface
    bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const
    {
      return partIdentifiers.contains( Item::FullPayload );
    }

    bool hasSupportedPayload( const Item &item ) const
    {
      return item.hasPayload<KContacts::ContactGroup>();
    }

    bool isDirty() const
    {
      return true;
    }

    bool isValid() const
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
    Collection mDefaultCollection;

    IncidenceEditorNG::EditorItemManager *mItemManager;

    EditorContactGroup *mEditor;
};

void ContactGroupEditorView::Private::saveFinished()
{
  qDebug();
  q->deleteLater();
}

void ContactGroupEditorView::Private::saveFailed( IncidenceEditorNG::EditorItemManager::SaveAction, const QString &errorMessage )
{
  qCritical() << errorMessage;
}

void ContactGroupEditorView::Private::load( const Item &item )
{
  Q_ASSERT( item.hasPayload<KContacts::ContactGroup>() );

  mItem = item;
  mCollection = item.parentCollection();

  const KContacts::ContactGroup contactGroup = mItem.payload<KContacts::ContactGroup>();

  if ( mEditor != 0 ) {
    mEditor->setDefaultCollection( mCollection );
    mEditor->loadContactGroup( contactGroup );
  }
}

Item ContactGroupEditorView::Private::save( const Item &item )
{
  Item result = item;

  result.setMimeType( KContacts::ContactGroup::mimeType() );

  KContacts::ContactGroup contactGroup;
  if ( mEditor != 0 ) {
    mEditor->saveContactGroup( contactGroup );
  }

  result.setPayload<KContacts::ContactGroup>( contactGroup );

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
      qWarning() << "Item Fetch Failed:" << errorMessage;
      break;

    case ItemHasInvalidPayload:
      qWarning() << "Item has Invalid Payload:" << errorMessage;
      break;
  }

  q->deleteLater();
}

ContactGroupEditorView::ContactGroupEditorView( QWidget *parent )
  : KDeclarativeFullScreenView( QLatin1String( "contactgroup-editor" ), parent ),
    d( new Private( this ) )
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( i18n( "Kontact Touch Contacts" ) );
}

void ContactGroupEditorView::doDelayedInit()
{
  qmlRegisterType<DeclarativeEditorContactGroup>( "org.kde.contacteditors", 4, 5, "ContactGroupEditor" );

  connect( d->mItemManager, SIGNAL(itemSaveFinished(IncidenceEditorNG::EditorItemManager::SaveAction)),
           SLOT(saveFinished()) );
  connect( d->mItemManager, SIGNAL(itemSaveFailed(IncidenceEditorNG::EditorItemManager::SaveAction,QString)),
           SLOT(saveFailed(IncidenceEditorNG::EditorItemManager::SaveAction,QString)) );
}

ContactGroupEditorView::~ContactGroupEditorView()
{
  delete d;
}

void ContactGroupEditorView::setEditor( EditorContactGroup *editor )
{
  d->mEditor = editor;

  if ( d->mEditor != 0 ) {
    if ( d->mDefaultCollection.isValid() ) {
      d->mEditor->setDefaultCollection( d->mDefaultCollection );
    }
    if ( d->mCollection.isValid() ) {
      d->mEditor->setDefaultCollection( d->mCollection );
    }
    connect( d->mEditor, SIGNAL(cancelClicked()) , SLOT(close()) );
    connect( d->mEditor, SIGNAL(saveClicked()), SLOT(save()) );
    connect( d->mEditor, SIGNAL(collectionChanged(Akonadi::Collection)),
             SLOT(collectionChanged(Akonadi::Collection)) );
    connect( d->mEditor, SIGNAL(requestLaunchAccountWizard()),
             this, SIGNAL(requestLaunchAccountWizard()) );
  }
}

void ContactGroupEditorView::setDefaultCollection( const Collection &collection )
{
  d->mDefaultCollection = collection;
}

void ContactGroupEditorView::loadContactGroup( const Item &item )
{
  if ( !d->mEditor ) {
    // the editor is not fully loaded yet, so try later again
    QMetaObject::invokeMethod( this, "loadContactGroup", Qt::QueuedConnection, Q_ARG( Akonadi::Item, item ) );
  } else {
    d->mItemManager->load( item );
  }
}

void ContactGroupEditorView::save()
{
  d->mItemManager->save();
}

void ContactGroupEditorView::cancel()
{
  deleteLater();
}

void ContactGroupEditorView::closeEvent( QCloseEvent *event )
{
  Q_UNUSED( event );
  cancel();
}

#include "moc_contactgroupeditorview.cpp"
