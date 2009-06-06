/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "standardcontactactionmanager.h"

#include "akonadi_next/entitytreemodel.h"

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include <QtGui/QItemSelectionModel>

using namespace Akonadi;

class StandardContactActionManager::Private
{
  public:
    Private( KActionCollection *actionCollection, QWidget *parentWidget, StandardContactActionManager *parent )
      : mActionCollection( actionCollection ), mParentWidget( parentWidget ),
        mCollectionSelectionModel( 0 ), mItemSelectionModel( 0 ), mParent( parent )
    {
      mGenericManager = new StandardActionManager( actionCollection, parentWidget );
      mParent->connect( mGenericManager, SIGNAL( actionStateUpdated() ),
                        mParent, SIGNAL( actionStateUpdated() ) );
      mGenericManager->createAllActions();

      mGenericManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Address Book" ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Address Book", "Copy %1 Address Books" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Address Book" ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Reload" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Properties..." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems, ki18np( "Copy Contact", "Copy %1 Contacts" ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete Contact", "Delete %1 Contacts" ) );

    }

    ~Private()
    {
      delete mGenericManager;
    }

    void updateActions()
    {
      int itemCount = 0;
      if ( mItemSelectionModel ) {
        itemCount = mItemSelectionModel->selectedRows().count();
        if ( itemCount == 1 ) {
          const QModelIndex index = mItemSelectionModel->selectedIndexes().first();
          if ( index.isValid() ) {
            const QString mimeType = index.data( EntityTreeModel::MimeTypeRole ).toString();
            if ( mimeType == KABC::Addressee::mimeType() ) {
              mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems,
                                              ki18np( "Copy Contact", "Copy %1 Contacts" ) );
              mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems,
                                              ki18np( "Delete Contact", "Delete %1 Contacts" ) );
              if ( mActions.contains( StandardContactActionManager::EditItem ) )
                mActions.value( StandardContactActionManager::EditItem )->setText( i18n( "Edit Contact..." ) );
            } else if ( mimeType == KABC::ContactGroup::mimeType() ) {
              mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems,
                                              ki18np( "Copy Group", "Copy %1 Groups" ) );
              mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems,
                                              ki18np( "Delete Group", "Delete %1 Groups" ) );
              if ( mActions.contains( StandardContactActionManager::EditItem ) )
                mActions.value( StandardContactActionManager::EditItem )->setText( i18n( "Edit Group..." ) );
            }
          }
        }
      }

      if ( mActions.contains( StandardContactActionManager::EditItem ) ) {
        mActions.value( StandardContactActionManager::EditItem )->setEnabled( itemCount == 1 );
      }

      emit mParent->actionStateUpdated();
    }

    void editTriggered()
    {
      if ( !mItemSelectionModel )
        return;

      const QModelIndex index = mItemSelectionModel->selectedIndexes().first();
      if ( !index.isValid() )
        return;

      const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( !item.isValid() )
        return;

      emit mParent->editItem( item );
    }

    KActionCollection *mActionCollection;
    QWidget *mParentWidget;
    StandardActionManager *mGenericManager;
    QItemSelectionModel *mCollectionSelectionModel;
    QItemSelectionModel *mItemSelectionModel;
    QHash<StandardContactActionManager::Type, KAction*> mActions;
    StandardContactActionManager *mParent;
};

StandardContactActionManager::StandardContactActionManager( KActionCollection *actionCollection, QWidget *parent )
  : d( new Private( actionCollection, parent, this ) )
{
}

StandardContactActionManager::~StandardContactActionManager()
{
  delete d;
}

void StandardContactActionManager::setCollectionSelectionModel( QItemSelectionModel *selectionModel )
{
  d->mCollectionSelectionModel = selectionModel;
  d->mGenericManager->setCollectionSelectionModel( selectionModel );
}

void StandardContactActionManager::setItemSelectionModel( QItemSelectionModel* selectionModel )
{
  d->mItemSelectionModel = selectionModel;
  d->mGenericManager->setItemSelectionModel( selectionModel );

  connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( updateActions() ) );
}

KAction* StandardContactActionManager::createAction( Type type )
{
  Q_ASSERT( type >= CreateContact && type < LastType );

  if ( d->mActions.contains( type ) )
    return d->mActions.value( type );

  KAction *action = 0;

  switch ( type ) {
    case CreateContact:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( "contact-new" ) );
      action->setText( i18n( "New &Contact..." ) );
      action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
      action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );
      d->mActions.insert( CreateContact, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_create" ), action );
      break;
    case CreateContactGroup:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( "user-group-new" ) );
      action->setText( i18n( "New &Group..." ) );
      action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
      action->setWhatsThis( i18n( "Create a new group<p>You will be presented with a dialog where you can add a new group of contacts.</p>" ) );
      d->mActions.insert( CreateContactGroup, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_group_create" ), action );
      break;
    case EditItem:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( "document-edit" ) );
      action->setText( i18n( "Edit Contact..." ) );
      action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_E ) );
      d->mActions.insert( EditItem, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_item_edit" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( editTriggered() ) );
      break;
    default:
      Q_ASSERT( false ); // should never happen
      break;
  }

  return action;
}

void StandardContactActionManager::createAllActions()
{
  createAction( CreateContact );
  createAction( CreateContactGroup );
  createAction( EditItem );

  d->mGenericManager->createAllActions();
}

KAction* StandardContactActionManager::action( Type type ) const
{
  if ( d->mActions.contains( type ) )
    return d->mActions.value( type );
  else
    return 0;
}

#include "standardcontactactionmanager.moc"
