/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "mainview.h"

#include "actionhelper.h"
#include "notelistproxy.h"
#include "notesexporthandler.h"
#include "notesfilterproxymodel.h"
#include "notesimporthandler.h"
#include "searchwidget.h"

#include <AkonadiWidgets/agentactionmanager.h>
#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/itemfetchscope.h>
#include <AkonadiWidgets/standardactionmanager.h>
#include <akonadi_next/note.h>
#include <akonadi_next/notecreatorandselector.h>

#include <QAction>
#include <KActionCollection>
#include <KLocale>
#include <KMessageBox>
#include <KMime/KMimeMessage>

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QDeclarativeItem>

#ifdef KDEQMLPLUGIN_STATIC
#include "runtime/qml/kde/kdeintegration.h"
#endif

using namespace Akonadi;

QML_DECLARE_TYPE( DeclarativeSearchWidget )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( QLatin1String("notes"), new NoteListProxy( Akonadi::EntityTreeModel::UserRole ), parent )
{
}

void MainView::doDelayedInit()
{
  setWindowTitle( i18n( "Notes" ) );

  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.notes", 4, 5, "SearchWidget" );

#ifdef KDEQMLPLUGIN_STATIC
  rootContext()->setContextProperty( QLatin1String( "KDE" ), new KDEIntegration( this ) );
#endif

  addMimeType( QLatin1String("text/x-vnd.akonadi.note") );
  itemFetchScope().fetchFullPayload();

  QAction *action = new QAction( i18n( "New Note" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(startComposer()) );
  actionCollection()->addAction( QLatin1String( "add_new_note" ), action );

  action = new QAction( i18n( "Import Notes" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(importItems()) );
  actionCollection()->addAction( QLatin1String( "import_notes" ), action );

  action = new QAction( i18n( "Export Notes From This Account" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportItems()) );
  actionCollection()->addAction( QLatin1String( "export_account_notes" ), action );

  action = new QAction( i18n( "Export Displayed Notes" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportItems()) );
  actionCollection()->addAction( QLatin1String( "export_selected_notes" ), action );
}

QString MainView::noteTitle( int row ) const
{
  if ( row < 0 )
    return QString();

  QObject *itemModelObject = engine()->rootContext()->contextProperty( QLatin1String("itemModel") ).value<QObject *>();
  QAbstractItemModel *itemModel = qobject_cast<QAbstractItemModel *>( itemModelObject );

  if ( !itemModel )
    return QString();

  static const int column = 0;
  const QModelIndex index = itemModel->index( row, column );

  if ( !index.isValid() )
    return QString();

  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return QString();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
   return QString();

  const KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  return note->subject()->asUnicodeString();
}

QString MainView::noteContent( int row ) const
{
  if ( row < 0 )
    return QString();

  QObject *itemModelObject = engine()->rootContext()->contextProperty( QLatin1String("itemModel") ).value<QObject *>();
  QAbstractItemModel *itemModel = qobject_cast<QAbstractItemModel *>( itemModelObject );

  if ( !itemModel )
    return QString();

  static const int column = 0;
  const QModelIndex index = itemModel->index( row, column );

  if ( !index.isValid() )
    return QString();

  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return QString();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
   return QString();

  const KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  // TODO: Rich mimetype.
  return note->mainBodyPart()->decodedText();
}

void MainView::saveNote( const QString& title, const QString& content )
{
  QAbstractItemModel *model = const_cast<QAbstractItemModel*>( itemSelectionModel()->model() );

  if ( !model->hasChildren() )
    return;

  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if ( list.size() != 1 )
    return;

  const QModelIndex index = list.first();

  Q_ASSERT( index.isValid() );

  Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return;

  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
  note->subject()->fromUnicodeString( title, "utf-8" );
  KMime::Content *c = note->mainBodyPart();
  c->fromUnicodeString( content );

  note->assemble();

  model->setData( index, QVariant::fromValue( item ), EntityTreeModel::ItemRole );
}

void MainView::saveCurrentNoteTitle( const QString& title )
{
  QAbstractItemModel *model = const_cast<QAbstractItemModel*>( itemSelectionModel()->model() );

  if ( !model->hasChildren() )
    return;

  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if ( list.size() != 1 )
    return;

  const QModelIndex index = list.first();

  Q_ASSERT( index.isValid() );

  Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return;

  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
  note->subject()->fromUnicodeString( title, "utf-8" );

  note->assemble();

  model->setData( index, QVariant::fromValue( item ), EntityTreeModel::ItemRole );
}

void MainView::saveCurrentNoteContent( const QString& content )
{
  QAbstractItemModel *model = const_cast<QAbstractItemModel*>( itemSelectionModel()->model() );

  if ( !model->hasChildren() )
    return;

  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if ( list.size() != 1 )
    return;

  const QModelIndex index = list.first();

  Q_ASSERT( index.isValid() );

  Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return;

  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
  KMime::Content *c = note->mainBodyPart();
  c->fromUnicodeString( content );

  note->assemble();

  model->setData( index, QVariant::fromValue( item ), EntityTreeModel::ItemRole );
}

Collection MainView::suitableContainerCollection( const QModelIndex &parent ) const
{
  const int rowCount = entityTreeModel()->rowCount( parent );
  for ( int row = 0; row < rowCount; ++row ) {
    static const int column = 0;
    const QModelIndex index = entityTreeModel()->index( row, column, parent );
    Q_ASSERT( index.isValid() );

    const Collection collection = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
    Q_ASSERT( collection.isValid() );

    if ( collection.contentMimeTypes().contains( Akonotes::Note::mimeType() ) )
      return collection;

    const Collection descendantCollection = suitableContainerCollection( index );
    if ( descendantCollection.isValid() )
      return descendantCollection;
  }

  return Collection();
}

void MainView::startComposer()
{
  // If a collection is currently selected, put the new note there.

  const int rowCount = selectedItemsModel()->rowCount();

  if ( rowCount > 1 )
    // Multiple items are selected. Find out how this should be handled.
    return;

  if ( rowCount == 1 ) {
    const QModelIndex index = selectedItemsModel()->index( 0, 0 );
    Q_ASSERT( index.isValid() );

    const Collection collection = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
    Q_ASSERT( collection.isValid() );

    Akonotes::NoteCreatorAndSelector *noteCreator = new Akonotes::NoteCreatorAndSelector( itemSelectionModel(), itemSelectionModel(), this );
    noteCreator->createNote( collection );

    return;
  }

  // otherwise nothing is selected, find a collection which can contain notes and put it there.

  const Collection collection = suitableContainerCollection();

  if ( !collection.isValid() ) {
    KMessageBox::information( this, i18n( "You do not appear to have any resources for notes. Please create one first." ),
                              i18n( "No resources available" ) );
    // No suitable collection found.
    // Create a resource with LocalResourceCreator,
    // then add a collection, then use the NoteCreatorAndSelector.
    return;
  }

  Akonotes::NoteCreatorAndSelector *noteCreator = new Akonotes::NoteCreatorAndSelector( regularSelectionModel(), itemSelectionModel(), this );
  noteCreator->createNote( collection );
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  Akonadi::StandardActionManager *manager = new Akonadi::StandardActionManager( actionCollection(), this );
  manager->setCollectionSelectionModel( collectionSelectionModel );
  manager->setItemSelectionModel( itemSelectionModel );

  manager->createAllActions();
  manager->interceptAction( Akonadi::StandardActionManager::CreateResource );

  connect( manager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL(triggered(bool)),
           this, SLOT(launchAccountWizard()) );

  ActionHelper::adaptStandardActionTexts( manager );

  manager->action( StandardActionManager::CollectionProperties )->setText( i18n( "Notebook Properties" ) );
  manager->action( StandardActionManager::CreateCollection )->setText( i18n( "New Sub Notebook" ) );
  manager->action( StandardActionManager::CreateCollection )->setProperty( "ContentMimeTypes", QStringList() << Akonadi::Collection::mimeType() << Akonotes::Note::mimeType() );
  manager->setActionText( StandardActionManager::SynchronizeCollections, ki18np( "Synchronize This Notebook", "Synchronize These Notebooks" ) );
  manager->setActionText( StandardActionManager::DeleteCollections, ki18np( "Delete Notebook", "Delete Notebooks" ) );
  manager->action( StandardActionManager::MoveCollectionToDialog )->setText( i18n( "Move Notebook To" ) );
  manager->action( StandardActionManager::CopyCollectionToDialog )->setText( i18n( "Copy Notebook To" ) );
  manager->action( StandardActionManager::CopyItemToDialog )->setText( i18n( "Copy Note To" ) );
  manager->action( StandardActionManager::MoveItemToDialog )->setText( i18n( "Move Note To" ) );
  manager->setActionText( StandardActionManager::DeleteItems, ki18np( "Delete Note", "Delete Notes" ) );

  actionCollection()->action( QLatin1String("synchronize_all_items") )->setText( i18n( "Synchronize All Accounts" ) );
}

void MainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  Akonadi::AgentActionManager *manager = createAgentActionManager( selectionModel );

  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::DialogTitle,
                           i18nc( "@title:window", "New Account" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageText,
                           ki18n( "Could not create account: %1" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageTitle,
                           i18n( "Account creation failed" ) );

  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxTitle,
                           i18nc( "@title:window", "Delete Account?" ) );
  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxText,
                           i18n( "Do you really want to delete the selected account?" ) );
}

QAbstractProxyModel* MainView::createItemFilterModel() const
{
  return new NotesFilterProxyModel();
}

ImportHandlerBase* MainView::importHandler() const
{
  return new NotesImportHandler();
}

ExportHandlerBase* MainView::exportHandler() const
{
  return new NotesExportHandler();
}
