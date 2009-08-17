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

#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttypedialog.h>
#include "akonadi/entitytreemodel.h"
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>

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

      mGenericManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Address Book Folder..." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Address Book Folder", "Copy %1 Address Book Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Address Book Folder" ) );
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

      if ( mCollectionSelectionModel ) {
        if ( mCollectionSelectionModel->selectedRows().count() == 1 ) {
          const QModelIndex index = mCollectionSelectionModel->selectedIndexes().first();
          if ( index.isValid() ) {
            const Collection collection = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
            if ( collection.isValid() ) {
              // action is only visible if the collection is a resource collection
              bool isVisible = (collection.parentCollection() == Collection::root());
              if ( mActions.contains( StandardContactActionManager::DeleteAddressBook ) )
                mActions[ StandardContactActionManager::DeleteAddressBook ]->setVisible( isVisible );
            }
          }
        }
      }

      if ( mActions.contains( StandardContactActionManager::EditItem ) ) {
        bool canEditItem = true;

        // only one selected item can be edited
        canEditItem = canEditItem && (itemCount == 1);

        // check whether parent collection allows changing the item
        const QModelIndexList rows = mItemSelectionModel->selectedRows();
        if ( rows.count() == 1 ) {
          const QModelIndex index = rows.first();
          const Collection parentCollection = index.data( EntityTreeModel::ParentCollectionRole ).value<Collection>();
          if ( parentCollection.isValid() )
            canEditItem = canEditItem && (parentCollection.rights() & Collection::CanChangeItem);
        }

        mActions.value( StandardContactActionManager::EditItem )->setEnabled( canEditItem );
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

    void addAddressBookTriggered()
    {
      AgentTypeDialog dlg( mParentWidget );
      dlg.setWindowTitle( i18n( "Add Address Book" ) );
      dlg.agentFilterProxyModel()->addMimeTypeFilter( KABC::Addressee::mimeType() );
      dlg.agentFilterProxyModel()->addMimeTypeFilter( KABC::ContactGroup::mimeType() );
      dlg.agentFilterProxyModel()->addCapabilityFilter( "Resource" ); // show only resources, no agents

      if ( dlg.exec() ) {
        const AgentType agentType = dlg.agentType();

        if ( agentType.isValid() ) {
          AgentInstanceCreateJob *job = new AgentInstanceCreateJob( agentType );
          job->configure( mParentWidget );
          mParent->connect( job, SIGNAL( result( KJob* ) ), mParent, SLOT( addAddressBookResult( KJob* ) ) );
          job->start();
        }
      }
    }

    void addAddressBookResult( KJob *job )
    {
      if ( job->error() ) {
        KMessageBox::error( mParentWidget, i18n( "Could not add address book: %1", job->errorString() ),
                            i18n( "Adding Address Book failed" ) );
      } else {
        const AgentInstanceCreateJob *createJob = static_cast<AgentInstanceCreateJob*>( job );

        AgentInstance instance = createJob->instance();
        if ( instance.isValid() )
          instance.synchronize();
      }
    }

    void deleteAddressBookTriggered()
    {
      if ( !mCollectionSelectionModel )
        return;

      const QModelIndex index = mCollectionSelectionModel->selectedIndexes().first();
      if ( !index.isValid() )
        return;

      const Collection collection = index.data( EntityTreeModel::CollectionRole).value<Collection>();
      const QString identifier = collection.resource();

      const AgentInstance instance = AgentManager::self()->instance( identifier );
      if ( !instance.isValid() )
        return;

      const QString text = i18n( "Do you really want to delete address book '%1'?", instance.name() );

      if ( KMessageBox::questionYesNo( mParentWidget, text,
           i18n( "Delete Address Book?"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
           QString(), KMessageBox::Dangerous ) != KMessageBox::Yes )
        return;

      AgentManager::self()->removeInstance( instance );
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

  connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( updateActions() ) );
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
      action->setEnabled( false );
      d->mActions.insert( EditItem, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_item_edit" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( editTriggered() ) );
      break;
    case CreateAddressBook:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( "folder-new" ) );
      action->setText( i18n( "Add &Address Book..." ) );
      d->mActions.insert( CreateAddressBook, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_addressbook_create" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( addAddressBookTriggered() ) );
      break;
    case DeleteAddressBook:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( "edit-delete" ) );
      action->setText( i18n( "&Delete Address Book" ) );
      action->setVisible( false );
      d->mActions.insert( DeleteAddressBook, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_addressbook_delete" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( deleteAddressBookTriggered() ) );
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
  createAction( CreateAddressBook );
  createAction( DeleteAddressBook );

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
